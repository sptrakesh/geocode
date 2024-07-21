set(Target_Name "lib")
file(GLOB_RECURSE geocode_SRCS *.cpp *.h)
add_library(${Target_Name} STATIC ${geocode_SRCS})
add_library(geocode::lib ALIAS ${Target_Name})
set_target_properties(${Target_Name} PROPERTIES OUTPUT_NAME geocode)
target_compile_definitions(${Target_Name} PRIVATE _FORTIFY_SOURCE=2)
target_include_directories(${Target_Name}
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
if (UNIX)
  if (APPLE)
    target_link_libraries(${Target_Name} INTERFACE nanolog Boost::json cpr::cpr ${JEMALLOC_LIBRARY})
  else (APPLE)
    target_link_libraries(${Target_Name} INTERFACE nanolog Boost::json cpr::cpr jemalloc)
  endif (APPLE)
else (UNIX)
  target_link_libraries(${Target_Name} INTERFACE nanolog Boost::json cpr::cpr)
endif (UNIX)
install(TARGETS ${Target_Name} EXPORT GeocodeLibTargets DESTINATION lib)
install(TARGETS ${Target_Name} DESTINATION lib)
install (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/lib/geocode
  DESTINATION include
  FILES_MATCHING PATTERN "*.h*")
install(EXPORT GeocodeLibTargets
  FILE GeocodeLibTargets.cmake
  NAMESPACE geocode::
  DESTINATION lib/cmake/Geocode
)
