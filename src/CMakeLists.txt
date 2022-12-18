add_compile_definitions(LIBPLATEAU_BUILD)


add_library(plateau SHARED "")
target_link_libraries(plateau PUBLIC citygml xerces-c GLTFSDK crypto ssl "${FBXSDK_LIBRARIES}" LibXml2 "${ZLIB_LIBRARY}")
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_include_directories(plateau PUBLIC "${CMAKE_SOURCE_DIR}/include" "${LIBCITYGML_INCLUDE}" "${GLTFSDK_INCLUDE}" "${CPPHTTPLIB_INCLUDE}" "${FBXSDK_INCLUDE}")
else()
  target_include_directories(plateau PUBLIC "${CMAKE_SOURCE_DIR}/include/geometry" "${CPPHTTPLIB_INCLUDE}" "${FBXSDK_INCLUDE}")
endif()

set_target_properties(plateau PROPERTIES RUNTIME_OUTPUT_DIRECTORY
  ${LIBPLATEAU_BINARY_DIR})

add_subdirectory(geometry)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  add_subdirectory(io)
  add_subdirectory(mesh_writer)
  add_subdirectory(polygon_mesh)
  add_subdirectory(basemap)
  add_subdirectory(c_wrapper)
  add_subdirectory(dataset)
  add_subdirectory(network)
else()
  set_target_properties(plateau PROPERTIES FRAMEWORK TRUE)
endif()

target_link_libraries(plateau PUBLIC c_wrapper)
