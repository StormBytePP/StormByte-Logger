include(GNUInstallDirs)

file(GLOB_RECURSE STORMBYTE_LOGGER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

add_library(StormByte-Logger SHARED ${STORMBYTE_LOGGER_SOURCES})
add_library(StormByte::Logger ALIAS StormByte-Logger)

set_target_properties(StormByte-Logger PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION ${CMAKE_PROJECT_VERSION}
	VERSION ${CMAKE_PROJECT_VERSION}
	POSITION_INDEPENDENT_CODE ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

if(MSVC)
	target_compile_options(StormByte-Logger PRIVATE
		/EHsc
		$<$<CONFIG:Release>:/O2>
		$<$<CONFIG:Debug>:/Od>
	)
	# Do NOT add /LTCG /GL here — breaks lld-link; use CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE instead
else()
	target_compile_options(StormByte-Logger PRIVATE
		$<$<CONFIG:Debug>:-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0>
	)
endif()

target_include_directories(StormByte-Logger
	SYSTEM BEFORE PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/public"
		"${CMAKE_CURRENT_LIST_DIR}/private"
)
