# ray_bomber

A multiplayer bomberman remake.

## Build requirements

* Go 1.26
* CMake 4.3
* protobuf compiler with go plugin
* vcpkg and Visual Studio 2026 (on Windows)

## Initial setup

### Windows client dependencies

Run these commands in PowerShell to install vcpkg and Protobuf. Skip cloning and bootstrapping if you already have vcpkg installed.

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
& C:\dev\vcpkg\bootstrap-vcpkg.bat
& C:\dev\vcpkg\vcpkg.exe install protobuf:x64-windows
```
### Server code-generation tools

Install the Go generator:
```shell
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
```

## Building the client

### Windows

Configure, build, and launch from PowerShell:
```powershell
cd client
cmake --preset windows-vcpkg
cmake --build --preset debug
.\bin\Windows\Debug\ray_bomber.exe
```

### Linux

Navigate to the client directory.
```shell
cd client
```

Build the client.
```shell
cmake --build build
```

Start the client via the compiled binary.
```shell
./bin/Linux/Debug/ray_bomber
```

## Building the server

### Windows and Linux

```shell
cd server
go mod download
go generate
go run server.go
```

## Development

### Windows

To generate protobuf code for server use:
```shell
cd ./server
go generate
```

To generate protobuf code for client use:
```shell
cd ./client
cmake --preset windows-vcpkg
cmake --build --preset debug
```

### Linux

To generate protobuf code for server use:
```shell
cd ./server
go generate
```

To generate protobuf code for client use:
```shell
cd ./client
cmake -B build/
cmake --build build --target proto_files
```
