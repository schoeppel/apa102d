cmake_minimum_required(VERSION 2.6)
project(apa102)

set(CMAKE_C_FLAGS "-O2 --std=gnu99 -Wall -Wextra -Wno-unused-parameter -ggdb")

add_executable(apa102d main.c apa102.c config.c test.c off.c bubbles.c text.c particles.c color.c)

target_link_libraries(apa102d -lm -lrt -lmosquitto)

install(TARGETS apa102d RUNTIME DESTINATION sbin)

