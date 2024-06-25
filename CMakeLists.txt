CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

PROJECT(ltsync LANGUAGES CXX)

include(etc/config.cmake)
include(etc/common.cmake)

add_library(lansync_common_obj STATIC ${src_codes})

include(etc/tests.cmake)

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
