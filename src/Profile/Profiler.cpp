#include <cmath> //for std::sqrt
#include <cstring> //for strlen
#include "Profile/Profiler.hpp"

static Profile::Profiler* s_Profiler = nullptr;

void Profile::SetProfiler(Profiler* _profiler)
{
	s_Profiler = _profiler;
}

Profile::Profiler* Profile::GetProfiler()
{
	return s_Profiler;
}

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

void Profile::ProfileBlockResult::Reset() noexcept
{
	elapsed = 0;
	elapsedSec = 0.0;
	hitCount = 0;
	processedByteCount = 0;
	proportionInTrack = 0.f;
	proportionInTotal = 0.f;
	bandwidthInB = 0;
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

void Profile::ProfileTrackResult::Reset() noexcept
{
	for (IT_TIMINGS_TYPE i = 0; i < blockCount; ++i)
	{
		timings[i].Reset();
	}
	elapsed = 0;
	elapsedSec = 0.0;
	proportionInTotal = 0.0;
	blockCount = 0;
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
#if PROFILER_ENABLED
	printf("Estimated CPU Frequency: %llu\n", Timer::GetEstimatedCPUFreq());
	printf("\n---- Profiler: %s (%fms) ----\n", name, 1000 * (f64)elapsed / (f64)Timer::GetEstimatedCPUFreq());
	for (ProfileTrack& track : tracks)
	{
		if (track.hasBlock)
		{
			track.Report(elapsed);
		}
	}
#else
	printf("Profiler report was called but it is disabled. Report is therefore empty and will be skipped.\nThe profiler can be enabled by defining _PROFILER_ENABLED in the compiler options.\n");
#endif
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
	
	for (IT_TRACKS_TYPE i = 0; i < trackCount; ++i)
	{
		tracks[i].Report();
	}
}

void Profile::ProfilerResults::Reset() noexcept
{
	for (IT_TRACKS_TYPE i = 0; i < trackCount; ++i)
	{
		tracks[i].Reset();
	}
	elapsed = 0;
	trackCount = 0;
}

void Profile::RepetitionProfiler::ComputeAverageResults(u64 _repetitionCount) noexcept
{
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		averageResults.elapsed += ptr_repetitionResults[i].elapsed;
		averageResults.elapsedSec += ptr_repetitionResults[i].elapsedSec;
		if (averageResults.trackCount < ptr_repetitionResults[i].trackCount)
			averageResults.trackCount = ptr_repetitionResults[i].trackCount;

		for (IT_TRACKS_TYPE j = 0; j < ptr_repetitionResults[i].trackCount; ++j)
		{
			averageResults.tracks[j].name = ptr_repetitionResults[i].tracks[j].name;
			averageResults.tracks[j].elapsed += ptr_repetitionResults[i].tracks[j].elapsed;
			averageResults.tracks[j].elapsedSec += ptr_repetitionResults[i].tracks[j].elapsedSec;
			averageResults.tracks[j].proportionInTotal += ptr_repetitionResults[i].tracks[j].proportionInTotal;
			if (averageResults.tracks[j].blockCount < ptr_repetitionResults[i].tracks[j].blockCount)
				averageResults.tracks[j].blockCount = ptr_repetitionResults[i].tracks[j].blockCount;

			for (IT_TIMINGS_TYPE k = 0; k < ptr_repetitionResults[i].tracks[j].blockCount; ++k)
			{
				averageResults.tracks[j].timings[k].blockName = ptr_repetitionResults[i].tracks[j].timings[k].blockName;
				averageResults.tracks[j].timings[k].elapsed += ptr_repetitionResults[i].tracks[j].timings[k].elapsed;
				averageResults.tracks[j].timings[k].hitCount += ptr_repetitionResults[i].tracks[j].timings[k].hitCount;
				averageResults.tracks[j].timings[k].processedByteCount += ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount;
				averageResults.tracks[j].timings[k].proportionInTrack += ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack;
				averageResults.tracks[j].timings[k].proportionInTotal += ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal;
				averageResults.tracks[j].timings[k].bandwidthInB += ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB;
			}
		}
	}

	// finish the average calculation for the whole profiler
	averageResults.elapsed /= _repetitionCount;
	averageResults.elapsedSec /= _repetitionCount;
	for (IT_TRACKS_TYPE j = 0; j < averageResults.trackCount; ++j)
	{
		// finish the average calculation for the current track
		averageResults.tracks[j].elapsed /= _repetitionCount;
		averageResults.tracks[j].elapsedSec /= _repetitionCount;
		averageResults.tracks[j].proportionInTotal /= _repetitionCount;
		for (IT_TIMINGS_TYPE k = 0; k < averageResults.tracks[j].blockCount; ++k)
		{
			// finish the average calculation for the current block
			averageResults.tracks[j].timings[k].elapsed /= _repetitionCount;
			averageResults.tracks[j].timings[k].hitCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].processedByteCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTrack /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTotal /= _repetitionCount;
			averageResults.tracks[j].timings[k].bandwidthInB /= _repetitionCount;
		}
	}
}

void Profile::RepetitionProfiler::ComputeVarianceResults(u64 _repetitionCount) noexcept
{
	if (averageResults.name == nullptr)
	{
		ComputeAverageResults(_repetitionCount);
	}

	stdResults.name = averageResults.name;
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		varianceResults.elapsed += (ptr_repetitionResults[i].elapsed - averageResults.elapsed) * (ptr_repetitionResults[i].elapsed - averageResults.elapsed);
		varianceResults.elapsedSec += (ptr_repetitionResults[i].elapsedSec - averageResults.elapsedSec) * (ptr_repetitionResults[i].elapsedSec - averageResults.elapsedSec);
		if (varianceResults.trackCount < ptr_repetitionResults[i].trackCount)
			varianceResults.trackCount = ptr_repetitionResults[i].trackCount;
		for (IT_TRACKS_TYPE j = 0; j < ptr_repetitionResults[i].trackCount; ++j)
		{
			varianceResults.tracks[j].name = ptr_repetitionResults[i].tracks[j].name;
			varianceResults.tracks[j].elapsed += (ptr_repetitionResults[i].tracks[j].elapsed - averageResults.tracks[j].elapsed) * (ptr_repetitionResults[i].tracks[j].elapsed - averageResults.tracks[j].elapsed);
			varianceResults.tracks[j].elapsedSec += (ptr_repetitionResults[i].tracks[j].elapsedSec - averageResults.tracks[j].elapsedSec) * (ptr_repetitionResults[i].tracks[j].elapsedSec - averageResults.tracks[j].elapsedSec);
			varianceResults.tracks[j].proportionInTotal += (ptr_repetitionResults[i].tracks[j].proportionInTotal - averageResults.tracks[j].proportionInTotal) * (ptr_repetitionResults[i].tracks[j].proportionInTotal - averageResults.tracks[j].proportionInTotal);
			if (varianceResults.tracks[j].blockCount < ptr_repetitionResults[i].tracks[j].blockCount)
				varianceResults.tracks[j].blockCount = ptr_repetitionResults[i].tracks[j].blockCount;
			for (IT_TIMINGS_TYPE k = 0; k < ptr_repetitionResults[i].tracks[j].blockCount; ++k)
			{
				varianceResults.tracks[j].timings[k].blockName = ptr_repetitionResults[i].tracks[j].timings[k].blockName;
				varianceResults.tracks[j].timings[k].elapsed += (ptr_repetitionResults[i].tracks[j].timings[k].elapsed - averageResults.tracks[j].timings[k].elapsed) * (ptr_repetitionResults[i].tracks[j].timings[k].elapsed - averageResults.tracks[j].timings[k].elapsed);
				varianceResults.tracks[j].timings[k].hitCount += (ptr_repetitionResults[i].tracks[j].timings[k].hitCount - averageResults.tracks[j].timings[k].hitCount) * (ptr_repetitionResults[i].tracks[j].timings[k].hitCount - averageResults.tracks[j].timings[k].hitCount);
				varianceResults.tracks[j].timings[k].processedByteCount += (ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount - averageResults.tracks[j].timings[k].processedByteCount) * (ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount - averageResults.tracks[j].timings[k].processedByteCount);
				varianceResults.tracks[j].timings[k].proportionInTrack += (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack - averageResults.tracks[j].timings[k].proportionInTrack) * (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack - averageResults.tracks[j].timings[k].proportionInTrack);
				varianceResults.tracks[j].timings[k].proportionInTotal += (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal - averageResults.tracks[j].timings[k].proportionInTotal) * (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal - averageResults.tracks[j].timings[k].proportionInTotal);
				varianceResults.tracks[j].timings[k].bandwidthInB += (ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB - averageResults.tracks[j].timings[k].bandwidthInB) * (ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB - averageResults.tracks[j].timings[k].bandwidthInB);
			}
		}
	}
}

void Profile::RepetitionProfiler::FindMaxResults(u64 _repetitionCount) noexcept
{
	maxResults.name = averageResults.name;
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		MaxAssign(maxResults.elapsed, ptr_repetitionResults[i].elapsed);
		MaxAssign(maxResults.elapsedSec, ptr_repetitionResults[i].elapsedSec);
		if (maxResults.trackCount < ptr_repetitionResults[i].trackCount)
			maxResults.trackCount = ptr_repetitionResults[i].trackCount;
		for (IT_TRACKS_TYPE j = 0; j < ptr_repetitionResults[i].trackCount; ++j)
		{
			maxResults.tracks[j].name = ptr_repetitionResults[i].tracks[j].name;
			MaxAssign(maxResults.tracks[j].elapsed, ptr_repetitionResults[i].tracks[j].elapsed);
			MaxAssign(maxResults.tracks[j].elapsedSec, ptr_repetitionResults[i].tracks[j].elapsedSec);
			MaxAssign(maxResults.tracks[j].proportionInTotal, ptr_repetitionResults[i].tracks[j].proportionInTotal);
			if (maxResults.tracks[j].blockCount < ptr_repetitionResults[i].tracks[j].blockCount)
				maxResults.tracks[j].blockCount = ptr_repetitionResults[i].tracks[j].blockCount;
			for (IT_TIMINGS_TYPE k = 0; k < ptr_repetitionResults[i].tracks[j].blockCount; ++k)
			{
				maxResults.tracks[j].timings[k].blockName = ptr_repetitionResults[i].tracks[j].timings[k].blockName;
				MaxAssign(maxResults.tracks[j].timings[k].elapsed, ptr_repetitionResults[i].tracks[j].timings[k].elapsed);
				MaxAssign(maxResults.tracks[j].timings[k].hitCount, ptr_repetitionResults[i].tracks[j].timings[k].hitCount);
				MaxAssign(maxResults.tracks[j].timings[k].processedByteCount, ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount);
				MaxAssign(maxResults.tracks[j].timings[k].proportionInTrack, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack);
				MaxAssign(maxResults.tracks[j].timings[k].proportionInTotal, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal);
				MaxAssign(maxResults.tracks[j].timings[k].bandwidthInB, ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB);
			}
		}
	}
}

void Profile::RepetitionProfiler::FindMinResults(u64 _repetitionCount) noexcept
{
	minResults.name = averageResults.name;
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		MinAssign(minResults.elapsed, ptr_repetitionResults[i].elapsed);
		MinAssign(minResults.elapsedSec, ptr_repetitionResults[i].elapsedSec);
		if (minResults.trackCount < ptr_repetitionResults[i].trackCount)
			minResults.trackCount = ptr_repetitionResults[i].trackCount;
		for (IT_TRACKS_TYPE j = 0; j < ptr_repetitionResults[i].trackCount; ++j)
		{
			minResults.tracks[j].name = ptr_repetitionResults[i].tracks[j].name;
			MinAssign(minResults.tracks[j].elapsed, ptr_repetitionResults[i].tracks[j].elapsed);
			MinAssign(minResults.tracks[j].elapsedSec, ptr_repetitionResults[i].tracks[j].elapsedSec);
			MinAssign(minResults.tracks[j].proportionInTotal, ptr_repetitionResults[i].tracks[j].proportionInTotal);
			if (minResults.tracks[j].blockCount < ptr_repetitionResults[i].tracks[j].blockCount)
				minResults.tracks[j].blockCount = ptr_repetitionResults[i].tracks[j].blockCount;
			for (IT_TIMINGS_TYPE k = 0; k < ptr_repetitionResults[i].tracks[j].blockCount; ++k)
			{
				minResults.tracks[j].timings[k].blockName = ptr_repetitionResults[i].tracks[j].timings[k].blockName;
				MinAssign(minResults.tracks[j].timings[k].elapsed, ptr_repetitionResults[i].tracks[j].timings[k].elapsed);
				MinAssign(minResults.tracks[j].timings[k].hitCount, ptr_repetitionResults[i].tracks[j].timings[k].hitCount);
				MinAssign(minResults.tracks[j].timings[k].processedByteCount, ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount);
				MinAssign(minResults.tracks[j].timings[k].proportionInTrack, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack);
				MinAssign(minResults.tracks[j].timings[k].proportionInTotal, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal);
				MinAssign(minResults.tracks[j].timings[k].bandwidthInB, ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB);
			}
		}
	}
}

void Profile::RepetitionProfiler::FixedCountRepetitionTesting(u64 _repetitionCount, RepetitionTest& _repetitionTest)
{
	Profiler* ptr_profiler = GetProfiler();

	ptr_profiler->Reset();
	Reset(_repetitionCount);

	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		ptr_profiler->Initialize();
		_repetitionTest();
		ptr_profiler->End();
		ptr_repetitionResults[i].Capture(ptr_profiler);
		ptr_profiler->ResetExistingTracks();
	}
}

void Profile::RepetitionProfiler::Report(u64 _repetitionCount) noexcept
{
#if PROFILER_ENABLED

	ComputeVarianceResults(_repetitionCount);

	FindMaxResults(_repetitionCount);

	FindMinResults(_repetitionCount);

	//go through all blocks and all tracks and print the average results with the
	//standard deviation, the minimum and the maximum values
	printf("\n---- ProfilerResults: %s ({%f, %f(+/-)%f, %f}ms) ----\n",
		averageResults.name,
		1000 * minResults.elapsedSec, 1000 * averageResults.elapsedSec, 1000 * std::sqrt(varianceResults.elapsedSec), 1000 * maxResults.elapsedSec);
	for (IT_TRACKS_TYPE i = 0; i < averageResults.trackCount; ++i)
	{
		if (averageResults.tracks[i].name != nullptr)
		{
			printf("\n---- Profile Track Results: %s ({%f, %f(+/-)%f, %f}ms; {%.2f, %.2f(+/-)%.2f, %.2f}%% of total) ----\n",
				averageResults.tracks[i].name,
				1000 * minResults.tracks[i].elapsedSec, 1000 * averageResults.tracks[i].elapsedSec,	1000 * std::sqrt(varianceResults.tracks[i].elapsedSec), 1000 * maxResults.tracks[i].elapsedSec,
				minResults.tracks[i].proportionInTotal, averageResults.tracks[i].proportionInTotal,	std::sqrt(varianceResults.tracks[i].proportionInTotal), maxResults.tracks[i].proportionInTotal);

			for (IT_TIMINGS_TYPE j = 0; j < averageResults.tracks[i].blockCount; ++j)
			{
				if (averageResults.tracks[i].timings[j].blockName != nullptr)
				{
					printf("%s[{%llu, %llu(+/-)%f, %llu}]: {%llu, %llu(+/-)%f, %llu} ({%.2f, %.2f(+/-)%.2f, %.2f}%% of track; {%.2f, %.2f(+/-)%.2f, %.2f}%% of total",
						averageResults.tracks[i].timings[j].blockName,
						minResults.tracks[i].timings[j].hitCount, averageResults.tracks[i].timings[j].hitCount, std::sqrt(varianceResults.tracks[i].timings[j].hitCount), maxResults.tracks[i].timings[j].hitCount,
						minResults.tracks[i].timings[j].elapsed, averageResults.tracks[i].timings[j].elapsed, std::sqrt(varianceResults.tracks[i].timings[j].elapsed), maxResults.tracks[i].timings[j].elapsed,
						minResults.tracks[i].timings[j].proportionInTrack, averageResults.tracks[i].timings[j].proportionInTrack, std::sqrt(varianceResults.tracks[i].timings[j].proportionInTrack), maxResults.tracks[i].timings[j].proportionInTrack,
						minResults.tracks[i].timings[j].proportionInTotal, averageResults.tracks[i].timings[j].proportionInTotal, std::sqrt(varianceResults.tracks[i].timings[j].proportionInTotal), maxResults.tracks[i].timings[j].proportionInTotal);
					if (averageResults.tracks[i].timings[j].processedByteCount > 0)
					{
						printf("; {%.3f, %.3f(+/-)%.3f, %.3f}MB at {%.3f, %.3f(+/-)%.3f, %.3f}MB/s | {%.3f, %.3f(+/-)%.3f, %.3f}GB/s",
							(f32)minResults.tracks[i].timings[j].processedByteCount / (1 << 20), (f32)averageResults.tracks[i].timings[j].processedByteCount / (1 << 20), std::sqrt(varianceResults.tracks[i].timings[j].processedByteCount) / (1 << 20), (f32)maxResults.tracks[i].timings[j].processedByteCount / (1 << 20),
							minResults.tracks[i].timings[j].bandwidthInB / (1 << 20), averageResults.tracks[i].timings[j].bandwidthInB / (1 << 20),	std::sqrt(varianceResults.tracks[i].timings[j].bandwidthInB) / (1 << 20), maxResults.tracks[i].timings[j].bandwidthInB / (1 << 20),
							minResults.tracks[i].timings[j].bandwidthInB / (1 << 30), averageResults.tracks[i].timings[j].bandwidthInB / (1 << 30),	std::sqrt(varianceResults.tracks[i].timings[j].bandwidthInB) / (1 << 30), maxResults.tracks[i].timings[j].bandwidthInB / (1 << 30));
					}
					printf(")\n");
				}
			}
		}
	}
#else
	printf("RepetitionProfiler report was called but profiling is disabled. Report is therefore empty and will be skipped.\nThe profiler can be enabled by defining _PROFILER_ENABLED in the compiler options.\n");
#endif
}

void Profile::RepetitionProfiler::Reset(u64 _repetitionCount) noexcept
{
	averageResults.Reset();
	varianceResults.Reset();
	maxResults.Reset();
	minResults.Reset();
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		ptr_repetitionResults[i].Reset();
	}
}