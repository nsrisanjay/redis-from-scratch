import socket
import time

HOST = "127.0.0.1"
PORT = 6969


def send_test(name, chunks, delay=0.5):
    print(f"\n--- {name} ---")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    for chunk in chunks:
        print(f"Sending: {chunk!r}")
        sock.sendall(chunk)
        time.sleep(delay)

    time.sleep(0.5)
    sock.close()

    print("Connection closed")


# 1. Complete request
send_test(
    "Complete PING",
    [
        b"*1\r\n$4\r\nPING\r\n"
    ]
)


# 2. Fragmented request
send_test(
    "Fragmented PING",
    [
        b"*1\r\n$4\r\nPI",
        b"NG\r\n"
    ],
    delay=2
)


# 3. Multiple requests in one TCP write
send_test(
    "Multiple requests",
    [
        b"*1\r\n$4\r\nPING\r\n"
        b"*1\r\n$4\r\nPING\r\n"
    ]
)


# 4. Multiple requests split across TCP writes
send_test(
    "Multiple fragmented requests",
    [
        b"*1\r\n$4\r\nPI",
        b"NG\r\n*1\r\n$4\r\nPI",
        b"NG\r\n"
    ],
    delay=1
)


# 5. Simple string
send_test(
    "Simple string",
    [
        b"+OK\r\n"
    ]
)


# 6. Integer
send_test(
    "Integer",
    [
        b":12345\r\n"
    ]
)


# 7. Bulk string
send_test(
    "Bulk string",
    [
        b"$5\r\nhello\r\n"
    ]
)


# 8. Empty bulk string
send_test(
    "Empty bulk string",
    [
        b"$0\r\n\r\n"
    ]
)


# 9. Null bulk string
send_test(
    "Null bulk string",
    [
        b"$-1\r\n"
    ]
)


# 10. Error
send_test(
    "Error",
    [
        b"-ERR something went wrong\r\n"
    ]
)


# 11. Nested array
send_test(
    "Nested array",
    [
        b"*2\r\n"
        b"$4\r\nPING\r\n"
        b"*1\r\n"
        b"$3\r\nGET\r\n"
    ]
)


# 12. Malformed RESP
send_test(
    "Malformed request",
    [
        b"*1\r\n$4\r\nPING"
    ]
)


# 13. Invalid RESP type
send_test(
    "Invalid RESP type",
    [
        b"@invalid\r\n"
    ]
)


# 14. Malformed array
send_test(
    "Malformed array",
    [
        b"*\r\n"
    ]
)


print("\nAll tests sent.")
