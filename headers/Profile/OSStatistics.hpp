#pragma once

#if _WIN32
#include <intrin.h> //for __rdtsc
#include <windows.h> //for QueryPerformanceCounter
#include <psapi.h> //for GetProcessMemoryInfo
#elif __ARM_ARCH // aimed at arm MacOs, but should work on any arm linux
#include <time.h> //for clock_gettime
#else // non-arm linux
#include <x86intrin.h> //for __rdtsc
#include <sys/time.h> //for gettimeofday
#include <sys/resource.h> //for getrusage
#include <unistd.h> //for getpagesize
#endif

#include "Export.hpp"
#include "Types.hpp"

namespace Profile
{
	/*!
	@brief A struct to give access to internal statistics such memory or
			performance related
	*/
	struct Surveyor
	{
		/*!
		@brief Utility struct to manipulate OS memory statistics.
		*/
		struct os_metrics
		{
			b32 Initialized;
			HANDLE ProcessHandle;
		};
		
		/*!
		@brief An instance of os_metrics to store required handle information to
				query memory statistics.
		*/
		static os_metrics GlobalMetrics;

		/*!
		@brief Gets the number of page faults that have occurred since the start
				of the process.
		*/
		PROFILE_API static u64 GetOSPageFaultCount();
		
		/*!
		@brief Gets the size of a page in the OS.
		*/
		PROFILE_API static u64 GetOSPageSize();

		/*!
		@brief On windows, it initializes the process handle to query memory statistics
			   (see ::GlobalMetrics::ProcessHandle). On linux or mac, it does nothing.
		*/
		PROFILE_API static void InitializeOSMetrics();
	};

	/*!
	@brief A struct to give access to the OS and CPU timers
			and frequencies.
	*/
	struct Timer
	{
		/*!
		@brief The estimated CPU frequency.
		@details This value is pre-computed once in the cpp file (so at compile time).
				 It is used to estimate the CPU frequency at runtime. You can also
				 recompute it at runtime by calling ::SetEstimatedCPUFreq.
		@see ::EstimateCPUFreq, ::SetEstimatedCPUFreq, ::GetEstimatedCPUFreq
		*/
		static u64 s_estimatedCPUFreq;

		/*!
		@brief A wrapper to __rdtsc() to get the CPU timer.
		*/
		PROFILE_API static u64 GetCPUTimer(void);
		
		/*!
		@brief Returns an estimated CPU frequency. You must call ::SetEstimatedCPUFreq
				at least once before calling this function.
		*/
		PROFILE_API static u64 GetEstimatedCPUFreq();
		
		/*!
		@brief A wrapper to get the current counter in the OS.
		@details On Windows, it uses QueryPerformanceCounter.
				 On Linux, it uses gettimeofday.
		*/
		PROFILE_API static u64 GetOSTimer(void);

		/*!
		@brief A wrapper to get the frequency of the OS timer.
		@details On Windows, it uses QueryPerformanceFrequency.
				 On Linux, it returns 1000000.
		*/
		PROFILE_API static u64 GetOSTimerFreq(void);

		/*!
		@brief Sets the value of the estimated CPU frequency (::s_estimatedCPUFreq).
		@param _msToWait The amount of time to wait in milliseconds. Default is 1000 (1 second).
		*/
		PROFILE_API static void SetEstimatedCPUFreq(u64 _msToWait = 1000);

		/*!
		@brief Estimates the CPU frequency by waiting for a
				given amount of time.
		@details Of course, the more time you wait, the more
				 accurate the estimation will be. 100ms is a
				 good value to start with.
		@param _msToWait The amount of time to wait in milliseconds.
		*/
		PROFILE_API static u64 EstimateCPUFreq(u64 _msToWait);
	};
}