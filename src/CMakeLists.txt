#Edited cmake-sfml-project CMakelists.txt template (https://github.com/SFML/cmake-sfml-project)

cmake_minimum_required(VERSION 3.28)
project(FluidSimulationSandbox LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

set(MY_EXE "fluid_simulation_sandbox")
add_executable(${MY_EXE} "./main.cpp"
    "./particle.h"
    "./fluid_sandbox.h" "./fluid_sandbox.cpp"
    "./spatial_hash_grid.h" "./spatial_hash_grid.cpp"
)

set_property(TARGET ${MY_EXE} PROPERTY CXX_STANDARD 23)
target_link_libraries(${MY_EXE} PRIVATE SFML::Graphics SFML::Window SFML::System)
