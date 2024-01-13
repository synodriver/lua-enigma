cmake_minimum_required(VERSION 3.27)
project(lua_enigma C)

set(CMAKE_C_STANDARD 11)

# change this to your lua.h directory
include_directories(D:\\conda\\envs\\py310\\Library\\include ./dep/src)
# change this to your lua.lib\liblua.a directory
link_directories("D:\\conda\\envs\\py310\\Library\\lib" D:\\conda\\envs\\py310\\Library\\bin)


add_library(enigma SHARED src/lenigma.c dep/src/enigma.c)
target_link_libraries(enigma lua)

set_target_properties(enigma PROPERTIES OUTPUT_NAME enigma PREFIX "")

