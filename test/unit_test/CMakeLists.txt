find_package(GTest REQUIRED)

include(${PROJECT_SOURCE_DIR}/etc/config.cmake)
include(${PROJECT_SOURCE_DIR}/etc/common.cmake)

enable_testing()

include(GoogleTest)

macro(ttest name)
  add_executable(${name} ${PROJECT_SOURCE_DIR}/test/unit_test/${name}.cpp)
  target_link_libraries(
    ${name}
    GTest::gtest_main
    libevent::core
    libevent::pthreads
    ${SPDLOG_LIBRARY}
    OpenSSL::Crypto
    OpenSSL::SSL
    lansync_common_obj
  )
  gtest_discover_tests(${name})
endmacro(ttest)

ttest(fixture_example)
ttest(io_utils_test)
ttest(lan_share_protocol_test)
ttest(net_addr_test)
ttest(openssl_utils_test)
ttest(properties_parse_test)
ttest(resource_manager_test)
ttest(rs_sync_mamanger_test)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --stop-on-failure --timeout 12)
