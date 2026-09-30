# ray_bomber

A multiplayer bomberman remake.

## Build requirements

* Go 1.26
* CMake 4.3
* protobuf compiler with go plugin

## Building the client

Navigate to the server directory.
```shell
cd client
```

Build the client.
```shell
cmake --build build
```

Start the client via the compiled binary.
```shell
cd ./bin
```

## Building the server

Generate code from the .proto files
```shell
cd ./server
go generate
```

Start the server
```shell
go run server.go
```
