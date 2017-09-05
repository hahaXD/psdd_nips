cmake_minimum_required(VERSION 3.6)
project(psdd)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/psdd_node.cpp src/psdd_node.hpp src/vtree.cpp src/vtree.hpp src/vtree_manager.cpp
        src/vtree_manager.hpp src/psdd_parameter.cpp src/psdd_parameter.hpp src/psdd_manager.cpp src/psdd_manager.hpp
        src/psdd_unique_table.cpp src/psdd_unique_table.hpp src/uai_network.cpp src/uai_network.hpp
        src/psdd_compiler.cpp src/psdd_compiler.hpp src/psdd_element.cpp src/psdd_element.hpp src/optionparser.h)
add_executable(psdd ${SOURCE_FILES} src/main.cpp)
add_executable(arthur ${SOURCE_FILES} src/arthur_main.cpp)