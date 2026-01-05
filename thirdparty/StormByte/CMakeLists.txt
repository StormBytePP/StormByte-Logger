if(WIN32)
	set(WITH_STORMBYTE "BUNDLED" CACHE STRING "Use StormByte components (SYSTEM/BUNDLED)" FORCE)
else()
	set(WITH_STORMBYTE "BUNDLED" CACHE STRING "Use StormByte components (SYSTEM/BUNDLED)")
endif()
if(WITH_STORMBYTE STREQUAL "SYSTEM")
	find_package(StormByte REQUIRED)
else()
	set(STORMBYTE_BASE_SRCDIR "${CMAKE_CURRENT_LIST_DIR}/base")
	ensure_build_dir(STORMBYTE_BASE_BUILDDIR)
	set(STORMBYTE_BASE_COMPONENT "StormByte Suite - Base")
	set(STORMBYTE_BASE_OPTIONS "")
	create_cmake_component(
		STORMBYTE_BASE_LIBRARY_CREATE_FILE
		"StormByte"
		"${STORMBYTE_BASE_COMPONENT}"
		"${STORMBYTE_BASE_SRCDIR}"
		"${STORMBYTE_BASE_BUILDDIR}"
		"${STORMBYTE_BASE_OPTIONS}"
		"shared"
		"StormByte"
		1
	)
	include("${STORMBYTE_BASE_LIBRARY_CREATE_FILE}")
	add_dependencies(StormByte-Logger StormByte_install)
endif()

cmake_policy(SET CMP0079 NEW)
target_link_libraries(StormByte-Logger PUBLIC StormByte)