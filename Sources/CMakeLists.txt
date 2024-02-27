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
)


target_link_libraries(${TEST_NAME}
    PUBLIC
      ${OSG_LIB}
      json11
)

install (TARGETS ${TEST_NAME}
    DESTINATION lib)

add_executable(osgpmesh main.cpp)
target_link_libraries(osgpmesh
PRIVATE
osgpmesh_base

)
set_target_properties(osgpmesh PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}
)
