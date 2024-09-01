set(Target_Name "geocode-shell")
file(GLOB_RECURSE geocode_SRCS *.cpp *.h)
add_executable(${Target_Name} ${geocode_SRCS} )

target_link_libraries(${Target_Name} PRIVATE lib readline)
set_target_properties(${Target_Name} PROPERTIES OUTPUT_NAME geocodesh)
install(TARGETS ${Target_Name} DESTINATION bin)
