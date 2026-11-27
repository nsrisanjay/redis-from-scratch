# Redis From Scratch

An educational Redis-like server written in C++20. It accepts RESP-formatted requests over TCP and supports a small set of in-memory string commands.

This is an early learning project, not a drop-in replacement for Redis.

## Features

- RESP request parsing and response serialization
- TCP server with a configurable port (default: `6969`)
- In-memory key-value storage shared across client connections
- Commands: `PING`, `SET`, `GET`, `EXISTS`, `DEL`, `TYPE`, and `DBSIZE`
- Handles incremental and pipelined RESP input in the protocol layer

## Requirements

- CMake 3.16 or newer
- A C++20 compiler
- Linux or Windows Subsystem for Linux (WSL)

The server currently uses POSIX networking APIs, so it does not build as a native Windows executable.

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build --target redis-server
```

For an optimized Release build:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target redis-server -j
```

The server executable will be at `build/redis-server` or `build-release/redis-server`, depending on the build command used.

## Run

Start the server on the default port:

```bash
./build/redis-server
```

Or choose a port:

```bash
./build/redis-server 7000
```

The server listens on all IPv4 interfaces. Press `Ctrl+C` to stop it. Stored keys are in memory and are lost when the server exits.

## Connect with redis-cli

Install the Redis command-line client if needed. On Ubuntu or WSL:

```bash
sudo apt update
sudo apt install redis-tools
```

In another terminal, connect to the server:

```bash
redis-cli -p 6969
```

Then try:

```text
PING
SET name Sanjay
GET name
EXISTS name
TYPE name
DBSIZE
DEL name
```

Use `QUIT` to disconnect from `redis-cli`.

## Supported commands

| Command | Behavior |
| --- | --- |
| `PING` | Returns `PONG`; `PING message` returns the message. |
| `SET key value` | Stores or replaces a string value and returns `OK`. |
| `GET key` | Returns the value, or a null bulk string if the key is missing. |
| `EXISTS key [key ...]` | Returns the number of supplied keys that exist. |
| `DEL key [key ...]` | Deletes supplied keys and returns the number deleted. |
| `TYPE key` | Returns `string` for an existing key or `none` otherwise. |
| `DBSIZE` | Returns the number of stored keys. |

## Tests

Build and run the parser and serializer unit tests:

```bash
cmake --build build --target array_parser_tests serializer_tests
./build/array_parser_tests
./build/serializer_tests
```

The repository also includes a server integration script. Run the server in one terminal, then run this from another:

```bash
python3 tester.py --port 6969
```

The integration script currently expects simultaneous client connections, which the server does not yet handle. The server processes one connected client at a time, so this check will time out when it tries to use a second client before the first disconnects.

## Current limitations

- Only one client connection is actively handled at a time.
- Data is stored in memory and is not persisted.
- The supported commands and RESP behavior cover only a subset of Redis.
- Native Windows builds are not supported yet; use WSL on Windows.
