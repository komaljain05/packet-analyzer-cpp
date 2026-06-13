cmake_minimum_required(VERSION 3.16)
project(PacketAnalyzer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files (DPI Engine entrypoint)
set(SOURCES
    src/main_dpi.cpp
    src/dpi_engine.cpp
    src/pcap_reader.cpp
    src/packet_parser.cpp
    src/sni_extractor.cpp
    src/types.cpp
    src/rule_manager.cpp
    src/connection_tracker.cpp
    src/fast_path.cpp
    src/load_balancer.cpp
    src/mini_json.cpp
)

# Create the executable
add_executable(packet_analyzer ${SOURCES})

target_include_directories(packet_analyzer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)

target_link_libraries(packet_analyzer PRIVATE Threads::Threads)


# For macOS, we might need to link against system libraries later
if(APPLE)
    # Add any macOS-specific settings here
endif()
