cmake_minimum_required(VERSION 3.18)
project(hello_world VERSION 1.0 DESCRIPTION "SFU: as1 for CMPT433")

set(CMAKE_C_STANDARD 11)
add_compile_options(
    -g
    -Wall
    -Werror
    -Wpedantic
    -Wextra
    -D_POSIX_C_SOURCE=200809L
)

add_executable(as1 hello.c)

# Copy executable to final location (change `hello_world` to project name as needed)
add_custom_command(TARGET as1 POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:as1>"
     "~/cmpt433/public/myApps/as1" 
  COMMENT "Copying ARM executable to public NFS directory")