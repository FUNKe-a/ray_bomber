find_package(protobuf CONFIG REQUIRED)

set(PROTO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto")
set(PROTO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${PROTO_GENERATED_DIR}")

add_library(proto_files STATIC
    "${PROTO_SOURCE_DIR}/envelope.proto"
    "${PROTO_SOURCE_DIR}/lobby.proto"
    "${PROTO_SOURCE_DIR}/match.proto"
    "${PROTO_SOURCE_DIR}/player.proto"
)

target_link_libraries(proto_files
    PUBLIC protobuf::libprotobuf
)

target_include_directories(proto_files
    PUBLIC "${PROTO_GENERATED_DIR}"
)

protobuf_generate(
    TARGET proto_files
    LANGUAGE cpp
    APPEND_PATH
    IMPORT_DIRS "${PROTO_SOURCE_DIR}"
    PROTOC_OUT_DIR "${PROTO_GENERATED_DIR}"
)
