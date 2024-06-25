
include(${PROJECT_SOURCE_DIR}/etc/config.cmake)
include(${PROJECT_SOURCE_DIR}/etc/common.cmake)

macro(compile_example name)
    add_executable(${name} ${PROJECT_SOURCE_DIR}/examples/${name}.cpp)
    target_link_libraries(
        ${name}
        libevent::core
        libevent::pthreads
        ${SPDLOG_LIBRARY}
        OpenSSL::Crypto
        OpenSSL::SSL
        lansync_common_obj
    )
endmacro(compile_example)

compile_example(tcpcli)
compile_example(tcpserver)
compile_example(timer)
compile_example(udpserver)
