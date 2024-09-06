#include "Profile/OSStatistics.hpp"

Profile::Surveyor::os_metrics Profile::Surveyor::GlobalMetrics = {};

Profile::u64 Profile::Surveyor::ReadOSPageFaultCount(void)
{
#if _WIN32
	PROCESS_MEMORY_COUNTERS_EX MemoryCounters = {};
	MemoryCounters.cb = sizeof(MemoryCounters);
	GetProcessMemoryInfo(GlobalMetrics.ProcessHandle, (PROCESS_MEMORY_COUNTERS *)&MemoryCounters, sizeof(MemoryCounters));
	u64 Result = MemoryCounters.PageFaultCount;
	return Result;
#else
	// NOTE: ru_minflt  the number of page faults serviced without any I/O activity.
    //       ru_majflt  the number of page faults serviced that required I/O activity.
    struct rusage Usage = {};
    getrusage(RUSAGE_SELF, &Usage);
    int Result = Usage.ru_minflt + Usage.ru_majflt;
    return Result;
#endif
}

void Profile::Surveyor::InitializeOSMetrics(void)
{
#if _WIN32
	if(!GlobalMetrics.Initialized)
	{
		GlobalMetrics.Initialized = true;
		GlobalMetrics.ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	}
#endif
}

Profile::u64 Profile::Timer::GetOSTimerFreq(void)
{
#if _WIN32
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
#else
	return 1000000;
#endif
}

Profile::u64 Profile::Timer::GetOSTimer(void)
{
#if _WIN32
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
#elif __ARM_ARCH
	struct timespec value;
	clock_gettime(CLOCK_MONOTONIC, &value);
	return GetOSTimerFreq() * (u64)value.tv_sec * + (u64)value.tv_nsec;
#else
	struct timeval value;
	gettimeofday(&value, 0);
	return GetOSTimerFreq() * (u64)value.tv_sec * + (u64)value.tv_usec;
#endif
}

Profile::u64 Profile::Timer::GetCPUTimer(void)
{
#if __ARM_ARCH
	struct timespec value;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &value);
	return (u64)value.tv_sec * 1000000000 + (u64)value.tv_nsec;
#else
	return __rdtsc();
#endif
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

Profile::u64 Profile::Timer::s_estimatedCPUFreq = Profile::Timer::EstimateCPUFreq(1000);

Profile::u64 Profile::Timer::GetEstimatedCPUFreq()
{
	return s_estimatedCPUFreq;
}

void Profile::Timer::SetEstimatedCPUFreq(u64 _msToWait)
{
	s_estimatedCPUFreq = EstimateCPUFreq(_msToWait);
}
