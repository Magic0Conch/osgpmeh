set(OSGPMESH_SRC_DIR "${OSGPMESH_SOURCES_DIR}/src")
set(TEST_NAME "osgpmesh_base")
file(GLOB_RECURSE TEST_SRC "${OSGPMESH_SRC_DIR}/*.cpp")
add_library(${TEST_NAME} STATIC ${TEST_SRC})
target_include_directories(
  ${TEST_NAME}
    PUBLIC
    external
        ${OSG_INC_PATH}
        ${OSGPMESH_INC_PATH}
        "C:/Program Files (x86)/meshoptimizer/include"
)


target_link_libraries(${TEST_NAME}
    PUBLIC
      ${OSG_LIB}
      json11
)

install (TARGETS ${TEST_NAME}
    DESTINATION lib)

add_executable(osgpmesh main.cpp)

find_package(meshoptimizer REQUIRED)
if (meshoptimizer_FOUND)
    message(STATUS "meshoptimizer found!")
else()
    message(FATAL_ERROR "meshoptimizer not found!")
endif()


target_link_libraries(osgpmesh
PRIVATE
osgpmesh_base
"C:/Program Files (x86)/meshoptimizer/lib/meshoptimizer.lib"
)
set_target_properties(osgpmesh PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}
)
