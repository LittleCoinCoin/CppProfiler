#include "Profile/Profiler.hpp"

/*!
@brief Tests the macro time profiling macro on track 0: PROFILE_FUNCTION_TIME(0).
@details Fills an array with the index of the element.
@param _arr The array to fill.
@param _count The number of elements to fill.
*/
void TestFunction(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME(0);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

/*!
@brief Tests the macro time and bandwidth profiling macro on track 0: PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u64) * _count).
@details Fills an array with the index of the element.
@param _arr The array to fill.
@param _count The number of elements to fill.
*/
void TestFunction_Bandwidth(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u64) * _count);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

/*!
@brief Tests the macro time profiling macro on track 1: PROFILE_FUNCTION_TIME(1).
@details Fills an array with the index of the element.
@param _arr The array to fill.
@param _count The number of elements to fill.
*/
void TestFunction_Track2(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME(1);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

/*!
@brief Tests the overflow of tracks.
@details Adds NB_TRACKS - _profiler.trackCount + 1 subtracks to overflow NB_TRACKS
		 maximum tracks when the profiler has already been added _profiler.trackCount tracks.
@param _profiler The profiler to test.
*/
void TestFunction_OverflowTracks(Profile::Profiler& _profiler)
{
	printf("Adding %u subtracks to overflow %u maximum tracks when %u have already been added.\n",
		NB_TRACKS - _profiler.trackCount + 1, NB_TRACKS, _profiler.trackCount);
	for (Profile::u8 trackIdx = _profiler.trackCount; trackIdx <= NB_TRACKS; ++trackIdx)
	{
		_profiler.AddTrack("SubtrackToOverflow");
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

	//This should lead to a message in the application that the track could not
	//be added because only NB_TRACKS tracks are allowed.
	TestFunction_OverflowTracks(profiler);

	profiler.End();
	profiler.Report();

	return 0;
}