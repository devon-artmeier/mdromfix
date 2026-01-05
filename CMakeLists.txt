cmake_minimum_required(VERSION 3.15)
project(mdromfix LANGUAGES C)

add_executable(mdromfix
	"src/main.c"
)

if(NOT MSVC)
	target_link_libraries(mdromfix PRIVATE m)
endif()