#include "Profile/Profiler.hpp"

/*!
@brief Tests the macro time profiling macro on track 0: PROFILE_FUNCTION_TIME(0).
@details Fills an array with the index of the element.
@param _arr The array to fill.
@param _count The number of elements to fill.
*/
void TestFunction_ProfileFunction(Profile::u64 _arr[], Profile::u64 _count)
{
	PROFILE_FUNCTION_TIME(0);

	for (Profile::u64 i = 0; i < _count; ++i)
	{
		_arr[i] = i;
	}
}

/*!
@brief A wrapper around the TestFunction_ProfileFunction when it will be used TestFunction_FixedRepetitionTesting.
*/
struct RepetitionTest_TestFunction_ProfileFunction : public Profile::RepetitionTest
{
	Profile::u64* arr = nullptr;
	Profile::u64 count = 0;
	RepetitionTest_TestFunction_ProfileFunction(Profile::u64* _arr, Profile::u64 _count) : arr(_arr), count(_count) {}

	inline void operator()() override
	{
		TestFunction_ProfileFunction(arr, count);
	}
};

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
@brief Tests the FixedCountRepetitionTesting function of the RepetitionProfiler.
@details Repeatedly tests the function a hundred times and reports the results.
*/
void TestFunction_FixedRepetitionTesting()
{
	Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * 8192);
	RepetitionTest_TestFunction_ProfileFunction repetitionTest(arr, 8192);

	Profile::u8 repetitionCount = 100;
	Profile::RepetitionProfiler* repetitionProfiler = (Profile::RepetitionProfiler*)calloc(1, sizeof(Profile::RepetitionProfiler));
	Profile::ProfilerResults* results = (Profile::ProfilerResults*)calloc(repetitionCount, sizeof(Profile::ProfilerResults));

	repetitionProfiler->SetRepetitionResults(results);
	repetitionProfiler->FixedCountRepetitionTesting(repetitionCount, repetitionTest);
	repetitionProfiler->ComputeAverageResults(repetitionCount);
	repetitionProfiler->Report(repetitionCount);

	free(results);
	free(repetitionProfiler);
	free(arr);
}

int main()
{
	//Profile::Profiler profiler("Tests");
	Profile::Profiler* profiler = (Profile::Profiler*)calloc(1, sizeof(Profile::Profiler));
	profiler->SetProfilerName("Tests");

	Profile::SetProfiler(profiler);
	profiler->SetTrackName(0, "Main");
	profiler->Initialize();
	Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * 8192);

	TestFunction_ProfileFunction(arr, 8192);
	TestFunction_Bandwidth(arr, 8192);

	profiler->SetTrackName(1, "SubTrack");
	TestFunction_Track2(arr, 8192);

	profiler->End();
	profiler->Report();

	TestFunction_FixedRepetitionTesting();
	
	free(arr);
	free(profiler);

	return 0;
}