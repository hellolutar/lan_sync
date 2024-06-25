link_directories(
  /usr/local/lib64
  /usr/local/lib
  /usr/lib/x86_64-linux-gnu
  /usr/lib
  ${CMAKE_BINARY_DIR}
)

include_directories(
  /usr/local/include
  /usr/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/libs/include
)

find_package(Libevent REQUIRED)
find_library(SPDLOG_LIBRARY libspdlog.so REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIRS})
  message(STATUS "OpenSSL Found!")
endif()

add_compile_options(-Wno-format-contains-nul -DRELEASE)

file(GLOB_RECURSE src_codes ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM src_codes ${PROJECT_SOURCE_DIR}/src/main.cpp ) 
