#pragma once

#ifndef PROFILE_API
	#ifdef _WIN32
		#ifdef BUILD_PROFILER_LIB
			#define PROFILE_API __declspec(dllexport)
		#elif USE_PROFILER_LIB
			#define PROFILE_API __declspec(dllimport)
		#else
			#define PROFILE_API
		#endif
	#else
		#if defined(BUILD_PROFILER_LIB) || defined(USE_PROFILER_LIB)
			#define PROFILE_API __attribute__((__visibility__("default")))
		#else
			#define PROFILE_API
		#endif
	#endif
#endif