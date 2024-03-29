cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
project(transport_catalogue VERSION 0.0.1 DESCRIPTION "Transport catalogue")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS catalogue.proto svg.proto renderer.proto router.proto)

set(GEO geo/geo.h geo/geo.cpp)
set(GRAPH graph/graph.h graph/router.h graph/ranges.h)
set(SVG svg/svg.h svg/svg.cpp svg.proto)
set(JSON json/json.h json/json.cpp json/builder.h json/builder.cpp)
set(CATALOGUE domain.h catalogue.h catalogue.cpp reader.h reader.cpp catalogue.proto)
set(HANDLER handler.h handler.cpp)
set(ROUTER router.h router.cpp router.proto)
set(RENDERER renderer.h renderer.cpp renderer.proto)
set(SERIALIZER serializer.h serializer.cpp)
set(LOG log/easylogging++.h log/easylogging++.cc)

add_executable(transport_catalogue main.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${GEO} ${GRAPH} ${CATALOGUE} ${ROUTER} ${JSON} ${SVG} ${RENDERER} ${SERIALIZER} ${HANDLER} ${LOG})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
