project(surface2volume)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${surface2volume_SOURCE_DIR}/config")

#Enable SSE instructions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2 -msse3")

#Enable C++11 standard
add_definitions(-std=c++11)

find_package(Vigra REQUIRED)
include_directories(${VIGRA_INCLUDE_DIR})

find_package(HDF5 REQUIRED)
add_definitions(${HDF5_DEFINITIONS})

find_package(Boost COMPONENTS program_options regex REQUIRED)
include_directories(${BOOST_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(fastbvh SHARED
    fastbvh/BBox.cpp
    fastbvh/BVH.cpp
    Triangle.cpp
)

add_executable(surface2volume
    Mesh.cpp
    OBJReader.cpp
    Scene.cpp
    surface2volume.cpp)
target_link_libraries(surface2volume
    fastbvh
    ${VIGRA_IMPEX_LIBRARY}
    ${HDF5_C_LIBRARIES} 
    ${HDF5_CXX_LIBRARIES} 
    ${HDF5_HL_LIBRARIES} 
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    hdf5_hl
)
get_property(location TARGET surface2volume PROPERTY LOCATION)
add_custom_command(TARGET surface2volume
    POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        ${location}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(test
    test.cpp)
target_link_libraries(test
    fastbvh
)
get_property(location TARGET test PROPERTY LOCATION)
add_custom_command(TARGET test
    POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        ${location}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
