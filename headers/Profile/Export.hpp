#pragma once

#if BUILD_PROFILER_LIB
	#if HAVE_VISIBILITY
		#define PROFILE_API __attribute__((__visibility__("default")))
	#elif (defined _WIN32 && !defined __CYGWIN__) && BUILD_PROFILER_LIB
		#define PROFILE_API __declspec(dllexport)
	#elif (defined _WIN32 && !defined __CYGWIN__)
		#define PROFILE_API __declspec(dllimport)
	#endif
#else
	#define PROFILE_API
#endif