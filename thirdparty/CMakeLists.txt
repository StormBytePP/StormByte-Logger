# Initialize StormByte BuildMaster
add_subdirectory(buildmaster)
include(buildmaster/propagate_vars.cmake)

# Construct StormByte Dependencies
add_subdirectory(StormByte)