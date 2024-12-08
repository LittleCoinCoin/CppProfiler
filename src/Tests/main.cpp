#include <filesystem>
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
@brief Tests the macro time profiling macro on track 0: PROFILE_BLOCK_TIME("TestFunction_ProfileBlock_Write", 0).
@details Fills an array with the index of the element. Profiling happens within
		 the loop filling the array.
@param _arr The array to fill.
@param _count The number of elements to fill.
*/
void TestFunction_ProfileBlock(Profile::u64 _arr[], Profile::u64 _count)
{
	for (Profile::u64 i = 0; i < _count; ++i)
	{
		PROFILE_BLOCK_TIME(TestFunction_ProfileBlock_Write, 0);
		_arr[i] = i;
	}
}

/*!
@brief A wrapper around the TestFunction_ProfileFunction when it will be used in repetition tester.
*/
struct RepetitionTest_TestFunction_ProfileFunction : public Profile::RepetitionTest
{
	Profile::u64* arr = nullptr;
	Profile::u64 count = 0;
	RepetitionTest_TestFunction_ProfileFunction(Profile::u64* _arr, Profile::u64 _count) : arr(_arr), count(_count) {}
	RepetitionTest_TestFunction_ProfileFunction(const char* _name, Profile::u64* _arr, Profile::u64 _count) : RepetitionTest(_name), arr(_arr), count(_count) {}

	inline void operator()() override
	{
		TestFunction_ProfileFunction(arr, count);
	}
};

/*!
@brief A wrapper around the TestFunction_ProfileBlock when it will be used in repetition tester.
*/
struct RepetitionTest_TestFunction_ProfileBlock : public Profile::RepetitionTest
{
	Profile::u64* arr = nullptr;
	Profile::u64 count = 0;
	RepetitionTest_TestFunction_ProfileBlock(Profile::u64* _arr, Profile::u64 _count) : arr(_arr), count(_count) {}
	RepetitionTest_TestFunction_ProfileBlock(const char* _name, Profile::u64* _arr, Profile::u64 _count) : RepetitionTest(_name), arr(_arr), count(_count) {}

	inline void operator()() override
	{
		TestFunction_ProfileBlock(arr, count);
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
@brief A wrapper around the TestFunction_Bandwidth when it will be used in repetition tester.
@details In this one we are consistently reallocating a new array of 1MB at the
		 beginning to trigger page faults and see them in the repetition profiling report.
*/
struct RepetitionTest_TestFunction_Bandwidth : public Profile::RepetitionTest
{
	RepetitionTest_TestFunction_Bandwidth() = default;
	RepetitionTest_TestFunction_Bandwidth(const char* _name) : RepetitionTest(_name) {}

	inline void operator()() override
	{
		Profile::u64 arraySize = 1024 * 1024;
		Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * arraySize);
		TestFunction_Bandwidth(arr, arraySize);
		free(arr);
	}
};

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
@brief Tests the trigger of page fault information.
@details Allocates a new large array and fills it with values. This should trigger
		 page faults. The page fault count is then reported through the Report.
*/
void TestFunction_PageFaultCounter()
{
	Profile::u64 arraySize = 1024 * 1024;
	Profile::u8* arr = (Profile::u8*)malloc(sizeof(Profile::u8) * arraySize);

	PROFILE_FUNCTION_TIME_BANDWIDTH(0, sizeof(Profile::u8) * arraySize);

	if (arr)
	{
		for (Profile::u64 i = 0; i < arraySize; ++i)
		{
			arr[i] = (Profile::u8)i;
		}
	}
	else
	{
		printf("ERROR: Could not malloc %llu bytes in TestFunction_PageFaultCounter", arraySize);
	}
	free(arr);
}

void TestFunction_BestPerfSearch()
{

	Profile::ProfilerResults* results = new Profile::ProfilerResults[2];
	
	Profile::RepetitionProfiler* repetitionProfiler = new Profile::RepetitionProfiler();
	
	Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * 8192);
	RepetitionTest_TestFunction_ProfileBlock repetitiontest("TestFunction_ProfileBlock", arr, 8192);
	RepetitionTest_TestFunction_ProfileFunction repetitiontest2(arr, 8192);
	repetitionProfiler->PushBackRepetitionTest(&repetitiontest);
	repetitionProfiler->PushBackRepetitionTest(&repetitiontest2);

	repetitionProfiler->SetRepetitionResults(results);
	repetitionProfiler->BestPerfSearchRepetitionTesting(3, false, true, 10);

	free(arr);
	delete[] results;
	delete repetitionProfiler;
}


/*!
@brief Tests the FixedCountRepetitionTesting function of the RepetitionProfiler.
@details Repeatedly tests the function a hundred times and reports the results.
*/
void TestFunction_FixedRepetitionTesting()
{
	Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * 8192);

	Profile::u16 repetitionCount = 10;
	Profile::ProfilerResults* results = new Profile::ProfilerResults[repetitionCount];
	Profile::RepetitionProfiler* repetitionProfiler = new Profile::RepetitionProfiler();
	
	RepetitionTest_TestFunction_ProfileFunction repetitiontest(arr, 8192);
	RepetitionTest_TestFunction_Bandwidth repetitionTest2("Page fault triggering");
	repetitionProfiler->PushBackRepetitionTest(&repetitiontest);
	repetitionProfiler->PushBackRepetitionTest(&repetitionTest2);

	repetitionProfiler->SetRepetitionResults(results);
	repetitionProfiler->FixedCountRepetitionTesting(repetitionCount);

	//Test exporting as CSV
	//Create file directory ./ProfileResults/Summary and ./ProfileResults/Repetitions
	if (std::filesystem::create_directories("./ProfileResults/Summary"))
	{
		printf("\nCreating directory ./ProfileResults/Summary\n");
	}
	else
	{
		printf("\nDirectory ./ProfileResults/Summary already exists\n");
	}

	if (std::filesystem::create_directories("./ProfileResults/Repetitions"))
	{
		printf("\nCreating directory ./ProfileResults/Repetitions\n");
	}
	else
	{
		printf("\nDirectory ./ProfileResults/Repetitions already exists\n");
	}
	repetitionProfiler->ExportToCSV("./ProfileResults", repetitionCount);

	delete[] results;
	delete repetitionProfiler;
	free(arr);
}

int main()
{
	Profile::u64 testArraySize = 1024 * 1024;

	Profile::Profiler* profiler = new Profile::Profiler();
	profiler->SetProfilerName("Tests");

	Profile::SetProfiler(profiler);
	profiler->SetTrackName(0, "Main");
	profiler->Initialize();
	Profile::u64* arr = (Profile::u64*)malloc(sizeof(Profile::u64) * testArraySize);

	TestFunction_ProfileFunction(arr, testArraySize);
	TestFunction_ProfileBlock(arr, testArraySize);
	TestFunction_Bandwidth(arr, testArraySize);
	TestFunction_PageFaultCounter();

	profiler->End();
	profiler->Report();
	profiler->ClearTracks();

	profiler->SetTrackName(1, "SubTrack");
	profiler->Initialize();
	TestFunction_Track2(arr, testArraySize);

	profiler->End();
	profiler->Report();
	profiler->ClearTracks();

	profiler->SetTrackName(0, "Main");
	profiler->Initialize();

	TestFunction_ProfileFunction(arr, testArraySize);
	TestFunction_ProfileBlock(arr, testArraySize);
	TestFunction_Bandwidth(arr, testArraySize);

	profiler->End();
	profiler->Report();

	//Test exporting as CSV
	//Create file directory ./ProfileResults/TestResults.csv
	if (std::filesystem::create_directories("./ProfileResults"))
	{
		printf("\nCreating directory ./ProfileResults\n");
	}
	else
	{
		printf("\nDirectory ./ProfileResults already exists\n");
	}

	//Export
	profiler->ExportToCSV("./ProfileResults/TestResults.csv");

	profiler->ClearTracks();

	TestFunction_FixedRepetitionTesting();

	// Run the repetition profiling a second time to check that the internal 
	// reset works appropriately. The profiling results should be close to the
	// first run.
	TestFunction_FixedRepetitionTesting();

	TestFunction_BestPerfSearch();

	
	free(arr);
	delete profiler;

	return 0;
}