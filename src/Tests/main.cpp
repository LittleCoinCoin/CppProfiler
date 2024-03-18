#include "Profile/Profiler.hpp"

void TestFunction(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME(0);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

void TestFunction_Bandwidth(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u64) * _count);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

void TestFunction_Track2(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME(1);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

int main()
{
	Profile::Profiler profiler("Tests");
	Profile::SetProfiler(&profiler);
	profiler.AddTrack("main");
	profiler.Initialize();
	Profile::u64 arr[8192];

	profiler.tracks[0].Initialize();
	TestFunction(arr, 8192);
	TestFunction_Bandwidth(arr, 8192);
	profiler.tracks[0].End();

	profiler.AddTrack("subtrack");
	profiler.tracks[1].Initialize();
	TestFunction_Track2(arr, 8192);
	profiler.tracks[1].End();

	profiler.End();
	profiler.Report();

	return 0;
}

PROFILER_END_CHECK;
