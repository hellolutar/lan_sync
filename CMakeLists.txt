CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

PROJECT(ltsync LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(TARGET_NAME lansync)

link_directories(
  /usr/local/lib64
  /usr/local/lib
  /usr/lib/x86_64-linux-gnu
  /usr/lib
)

include_directories(
  /usr/local/include
  /usr/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/libs/include
)

find_package(Libevent REQUIRED)
find_library(SPDLOG_LIBRARY libspdlog.so REQUIRED)
set(OPENSSL_SSL_LIBRARY /usr/local/lib64/libssl.so )
set(OPENSSL_CRYPTO_LIBRARY /usr/local/lib64/libcrypto.so)

add_compile_options(-Wno-format-contains-nul -DRELEASE)

file(GLOB_RECURSE src_codes ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${TARGET_NAME} ${src_codes})

target_link_libraries(
    ${TARGET_NAME} 
    libevent::core 
    libevent::pthreads 
    ${SPDLOG_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
)