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

void Profile::ProfileBlockResult::Capture(ProfileBlockRecorder& _record, NB_TRACKS_TYPE _trackIdx,
	NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _trackElapsedReference, u64 _totalElapsedReference) noexcept
{
	lineNumber = _record.lineNumber;
	trackIdx = _trackIdx;
	profileBlockRecorderIdx = _profileBlockRecorderIdx;
	fileName = _record.fileName;
	blockName = _record.blockName;
	elapsed = _record.elapsed;
	elapsedSec = (f64)_record.elapsed / (f64)Timer::GetEstimatedCPUFreq();
	hitCount = _record.hitCount;
	processedByteCount = _record.processedByteCount;
	proportionInTrack = _trackElapsedReference == 0 ? 0 : 100.0f * (f64)_record.elapsed / (f64)_trackElapsedReference;
	proportionInTotal = _totalElapsedReference == 0 ? 0 : 100.0f * (f64)_record.elapsed / (f64)_totalElapsedReference;
	bandwidthInB = _trackElapsedReference == 0 ? 0 : _record.processedByteCount / (((f64)_record.elapsed / (f64)_trackElapsedReference) * elapsedSec);
}

void Profile::ProfileBlockResult::Report() noexcept
{
	printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
		blockName, hitCount, elapsed, proportionInTrack,proportionInTotal);
	if (processedByteCount > 0)
	{
		printf("; %lluMB at %.2fMB/s | %.2fGB/s", processedByteCount / (1<<20), bandwidthInB / (1<<20), bandwidthInB / (1<<30));
	}
	printf(")\n");
}

void Profile::ProfileTrackResult::Capture(ProfileTrack& _track, u64 _trackIdx, u64 _totalElapsedReference) noexcept
{
	name = _track.name;
	elapsed = _track.elapsed;
	elapsedSec = (f64)_track.elapsed / (f64)Timer::GetEstimatedCPUFreq();
	proportionInTotal = _totalElapsedReference == 0 ? 0 : 100.0f * (f64)_track.elapsed / (f64)_totalElapsedReference;
	NB_TIMINGS_TYPE _profileBlockRecorderIdx = 0;
	for (ProfileBlockRecorder record : _track.timings)
	{
		if (record.blockName != nullptr)
		{
			timings[blockCount].Capture(record, _trackIdx, _profileBlockRecorderIdx, _track.elapsed, _totalElapsedReference);
			blockCount++;
		}
		_profileBlockRecorderIdx++;
	}
}

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

void Profile::ProfileTrackResult::Report() noexcept
{
	printf("\n---- Profile Track Results: %s (%fms; %.2f%% of total) ----\n", name, 1000 * elapsedSec, proportionInTotal);
	for (ProfileBlockResult& record : timings)
	{
		if (record.blockName != nullptr)
		{
			record.Report();
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

void Profile::ProfilerResults::Capture(Profiler* _profiler) noexcept
{
	name = _profiler->name;
	elapsed = _profiler->elapsed;
	elapsedSec = (f64)_profiler->elapsed / (f64)Timer::GetEstimatedCPUFreq();
	NB_TRACKS_TYPE trackIdx = 0;
	for (ProfileTrack& track : _profiler->tracks)
	{
		if (track.hasBlock)
		{
			tracks[trackCount].Capture(track, trackIdx, _profiler->elapsed);
			trackCount++;
		}
		trackIdx++;
	}
}

void Profile::ProfilerResults::Report() noexcept
{
	printf("\n---- ProfilerResults: %s (%fms) ----\n", name, 1000 * elapsedSec);
	
	for (NB_TRACKS_TYPE i = 0; i < trackCount; ++i)
	{
		tracks[i].Report();
	}
}

void Profile::RepetitionProfiler::Report(ProfilerResults* _results, u64 _repetitionCount) noexcept
{
	char* name = (char*)malloc(strlen(_results->name) + 100); //100 is enough for the string below (average of profiler "%s" over %llu
	sprintf(name, "Reporting average of profiler \"%s\" over %llu repetitions", _results->name, _repetitionCount);
	averageResults.name = name;
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		averageResults.elapsed += _results[i].elapsed;
		averageResults.elapsedSec += _results[i].elapsedSec;
		if (averageResults.trackCount < _results[i].trackCount)
		{
			averageResults.trackCount = _results[i].trackCount;
		}
		for (NB_TRACKS_TYPE j = 0; j < _results[i].trackCount; ++j)
		{
			averageResults.tracks[j].name = _results[i].tracks[j].name;
			averageResults.tracks[j].elapsed += _results[i].tracks[j].elapsed;
			averageResults.tracks[j].elapsedSec += _results[i].tracks[j].elapsedSec;
			if (averageResults.tracks[j].blockCount < _results[i].tracks[j].blockCount)
			{
				averageResults.tracks[j].blockCount = _results[i].tracks[j].blockCount;
			}
			for (NB_TIMINGS_TYPE k = 0; k < _results[i].tracks[j].blockCount; ++k)
			{
				averageResults.tracks[j].timings[k].blockName = _results[i].tracks[j].timings[k].blockName;
				averageResults.tracks[j].timings[k].elapsed += _results[i].tracks[j].timings[k].elapsed;
				averageResults.tracks[j].timings[k].hitCount += _results[i].tracks[j].timings[k].hitCount;
				averageResults.tracks[j].timings[k].processedByteCount += _results[i].tracks[j].timings[k].processedByteCount;
				averageResults.tracks[j].timings[k].proportionInTrack += _results[i].tracks[j].timings[k].proportionInTrack;
				averageResults.tracks[j].timings[k].proportionInTotal += _results[i].tracks[j].timings[k].proportionInTotal;
				averageResults.tracks[j].timings[k].bandwidthInB += _results[i].tracks[j].timings[k].bandwidthInB;
			}

		}
	}

	averageResults.elapsed /= _repetitionCount;
	averageResults.elapsedSec /= _repetitionCount;
	for (NB_TRACKS_TYPE j = 0; j < averageResults.trackCount; ++j)
	{
		averageResults.tracks[j].elapsed /= _repetitionCount;
		averageResults.tracks[j].elapsedSec /= _repetitionCount;
		for (NB_TIMINGS_TYPE k = 0; k < averageResults.tracks[j].blockCount; ++k)
		{
			averageResults.tracks[j].timings[k].elapsed /= _repetitionCount;
			averageResults.tracks[j].timings[k].hitCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].processedByteCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTrack /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTotal /= _repetitionCount;
			averageResults.tracks[j].timings[k].bandwidthInB /= _repetitionCount;
		}
	}

	//Report the average results
	averageResults.Report();
}