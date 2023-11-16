// Profiler.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "Profiler.hpp"

void TestFunction(u64 _arr[], u64 _count)
{
	PROFILE_FUNCTION_TIME(0);

	for (u64 i = 0; i < _count; ++i)
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
	profiler.tracks[0].Initialize();

	std::cout << "Hello CMake." << std::endl;
	u64 arr[8192];
	TestFunction(arr, 8192);

	profiler.tracks[0].End();
	profiler.End();
	profiler.Report();

	return 0;
}
