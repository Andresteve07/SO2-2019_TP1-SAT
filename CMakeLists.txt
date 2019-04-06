cmake_minimum_required (VERSION 2.6)

project (SO2-2019_TP1)
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(lib)
# ncurses library
#find_package(Curses REQUIRED)
#include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DLOG_USE_COLOR)
#target_compile_definitions(run_test PRIVATE -DLOG_USE_COLOR)
 
add_executable(run_test 
unittest/test_both_even.c 
src/both.c 
src/frozen.c 
)

add_executable(so2_tp1_station 
src/station_app.c 
src/frozen.c 
src/presenter.c 
src/vista.c
src/login_service.c
src/tcp_socket_client.c
src/udp_socket_client.c
src/satellite_service.c
src/log.c)

#ncurses library
#target_link_libraries(run_test ${CURSES_LIBRARIES})