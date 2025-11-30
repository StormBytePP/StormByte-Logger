option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST AND NOT STORMBYTE_AS_DEPENDENCY)
	add_executable(LogTests logger_test.cxx)
	target_link_libraries(LogTests StormByte::Logger)
	add_test(NAME LogTests COMMAND LogTests)

	# Separate executable for ThreadedLog unit tests
	add_executable(ThreadedLogTests threaded_log_test.cxx)
	target_link_libraries(ThreadedLogTests StormByte::Logger)
	add_test(NAME ThreadedLogTests COMMAND ThreadedLogTests)
endif()