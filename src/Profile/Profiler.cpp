#include "Profile/Profiler.hpp"

static Profile::Profiler* s_Profiler = nullptr;

PROFILE_API void Profile::SetProfiler(Profiler* _profiler)
{
	s_Profiler = _profiler;
}

PROFILE_API Profile::Profiler* Profile::GetProfiler()
{
	return s_Profiler;
}

#if PROFILER_ENABLED

Profile::ProfileBlock::ProfileBlock(NB_TRACKS_TYPE _trackIdx, NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _byteCount) :
	trackIdx(_trackIdx), profileBlockRecorderIdx(_profileBlockRecorderIdx)
{
	s_Profiler->OpenBlock(trackIdx, profileBlockRecorderIdx, _byteCount);
}

Profile::ProfileBlock::~ProfileBlock()
{
	s_Profiler->CloseBlock(trackIdx, profileBlockRecorderIdx);
}

void Profile::ProfileBlockRecorder::Reset() noexcept
{
	start = 0;
	elapsed = 0;
	hitCount = 0;
	processedByteCount = 0;
}
#endif

void Profile::ProfileTrack::Report(u64 _totalElapsedReference) noexcept
{
	f64 elapsedSec = (f64)elapsed / (f64)Timer::GetEstimatedCPUFreq();
	printf("\n---- Profile Track: %s (%fms; %.2f%% of total) ----\n", name, 1000 * elapsedSec,
		_totalElapsedReference == 0 ? 0 : 100.0f * (f64)elapsed / (f64)_totalElapsedReference);
	
	static f64 megaByte = 1<<20;
	static f64 gigaByte = 1<<30;

	for (ProfileBlockRecorder& record : timings)
	{
		if (record.blockName != nullptr)
		{
			printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
				record.blockName, record.hitCount, record.elapsed, elapsed == 0 ? 0 : 100.0f * (f64)record.elapsed / (f64)elapsed,
				_totalElapsedReference == 0 ? 0 : 100.0f * (f64)record.elapsed / (f64)_totalElapsedReference);
			if (record.processedByteCount > 0)
			{
				f64 bandwidth = (f64)record.processedByteCount / (((f64)record.elapsed / (f64)elapsed) * elapsedSec);
				printf("; %.2fMB at %.2fMB/s | %.2fGB/s", (f64)record.processedByteCount / megaByte, bandwidth / megaByte, bandwidth / gigaByte);
			}
			printf(")\n");
		}
	}
}

void Profile::ProfileTrack::Reset() noexcept
{
	elapsed = 0;
	ResetTimings();
}

void Profile::ProfileTrack::ResetTimings() noexcept
{
	for (ProfileBlockRecorder& record : timings)
	{
		if (record.blockName != nullptr)
		{
			record.Reset();
		}
	}
}

NB_TIMINGS_TYPE Profile::Profiler::GetProfileBlockRecorderIndex(NB_TRACKS_TYPE _trackIdx,
	const char* _fileName, u32 _lineNumber, const char* _blockName)
{
	NB_TIMINGS_TYPE profileBlockRecorderIndex = Hash(_fileName, _lineNumber) % NB_TIMINGS;

	ProfileBlockRecorder* profileBlockRecorder = &s_Profiler->tracks[_trackIdx].timings[profileBlockRecorderIndex];
	ProfileBlockRecorder* InitialprofileBlockRecorder = profileBlockRecorder;

	// no need to compare _fileName and LinenNumber values, because for
	// each (_fileName, _lineNumber) pair function will be called only once,
	// so simply find first unused place in table
	while (profileBlockRecorder->fileName)
	{
		profileBlockRecorderIndex = (profileBlockRecorderIndex + 1) % NB_TIMINGS;
		profileBlockRecorder = &s_Profiler->tracks[_trackIdx].timings[profileBlockRecorderIndex];

		// if we examined every entry in hash table that means hash table has less entries
		// than debug records we are putting in source code! Increase MAX_DEBUG_RECORD_COUNT!
		//Assert(profileBlockRecorder != InitialprofileBlockRecorder);
	}

	s_Profiler->tracks[_trackIdx].hasBlock = true;
	profileBlockRecorder->fileName = _fileName;
	profileBlockRecorder->lineNumber = _lineNumber;
	profileBlockRecorder->blockName = _blockName;

	return profileBlockRecorderIndex;
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
	printf("Estimated CPU Frequency: %llu\n", Timer::GetEstimatedCPUFreq());
	printf("\n---- Profiler: %s (%fms) ----\n", name, 1000 * (f64)elapsed / (f64)Timer::GetEstimatedCPUFreq());
	for (ProfileTrack& track : tracks)
	{
		if (track.hasBlock)
		{
			track.Report(elapsed);
		}
	}
}

void Profile::Profiler::Reset() noexcept
{
	start = 0;
	elapsed = 0;
	ResetExistingTracks();
}

void Profile::Profiler::ResetExistingTracks() noexcept
{
	for (ProfileTrack& track : tracks)
	{
		if (track.hasBlock)
		{
			track.Reset();
		}
	}
};