project(html-read)
cmake_minimum_required(VERSION 3.12)

add_library( htmlRead STATIC 
	src/entities.cc
	src/htmlRead.cc
	src/httpDown.cc
)
