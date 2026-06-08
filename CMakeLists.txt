cmake_minimum_required(VERSION 3.20)
project(deerstatus C)

add_custom_command(OUTPUT deer_running.c
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gendeerpage.sh running
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/deer_running.gif
                   VERBATIM)

add_custom_command(OUTPUT deer_sleeping.c
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gendeerpage.sh sleeping
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/deer_sleeping.gif
                   VERBATIM)

set(SOURCES main.c ping.c server.c deer_running.c deer_sleeping.c)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
target_compile_definitions(${PROJECT_NAME} PUBLIC EPOLL)

add_subdirectory(httpserver.h/src)

target_link_libraries(${PROJECT_NAME} httpsrv)
