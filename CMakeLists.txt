CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

PROJECT(ltsync LANGUAGES CXX)

include(etc/config.cmake)
include(etc/common.cmake)

add_library(lansync_common_obj STATIC ${src_codes})

add_subdirectory(test/unit_test test/unit_test)
add_subdirectory(examples examples)

add_executable(${TARGET_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(
    ${TARGET_NAME}
    libevent::core
    libevent::pthreads
    ${SPDLOG_LIBRARY}
    OpenSSL::Crypto
    OpenSSL::SSL
    lansync_common_obj
)
