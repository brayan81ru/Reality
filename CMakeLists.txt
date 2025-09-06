cmake_minimum_required(VERSION 3.31)

project(Reality)

set(CMAKE_CXX_STANDARD 20)

# Set third_party as base for external projects
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/engine/ThirdParty)

# Add all subprojects
add_subdirectory(engine/ThirdParty/DiligentEngine)
add_subdirectory(engine)
add_subdirectory(Samples/sandbox)


