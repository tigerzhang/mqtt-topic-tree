cmake_minimum_required(VERSION 3.5)
project(mqtt_topic_tree)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -ggdb3")

set(SOURCE_FILES
        src/TopicTreeNode.h
        src/TopicTreeNode.cc
        src/test.cc)

add_executable(mqtt_topic_tree ${SOURCE_FILES})
