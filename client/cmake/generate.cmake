add_library(proto_files)

set(PROTO_DIRECTORY "${CMAKE_SOURCE_DIR}/../proto")

make_directory("${CMAKE_BINARY_DIR}/protocol")
protobuf_generate(
	TARGET proto_files
	PROTOC_OUT_DIR "${CMAKE_BINARY_DIR}/protocol"
	PROTOS 
		"${PROTO_DIRECTORY}/envelope.proto"
		"${PROTO_DIRECTORY}/player.proto"
	IMPORT_DIRS 
		"${PROTO_DIRECTORY}"
)

target_link_libraries(proto_files
	PUBLIC
        protobuf::libprotobuf
)

target_include_directories(proto_files
	PUBLIC
		"${CMAKE_BINARY_DIR}/protocol"
)