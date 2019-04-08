cmake_minimum_required (VERSION 2.6)

project (SO2-2019_TP1-SAT)
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(lib)
# ncurses library
#find_package(Curses REQUIRED)
#include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DLOG_USE_COLOR)
#target_compile_definitions(run_test PRIVATE -DLOG_USE_COLOR)

add_executable(so2_tp1_satellite 
src/satellite_app.c 
src/frozen.c
src/log.c
src/shared/tcp_socket_operations.c
src/shared/udp_socket_operations.c
src/command_router.c
src/command_controller.c
src/system_stats_service.c
)

#ncurses library
#target_link_libraries(run_test ${CURSES_LIBRARIES})