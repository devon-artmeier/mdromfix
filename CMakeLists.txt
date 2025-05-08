cmake_minimum_required(VERSION 3.16)
project(mdromfix LANGUAGES CXX)

add_executable(mdromfix
	"src/helpers.cpp"
	"src/main.cpp")

if(MSVC)
	add_definitions("/wd6385")
endif()