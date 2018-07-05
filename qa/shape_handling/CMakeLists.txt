# CMake build file for the tests for shape handling

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(qa_shape_handling
    test_module.cpp
    test_detect_chain.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/polygon
    ${CMAKE_SOURCE_DIR}/common/geometry
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(qa_shape_handling
    polygon
    common
    polygon
    bitmaps
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${wxWidgets_LIBRARIES}
)

# add_dependencies(qa_shape_handling pcbnew)

add_test(shape_handling qa_shape_handling)