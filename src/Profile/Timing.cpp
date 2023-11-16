#include "Profile/Timing.hpp"

#if _WIN32

Profile::u64 Profile::Timer::GetOSTimerFreq(void)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

Profile::u64 Profile::Timer::GetOSTimer(void)
{
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
}

#else

Profile::u64 Profile::Timer::GetOSTimerFreq(void)
{
	return 1000000;
}

Profile::u64 Profile::Timer::GetOSTimer(void)
{
	struct timeval value;
	gettimeofday(&value, 0);

	Profile::u64 Result = Profile::Timer::GetOSTimerFreq() * (Profile::u64)value.tv_sec + (Profile::u64)value.tv_usec;
	return Result;
}

#endif

Profile::u64 Profile::Timer::GetCPUTimer(void)
{
	return __rdtsc();
}


Profile::u64 Profile::Timer::EstimateCPUFreq(u64 _msToWait)
{
	u64 OSFreq = GetOSTimerFreq();
	u64 CPUStart = GetCPUTimer();
	u64 OSStart = GetOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * _msToWait / 1000;
	while (OSElapsed < OSWaitTime)
	{
		OSEnd = GetOSTimer();
		OSElapsed = OSEnd - OSStart;
	}

	u64 CPUEnd = GetCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;
	u64 CPUFreq = 0;
	if (OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}

	return CPUFreq;
}
