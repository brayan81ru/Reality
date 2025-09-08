cmake_minimum_required(VERSION 3.31)

project(Reality)

set(CMAKE_CXX_STANDARD 20)

# Set third_party as base for external projects
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/Engine/ThirdParty)


# Add all subprojects
add_subdirectory(Engine)
add_subdirectory(Samples/Sandbox)


