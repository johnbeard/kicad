
add_library( version
    build_version.cpp
)

target_include_directories( version
    PRIVATE ${INC_BEFORE} ${INC_AFTER}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries( version
    ${wxWidgets_LIBRARIES}
)

# KiCad build version string defaults to "no-vcs-found" which forces the build version header
# command to look for git to create the version string header when the .git path is found in
# the source path.
set( KICAD_BRANCH_NAME "" CACHE STRING "KiCad repository name." )
set( KICAD_VERSION_EXTRA "" CACHE STRING
    "User defined configuration string to append to KiCad version." )

# Generate version header file.
add_custom_target(
    version_header ALL
    COMMAND ${CMAKE_COMMAND}
        -DKICAD_VERSION=${KICAD_VERSION}
        -DKICAD_BRANCH_NAME=${KICAD_BRANCH_NAME}
        -DKICAD_VERSION_EXTRA=${KICAD_VERSION_EXTRA}
        -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/kicad_build_version.h
        -DSRC_PATH=${PROJECT_SOURCE_DIR}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -P ${CMAKE_MODULE_PATH}/WriteVersionHeader.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating version string header"
    )

add_dependencies( version version_header )