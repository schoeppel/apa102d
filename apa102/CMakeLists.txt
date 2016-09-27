cmake_minimum_required(VERSION 2.6)
project(apa102)

set(CMAKE_C_FLAGS "-O2 --std=gnu99 -Wall -Wextra -Wno-unused-parameter -ggdb")

add_executable(apa102d main.c particles.c waves.c bubbles.c single_color.c simple_step.c stroboscope.c apa102.c color.c)
add_executable(apa102 client.c)

target_link_libraries(apa102d -lm -lrt)
target_link_libraries(apa102 -lm -lrt)

install(TARGETS apa102 RUNTIME DESTINATION bin)
