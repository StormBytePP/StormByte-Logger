option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()
	
	add_executable(LogTests logger_test.cxx)
	target_link_libraries(LogTests StormByte::Logger)
	add_test(NAME LogTests COMMAND LogTests)

	# Separate executable for ThreadedLog unit tests
	add_executable(ThreadedLogTests threaded_log_test.cxx)
	target_link_libraries(ThreadedLogTests StormByte::Logger)
	add_test(NAME ThreadedLogTests COMMAND ThreadedLogTests)

	# Manipulators tests
	add_executable(ManipulatorsTests manipulators_test.cxx)
	target_link_libraries(ManipulatorsTests StormByte::Logger)
	add_test(NAME ManipulatorsTests COMMAND ManipulatorsTests)

	# Format mask tests
	add_executable(FormatMaskTests format_mask_test.cxx)
	target_link_libraries(FormatMaskTests StormByte::Logger)
	add_test(NAME FormatMaskTests COMMAND FormatMaskTests)
endif()