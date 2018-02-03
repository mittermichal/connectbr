project(connectbr)
cmake_minimum_required(VERSION 2.8)
add_definitions(-DCLIENTONLY)
#CONFIG += debug
set (CONFIG ${CONFIG} debug)
set(SRC_LIST sys_posix.c main.c EX_browser_pathfind.c EX_browser.c EX_browser_net.c EX_browser_ping.c EX_browser_sources.c cvar.c net.c q_shared.h q_shared.c)
#if (WIN32)
#    set(SRC_LIST ${SRC_LIST} sys_win.c)
#endif (WIN32)
#if (UNIX)
#    set(SRC_LIST ${SRC_LIST} sys_posix.c)
#endif (UNIX)
add_executable(${PROJECT_NAME} ${SRC_LIST})

