import argparse
import select
import socket
import uuid


def encode_command(*arguments):
    encoded = [f"*{len(arguments)}\r\n".encode()]
    for argument in arguments:
        if isinstance(argument, str):
            argument = argument.encode()
        encoded.extend((f"${len(argument)}\r\n".encode(), argument, b"\r\n"))
    return b"".join(encoded)


def read_line(stream):
    line = stream.readline()
    if not line.endswith(b"\r\n"):
        raise AssertionError(f"incomplete RESP line: {line!r}")
    return line[:-2]


def read_reply(stream):
    prefix = stream.read(1)
    if not prefix:
        raise AssertionError("server closed the connection before replying")

    if prefix in (b"+", b"-", b":"):
        value = read_line(stream)
        return prefix, int(value) if prefix == b":" else value

    if prefix == b"$":
        length = int(read_line(stream))
        if length == -1:
            return prefix, None
        if length < -1:
            raise AssertionError(f"invalid bulk-string length: {length}")
        value = stream.read(length)
        if len(value) != length or stream.read(2) != b"\r\n":
            raise AssertionError("incomplete bulk-string response")
        return prefix, value

    if prefix == b"*":
        count = int(read_line(stream))
        if count == -1:
            return prefix, None
        if count < -1:
            raise AssertionError(f"invalid array length: {count}")
        return prefix, [read_reply(stream) for _ in range(count)]

    raise AssertionError(f"unknown RESP response prefix: {prefix!r}")


class Client:
    def __init__(self, host, port, timeout):
        self.socket = socket.create_connection((host, port), timeout=timeout)
        self.socket.settimeout(timeout)
        self.stream = self.socket.makefile("rb")

    def command(self, *arguments):
        self.socket.sendall(encode_command(*arguments))
        return read_reply(self.stream)

    def close(self):
        self.stream.close()
        self.socket.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()


def expect(actual, expected, name):
    if actual != expected:
        raise AssertionError(f"{name}: expected {expected!r}, got {actual!r}")
    print(f"PASS {name}")


def run_tests(host, port, timeout):
    prefix = f"checkpoint1:{uuid.uuid4().hex}:"
    key = prefix + "key"
    empty_key = prefix + "empty"
    large_key = prefix + "large"
    fragment_key = prefix + "fragmented"
    pipe_key = prefix + "pipelined"
    missing_key = prefix + "missing"
    large_value = b"x" * 20000

    with Client(host, port, timeout) as client:
        expect(client.command("PING"), (b"+", b"PONG"), "PING")
        expect(client.command("PING", "hello"), (b"$", b"hello"), "PING with message")

        expect(client.command("SET", key, "first"), (b"+", b"OK"), "SET new key")
        expect(client.command("SET", key, "updated"), (b"+", b"OK"), "SET overwrites key")
        expect(client.command("GET", key), (b"$", b"updated"), "GET existing key")
        expect(client.command("GET", missing_key), (b"$", None), "GET missing key")

        expect(client.command("SET", empty_key, ""), (b"+", b"OK"), "SET empty value")
        expect(client.command("GET", empty_key), (b"$", b""), "GET empty value")
        expect(client.command("SET", large_key, large_value), (b"+", b"OK"), "SET large value")
        expect(client.command("GET", large_key), (b"$", large_value), "GET large value")

        expect(client.command("EXISTS", key, missing_key, key), (b":", 2), "EXISTS multiple keys")
        expect(client.command("TYPE", key), (b"+", b"string"), "TYPE existing key")
        expect(client.command("TYPE", missing_key), (b"+", b"none"), "TYPE missing key")

        base_size = client.command("DBSIZE")
        expect(base_size[0], b":", "DBSIZE response type")
        expect(base_size[1], 3, "DBSIZE key count before stream tests")

        with Client(host, port, timeout) as second_client:
            expect(second_client.command("GET", key), (b"$", b"updated"), "value persists across clients")

        with Client(host, port, timeout) as fragmented:
            request = encode_command("SET", fragment_key, "fragmented-value")
            split = len(request) // 2
            fragmented.socket.sendall(request[:split])
            readable, _, _ = select.select([fragmented.socket], [], [], 0.1)
            if readable:
                raise AssertionError("server replied before the fragmented request was complete")
            fragmented.socket.sendall(request[split:])
            expect(read_reply(fragmented.stream), (b"+", b"OK"), "fragmented command")
            expect(fragmented.command("GET", fragment_key), (b"$", b"fragmented-value"), "GET fragmented command result")

        with Client(host, port, timeout) as pipelined:
            pipelined.socket.sendall(
                encode_command("SET", pipe_key, "pipeline-value")
                + encode_command("GET", pipe_key)
            )
            expect(read_reply(pipelined.stream), (b"+", b"OK"), "first pipelined reply")
            expect(read_reply(pipelined.stream), (b"$", b"pipeline-value"), "second pipelined reply")

        expect(client.command("SET"), (b"-", b"ERR wrong number of arguments for 'set' command"), "wrong arity")
        expect(client.command("NO-SUCH-COMMAND"), (b"-", b"ERR unknown command"), "unknown command")

        expect(client.command("DEL", key, empty_key, large_key, fragment_key, pipe_key, key), (b":", 5), "DEL multiple keys")
        expect(client.command("DEL", missing_key), (b":", 0), "DEL missing key")

    with Client(host, port, timeout) as malformed:
        malformed.socket.sendall(b"@invalid\r\n")
        expect(read_reply(malformed.stream), (b"-", b"ERR Protocol error"), "malformed RESP error")
        if malformed.socket.recv(1) != b"":
            raise AssertionError("server did not close the malformed client connection")

    with Client(host, port, timeout) as reconnect:
        expect(reconnect.command("PING"), (b"+", b"PONG"), "server accepts client after protocol error")


def main():
    parser = argparse.ArgumentParser(description="Checkpoint 1 RESP command integration tests")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6969)
    parser.add_argument("--timeout", type=float, default=3.0)
    arguments = parser.parse_args()

    try:
        run_tests(arguments.host, arguments.port, arguments.timeout)
    except (OSError, AssertionError, ValueError) as error:
        parser.exit(1, f"FAIL {error}\n")

    print("\nCheckpoint 1 command integration tests passed.")


if __name__ == "__main__":
    main()
