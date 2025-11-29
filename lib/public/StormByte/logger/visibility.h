#pragma once

#include <StormByte/platform.h>

#ifdef WINDOWS
	#ifdef StormByte_Logger_EXPORTS
		#define STORMBYTE_LOGGER_PUBLIC	__declspec(dllexport)
  	#else
      	#define STORMBYTE_LOGGER_PUBLIC	__declspec(dllimport)
  	#endif
  	#define STORMBYTE_LOGGER_PRIVATE
#else
    #define STORMBYTE_LOGGER_PUBLIC		__attribute__ ((visibility ("default")))
    #define STORMBYTE_LOGGER_PRIVATE	__attribute__ ((visibility ("hidden")))
#endif
