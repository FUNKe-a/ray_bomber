add_library(protobuf)

set(PROTO_DIRECTORY "${CMAKE_SOURCE_DIR}/../proto")

make_directory("${CMAKE_BINARY_DIR}/protocol")
protobuf_generate(
	TARGET protobuf
	PROTOC_OUT_DIR "${CMAKE_BINARY_DIR}/protocol"
	PROTOS 
		"${PROTO_DIRECTORY}/envelope.proto"
		"${PROTO_DIRECTORY}/player.proto"
	IMPORT_DIRS 
		"${PROTO_DIRECTORY}"
	
)
