cmake_minimum_required(VERSION 3.12)

project(transport_catalogue VERSION 0.0.1 DESCRIPTION "Transport catalogue")

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)

find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(UTILITY geo.h geo.cpp ranges.h)

set(TRANSPORT_CATALOGUE domain.h domain.cpp transport_catalogue.h transport_catalogue.cpp transport_catalogue.proto)
                      
set(ROUTER graph.h graph.proto router.h transport_router.h transport_router.cpp transport_router.proto)
                              
set(JSON json.h json.cpp json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
               
set(SVG svg.h svg.cpp svg.proto)
        
set(MAP_RENDERER map_renderer.h map_renderer.cpp map_renderer.proto)
              
set(SERIALIZATION serialization.h serialization.cpp)
                 
set(REQUEST_HANDLER request_handler.h request_handler.cpp)

add_executable(transport_catalogue main.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${UTILITY} ${TRANSPORT_CATALOGUE} ${ROUTER} ${JSON} ${SVG} ${MAP_RENDERER} ${SERIALIZATION} ${REQUEST_HANDLER})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
