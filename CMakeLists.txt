cmake_minimum_required(VERSION 3.28)
project(Pathfinder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
endif()

find_package(nlohmann_json REQUIRED)

if(NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
    if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Moves)
        add_custom_target(clear_moves
                COMMAND rm -r "${CMAKE_CURRENT_BINARY_DIR}/Moves"
        )
    endif()

    add_custom_target(copy_moves
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Moves ${CMAKE_CURRENT_BINARY_DIR}/Moves
    )

    add_custom_target(copy_examples
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/docs/examples ${CMAKE_CURRENT_BINARY_DIR}/docs/examples
    )
endif()

add_executable(Pathfinder
        pathfinder/main.cpp
        pathfinder/coordtensor/debug_util.h
        pathfinder/modules/ModuleManager.h
        pathfinder/modules/ModuleManager.cpp
        pathfinder/moves/MoveManager.h
        pathfinder/moves/MoveManager.cpp
        pathfinder/lattice/Lattice.h
        pathfinder/lattice/Lattice.cpp
        pathfinder/lattice/LatticeSetup.h
        pathfinder/lattice/LatticeSetup.cpp
        pathfinder/moves/Scenario.h
        pathfinder/moves/Scenario.cpp
        pathfinder/search/ConfigurationSpace.h
        pathfinder/search/ConfigurationSpace.cpp
        pathfinder/search/SearchAnalysis.h
        pathfinder/search/SearchAnalysis.cpp
        pathfinder/modules/Metamodule.h
        pathfinder/modules/Metamodule.cpp
        pathfinder/moves/Isometry.h
        pathfinder/moves/Isometry.cpp
        pathfinder/properties/Colors.h
        pathfinder/properties/Colors.cpp
        pathfinder/search/HeuristicCache.cpp
        pathfinder/search/HeuristicCache.h
        pathfinder/modules/ModuleProperties.cpp
        pathfinder/modules/ModuleProperties.h)

if(NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
    add_dependencies(Pathfinder copy_moves copy_examples)
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/Moves")
        add_dependencies(copy_moves clear_moves)
    endif()
endif()