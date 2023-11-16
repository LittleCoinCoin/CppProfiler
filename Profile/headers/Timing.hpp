#pragma once

#if _WIN32
#include <intrin.h>
#include <windows.h>
#else
#include <x86intrin.h>
#include <sys/time.h>
#endif

#include "types.hpp"

namespace Profile
{
	/*!
	@brief A struct to give access to the OS and CPU timers
			and frequencies.
	*/
	static struct Timer
	{
		/*!
		@brief A wrapper to get the current counter in the OS.
		@details On Windows, it uses QueryPerformanceCounter.
				 On Linux, it uses gettimeofday.
		*/
		static u64 GetOSTimer(void);

		/*!
		@brief A wrapper to get the frequency of the OS timer.
		@details On Windows, it uses QueryPerformanceFrequency.
				 On Linux, it returns 1000000.
		*/
		static u64 GetOSTimerFreq(void);

		/*!
		@brief A wrapper to __rdtsc() to get the CPU timer.
		*/
		static u64 GetCPUTimer(void);

		/*!
		@brief Estimates the CPU frequency by waiting for a
				given amount of time.
		@details Of course, the more time you wait, the more
				 accurate the estimation will be. 100ms is a
				 good value to start with.
		@param _msToWait The amount of time to wait in milliseconds.
		*/
		static u64 EstimateCPUFreq(u64 _msToWait);
	};
}