#include "Profile/Profiler.hpp"

static Profile::Profiler* s_Profiler = nullptr;

PROFILE_API void Profile::SetProfiler(Profiler* _profiler)
{
	s_Profiler = _profiler;
}

#if PROFILER_ENABLED

Profile::ProfileBlock::ProfileBlock(NB_TRACKS_TYPE _trackIdx, NB_TIMINGS_TYPE _profileResultIdx, u64 _byteCount) :
	trackIdx(_trackIdx), profileResultIdx(_profileResultIdx)
{
	s_Profiler->OpenBlock(trackIdx, profileResultIdx, _byteCount);
}

Profile::ProfileBlock::~ProfileBlock()
{
	s_Profiler->CloseBlock(trackIdx, profileResultIdx);
}

#endif

void Profile::ProfileTrack::End() noexcept
{
	elapsed = Timer::GetCPUTimer() - start;
}

void Profile::ProfileTrack::Initialize() noexcept
{
	start = Timer::GetCPUTimer();
}

void Profile::ProfileTrack::Report(u64 _totalElapsedReference) noexcept
{
	f64 elapsedSec = (f64)elapsed / (f64)Timer::EstimateCPUFreq(100);
	printf("\n---- Profile Track: %s (%fms; %.2f%% of total) ----\n", name, 1000 * elapsedSec, 100.0f * (f64)elapsed / (f64)_totalElapsedReference);
	
	static f64 megaByte = 1<<20;
	static f64 gigaByte = 1<<30;

	for (ProfileResult& result : timings)
	{
		if (result.blockName != nullptr)
		{
			printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
				result.blockName, result.hitCount, result.elapsed, 100.0f * (f64)result.elapsed / (f64)elapsed,
				100.0f * (f64)result.elapsed / (f64)_totalElapsedReference);
			if (result.processedByteCount > 0)
			{
				f64 bandwidth = (f64)result.processedByteCount / (((f64)result.elapsed / (f64)elapsed) * elapsedSec);
				printf("; %.2fMB at %.2fMB/s | %.2fGB/s", (f64)result.processedByteCount / megaByte, bandwidth / megaByte, bandwidth / gigaByte);
			}
			printf(")\n");
		}
	}
}

NB_TIMINGS_TYPE Profile::Profiler::GetProfileResultIndex(NB_TRACKS_TYPE _trackIdx, const char* _fileName, u32 _lineNumber, const char* _blockName)
{
	NB_TIMINGS_TYPE profileResultIndex = Hash(_fileName, _lineNumber) % NB_TIMINGS;

	ProfileResult* profileResult = &s_Profiler->tracks[_trackIdx].timings[profileResultIndex];
	ProfileResult* InitialprofileResult = profileResult;

	// no need to compare _fileName and LinenNumber values, because for
	// each (_fileName, _lineNumber) pair function will be called only once,
	// so simply find first unused place in table
	while (profileResult->fileName)
	{
		profileResultIndex = (profileResultIndex + 1) % NB_TIMINGS;
		profileResult = &s_Profiler->tracks[_trackIdx].timings[profileResultIndex];

		// if we examined every entry in hash table that means hash table has less entries
		// than debug records we are putting in source code! Increase MAX_DEBUG_RECORD_COUNT!
		//Assert(profileResult != InitialprofileResult);
	}

	profileResult->fileName = _fileName;
	profileResult->lineNumber = _lineNumber;
	profileResult->blockName = _blockName;

	return profileResultIndex;
}

bool Profile::Profiler::AddTrack(const char* _name)
{
	if (trackCount < NB_TRACKS)
	{
		tracks[trackCount].name = _name;
		trackCount++;
		return true;
	}

	printf("Track named \"%s\" could not be added because the profiler has reached the maximum number of tracks (%u)\n", _name, NB_TRACKS);
	return false;
}

void Profile::Profiler::End() noexcept
{
	elapsed = Timer::GetCPUTimer() - start;
}

void Profile::Profiler::Initialize() noexcept
{
	start = Timer::GetCPUTimer();
}

void Profile::Profiler::Report() noexcept
{
	printf("\n---- Profiler: %s (%fms) ----\n", name, 1000 * (f64)elapsed / (f64)Timer::EstimateCPUFreq(100));
	for (ProfileTrack& track : tracks)
	{
		track.Report(elapsed);
	}
}