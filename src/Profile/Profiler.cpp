#include <cmath> //for std::sqrt
#include <cstring> //for strlen
#include <stdio.h> //for FILE
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

void Profile::ProfileBlockRecorder::Clear() noexcept
{
	start = 0;
	elapsed = 0;
	hitCount = 0;
	pageFaultCountStart = 0;
	pageFaultCountTotal = 0;
	processedByteCount = 0;
}

void Profile::ProfileBlockRecorder::Reset() noexcept
{
	start = 0;
	elapsed = 0;
	hitCount = 0;
	pageFaultCountStart = 0;
	pageFaultCountTotal = 0;
	processedByteCount = 0;
}

void Profile::ProfileBlockResult::Capture(ProfileBlockRecorder& _record, NB_TRACKS_TYPE _trackIdx,
	NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _trackElapsedReference, u64 _totalElapsedReference) noexcept
{
	trackIdx = _trackIdx;
	profileBlockRecorderIdx = _profileBlockRecorderIdx;
	blockName = _record.blockName;
	elapsed = _record.elapsed;
	elapsedSec = (f64)_record.elapsed / (f64)Timer::GetEstimatedCPUFreq();
	hitCount = _record.hitCount;
	pageFaultCountTotal = _record.pageFaultCountTotal;
	processedByteCount = _record.processedByteCount;
	proportionInTrack = _trackElapsedReference == 0 ? 0 : 100.0f * (f64)_record.elapsed / (f64)_trackElapsedReference;
	proportionInTotal = _totalElapsedReference == 0 ? 0 : 100.0f * (f64)_record.elapsed / (f64)_totalElapsedReference;
	bandwidthInB = _trackElapsedReference == 0 ? 0 : _record.processedByteCount / (((f64)_record.elapsed / (f64)_trackElapsedReference) * elapsedSec);
}

void Profile::ProfileBlockResult::Clear() noexcept
{
	trackIdx = 0;
	profileBlockRecorderIdx = 0;
	elapsed = 0;
	elapsedSec = 0.0;
	hitCount = 0;
	pageFaultCountTotal = 0;
	processedByteCount = 0;
	proportionInTrack = 0.f;
	proportionInTotal = 0.f;
	bandwidthInB = 0;
}

void Profile::ProfileBlockResult::Report() noexcept
{
	printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
		blockName, hitCount, elapsed, proportionInTrack,proportionInTotal);
	if (processedByteCount > 0)
	{
		printf("; %lluMB at %.2fMB/s | %.2fGB/s", processedByteCount / (1<<20), bandwidthInB / (1<<20), bandwidthInB / (1<<30));
	}

	if(pageFaultCountTotal > 0)
	{
		printf("; %llu PF (%0.4fKB/fault); Page size is %llu bytes", pageFaultCountTotal, (f64)processedByteCount / ((f64)pageFaultCountTotal * 1024.0), Surveyor::GetOSPageSize());
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
	blockCount = 0;
	NB_TIMINGS_TYPE _profileBlockRecorderIdx = 0;
	for (ProfileBlockRecorder record : _track.timings)
	{
		if (record.hitCount)
		{
			timings[blockCount].Capture(record, _trackIdx, _profileBlockRecorderIdx, _track.elapsed, _totalElapsedReference);
			blockCount++;
		}
		_profileBlockRecorderIdx++;
	}
}

void Profile::ProfileTrackResult::Clear() noexcept
{
	name = nullptr;
	elapsed = 0;
	elapsedSec = 0.0;
	proportionInTotal = 0.0;
	for (IT_TIMINGS_TYPE i = 0; i < blockCount; ++i)
	{
		timings[i].Clear();
	}
	blockCount = 0;
}

void Profile::ProfileTrack::Report(u64 _totalElapsedReference) noexcept
{
	f64 elapsedSec = (f64)elapsed / (f64)Timer::GetEstimatedCPUFreq();
	printf("---- Profile Track: %s (%fms; %.2f%% of total) ----\n", name, 1000 * elapsedSec,
		_totalElapsedReference == 0 ? 0 : 100.0f * (f64)elapsed / (f64)_totalElapsedReference);
	
	static f64 megaByte = 1<<20;
	static f64 gigaByte = 1<<30;

	for (ProfileBlockRecorder& record : timings)
	{
		if (record.hitCount)
		{
			printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
				record.blockName, record.hitCount, record.elapsed, elapsed == 0 ? 0 : 100.0f * (f64)record.elapsed / (f64)elapsed,
				_totalElapsedReference == 0 ? 0 : 100.0f * (f64)record.elapsed / (f64)_totalElapsedReference);
			if (record.processedByteCount > 0)
			{
				f64 bandwidth = (f64)record.processedByteCount / (((f64)record.elapsed / (f64)elapsed) * elapsedSec);
				printf("; %.2fMB at %.2fMB/s | %.2fGB/s", (f64)record.processedByteCount / megaByte, bandwidth / megaByte, bandwidth / gigaByte);
			}

			if(record.pageFaultCountTotal > 0)
			{
				printf("; %llu PF (%0.4fKB/fault); Page size is %llu bytes", record.pageFaultCountTotal, (f64)record.processedByteCount / ((f64)record.pageFaultCountTotal * 1024.0), Surveyor::GetOSPageSize());
			}

			printf(")\n");
		}
	}
}

void Profile::ProfileTrack::Clear() noexcept
{
	strcpy(name, "\0");
	elapsed = 0;
	ClearTimings();
}

void Profile::ProfileTrack::ClearTimings() noexcept
{
	hasBlock = false;
	for (ProfileBlockRecorder& record : timings)
	{
		if (record.blockName != nullptr)
		{
			record.Clear();
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
		if (record.hitCount)
		{
			record.Reset();
		}
	}
}

void Profile::ProfileTrackResult::Report() noexcept
{
	printf("---- Profile Track Results: %s (%fms; %.2f%% of total) ----\n", name, 1000 * elapsedSec, proportionInTotal);
	for (ProfileBlockResult& record : timings)
	{
		if (record.hitCount)
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

	while (profileBlockRecorder->hitCount)
	{
		profileBlockRecorderIndex = (profileBlockRecorderIndex + 1) % NB_TIMINGS;
		profileBlockRecorder = &s_Profiler->tracks[_trackIdx].timings[profileBlockRecorderIndex];

		// if we examined every entry in hash table that means hash table has less entries
		// than debug records we are putting in source code! Increase MAX_DEBUG_RECORD_COUNT!
		//Assert(profileBlockRecorder != InitialprofileBlockRecorder);
	}
	profileBlockRecorder->blockName = _blockName;

	return profileBlockRecorderIndex;
}

void Profile::Profiler::SetProfilerNameFmt(const char* _fmt, ...)
{
	//for security, check if the _fmt and the arguments is not bigger than the track name
	std::va_list args;
	va_start(args, _fmt);
	int length = std::vsnprintf(nullptr, 0, _fmt, args);
	va_end(args);

	if (length > sizeof(name))
	{
		printf("Warning: Tried to a name for the profiler that is too long for the buffer. The name will be truncated.\n");
	}

	if (length > 0)
	{
		va_start(args, _fmt);
		std::vsnprintf(name, sizeof(name), _fmt, args);
		va_end(args);
	}
}

void Profile::Profiler::SetTrackNameFmt(NB_TRACKS_TYPE _trackIdx, const char* _fmt, ...)
{
	if (_trackIdx < NB_TRACKS)
	{
		//for security, check if the _fmt and the arguments is not bigger than the track name
		std::va_list args;
		va_start(args, _fmt);
		int length = std::vsnprintf(nullptr, 0, _fmt, args);
		va_end(args);

		if (length > sizeof(tracks[_trackIdx].name))
		{
			printf("Warning: Tried to set a track name that is too long for the buffer. Track index: %d. The name will be truncated.\n", _trackIdx);
		}

		if (length > 0)
		{
			va_start(args, _fmt);
			std::vsnprintf(tracks[_trackIdx].name, sizeof(tracks[_trackIdx].name), _fmt, args);
			va_end(args);
		}
	}
}

void Profile::Profiler::Clear() noexcept
{
	strcpy(name, "\0");
	start = 0;
	elapsed = 0;
	ClearTracks();
}

void Profile::Profiler::ClearTracks() noexcept
{
	for (ProfileTrack& track : tracks)
	{
		if (track.hasBlock)
		{
			track.Clear();
		}
	}
}

void Profile::Profiler::ExportToCSV(const char* _fileName) noexcept
{
#if PROFILER_ENABLED
	FILE* file = fopen(_fileName, "w");
	if (file)
	{
		printf("Exporting profiler results to %s\n", _fileName);
		fprintf(file, "Estimated CPU Frequency,Profiler Name,Total Elapsed,Total Time in Seconds\n");
		fprintf(file, "%llu,%s,%llu,%f\n",
		Timer::GetEstimatedCPUFreq(), //Estimated CPU Frequency
		name, //Profiler Name
		elapsed, //Total Elapsed
		(f64)elapsed / (f64)Timer::GetEstimatedCPUFreq() //Total Time in Seconds
		);
		fprintf(file, "Track Name,Track Elapsed,Track Elapsed in Secconds,Track Proportion in Total,Block Name,Block Hit Count,Block Elapsed,Block Elapsed in Seconds,Block Proportion in Track,Block Proportion in Total,Block Associated Page Faults Count,Block Processed Byte Count,Block Bandwidth In Bytes\n");
		for (ProfileTrack& track : tracks)
		{
			if (track.hasBlock)
			{
				for (ProfileBlockRecorder& record : track.timings)
				{
					if (record.hitCount)
					{
						fprintf(file, "%s,%llu,%f,%f,%s,%llu,%llu,%f,%f,%f,%llu,%llu,%f\n",
							track.name, //Track Name
							track.elapsed, //Track Elapsed
							(f64)track.elapsed / (f64)Timer::GetEstimatedCPUFreq(), //Track Elapsed in Seconds
							100.0f * (f64)track.elapsed / (f64)elapsed, //Track Proportion in Total
							record.blockName, //Block Name
							record.hitCount, //Block Hit Count
							record.elapsed, //Block Elapsed
							(f64)record.elapsed / (f64)Timer::GetEstimatedCPUFreq(), //Block Elapsed in Seconds
							100.0 * (f64)record.elapsed / (f64)track.elapsed, //Block Proportion in Track
							100.0 * (f64)record.elapsed / (f64)elapsed, //Block Proportion in Total
							record.pageFaultCountTotal, //Block Associated Page Faults Count
							record.processedByteCount, //Block Processed Byte Count
							record.processedByteCount / (((f64)record.elapsed / (f64)track.elapsed) * (f64)track.elapsed / (f64)Timer::GetEstimatedCPUFreq()) //Block Bandwidth In Bytes
							);
					}
				}
			}
		}
		fclose(file);
	}
	else
	{
		printf("Error: Could not open file %s for writing.\n", _fileName);
	}
#else
	printf("Profiler export as CSV was called but it is disabled. The profiler is therefore empty and the export will be skipped.\nThe profiler can be enabled by defining _PROFILER_ENABLED in the compiler options.\n");
#endif
}

void Profile::Profiler::End() noexcept
{
	elapsed = Timer::GetCPUTimer() - start;
}

void Profile::Profiler::Initialize() noexcept
{
	Surveyor::InitializeOSMetrics();
	start = Timer::GetCPUTimer();
}

void Profile::Profiler::Report() noexcept
{
#if PROFILER_ENABLED
	printf("\n---- Estimated CPU Frequency: %llu ----\n", Timer::GetEstimatedCPUFreq());
	printf("---- Profiler Report: %s (%fms) ----\n", name, 1000 * (f64)elapsed / (f64)Timer::GetEstimatedCPUFreq());

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
	ResetTracks();
}

void Profile::Profiler::ResetTracks() noexcept
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
	trackCount = 0;
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

void Profile::ProfilerResults::Clear() noexcept
{
	name = nullptr;
	elapsed = 0;
	elapsedSec = 0.0;
	for (IT_TRACKS_TYPE i = 0; i < trackCount; ++i)
	{
		tracks[i].Clear();
	}
	trackCount = 0;
}

void Profile::ProfilerResults::ExportToCSV(const char* _path) noexcept
{
#if PROFILER_ENABLED
    FILE* file = fopen(_path, "w");
    if (file)
    {
        printf("Exporting profiler results to %s\n", _path);
        fprintf(file, "Estimated CPU Frequency,Profiler Name,Total Elapsed,Total Time in Seconds\n");
        fprintf(file, "%llu,%s,%llu,%f\n",
		Timer::GetEstimatedCPUFreq(), //Estimated CPU Frequency
		name, //Profiler Name
		elapsed, //Total Elapsed
		elapsedSec //Total Time in Seconds
		);
        fprintf(file, "Track Name,Track Elapsed,Track Elapsed in Seconds,Track Proportion in Total,Block Name,Block Hit Count,Block Elapsed,Block Elapsed in Seconds,Block Proportion in Track,Block Proportion in Total,Block Associated Page Faults Count,Block Processed Byte Count,Block Bandwidth In Bytes\n");
        for (IT_TRACKS_TYPE i = 0; i < trackCount; ++i)
        {
            for (IT_TIMINGS_TYPE j = 0; j < tracks[i].blockCount; ++j)
            {
                fprintf(file, "%s,%llu,%f,%f,%s,%llu,%llu,%f,%f,%f,%llu,%llu,%f\n",
					tracks[i].name, //Track Name
					tracks[i].elapsed, //Track Elapsed
					tracks[i].elapsedSec, //Track Elapsed in Seconds
					100.0 * (f64)tracks[i].elapsed / (f64)elapsed, //Track Proportion in Total
					tracks[i].timings[j].blockName, //Block Name
					tracks[i].timings[j].hitCount, //Block Hit Count
					tracks[i].timings[j].elapsed, //Block Elapsed
					tracks[i].timings[j].elapsedSec, //Block Elapsed in Seconds
					100.0 * (f64)tracks[i].timings[j].elapsed / (f64)tracks[i].elapsed, //Block Proportion in Track
					100.0 * (f64)tracks[i].timings[j].elapsed / (f64)elapsed, //Block Proportion in Total
					tracks[i].timings[j].pageFaultCountTotal, //Block Associated Page Faults Count
					tracks[i].timings[j].processedByteCount, //Block Processed Byte Count
					(f64)tracks[i].timings[j].processedByteCount / (((f64)tracks[i].timings[j].elapsed / (f64)tracks[i].elapsed) * ((f64)tracks[i].elapsed / (f64)Timer::GetEstimatedCPUFreq())) //Block Bandwidth In Bytes
					);
            }
        }
        fclose(file);
    }
	else
	{
		printf("Error: Could not open file %s for writing.\n", _path);
	}
#else
	printf("Profiler export as CSV was called but the profiler is disabled. The profiler results are therefore empty and the export will be skipped.\nThe profiler can be enabled by defining _PROFILER_ENABLED in the compiler options.\n");
#endif
}

void Profile::ProfilerResults::Report() noexcept
{
	printf("---- ProfilerResults: %s (%fms) ----\n", name, 1000 * elapsedSec);
	
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

void Profile::RepetitionProfiler::Clear(u64 _repetitionCount) noexcept
{
	averageResults.Clear();
	maxResults.Clear();
	minResults.Clear();
	varianceResults.Clear();
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		ptr_repetitionResults[i].Clear();
	}
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
				averageResults.tracks[j].timings[k].elapsedSec += ptr_repetitionResults[i].tracks[j].timings[k].elapsedSec;
				averageResults.tracks[j].timings[k].hitCount += ptr_repetitionResults[i].tracks[j].timings[k].hitCount;
				averageResults.tracks[j].timings[k].pageFaultCountTotal += ptr_repetitionResults[i].tracks[j].timings[k].pageFaultCountTotal;
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
			averageResults.tracks[j].timings[k].elapsedSec /= _repetitionCount;
			averageResults.tracks[j].timings[k].hitCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].pageFaultCountTotal /= _repetitionCount;
			averageResults.tracks[j].timings[k].processedByteCount /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTrack /= _repetitionCount;
			averageResults.tracks[j].timings[k].proportionInTotal /= _repetitionCount;
			averageResults.tracks[j].timings[k].bandwidthInB /= _repetitionCount;
		}
	}
}

void Profile::RepetitionProfiler::ComputeVarianceResults(u64 _repetitionCount) noexcept
{
	ComputeAverageResults(_repetitionCount);

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
				varianceResults.tracks[j].timings[k].elapsedSec += (ptr_repetitionResults[i].tracks[j].timings[k].elapsedSec - averageResults.tracks[j].timings[k].elapsedSec) * (ptr_repetitionResults[i].tracks[j].timings[k].elapsedSec - averageResults.tracks[j].timings[k].elapsedSec);
				varianceResults.tracks[j].timings[k].hitCount += (ptr_repetitionResults[i].tracks[j].timings[k].hitCount - averageResults.tracks[j].timings[k].hitCount) * (ptr_repetitionResults[i].tracks[j].timings[k].hitCount - averageResults.tracks[j].timings[k].hitCount);
				varianceResults.tracks[j].timings[k].pageFaultCountTotal += (ptr_repetitionResults[i].tracks[j].timings[k].pageFaultCountTotal - averageResults.tracks[j].timings[k].pageFaultCountTotal) * (ptr_repetitionResults[i].tracks[j].timings[k].pageFaultCountTotal - averageResults.tracks[j].timings[k].pageFaultCountTotal);
				varianceResults.tracks[j].timings[k].processedByteCount += (ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount - averageResults.tracks[j].timings[k].processedByteCount) * (ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount - averageResults.tracks[j].timings[k].processedByteCount);
				varianceResults.tracks[j].timings[k].proportionInTrack += (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack - averageResults.tracks[j].timings[k].proportionInTrack) * (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack - averageResults.tracks[j].timings[k].proportionInTrack);
				varianceResults.tracks[j].timings[k].proportionInTotal += (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal - averageResults.tracks[j].timings[k].proportionInTotal) * (ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal - averageResults.tracks[j].timings[k].proportionInTotal);
				varianceResults.tracks[j].timings[k].bandwidthInB += (ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB - averageResults.tracks[j].timings[k].bandwidthInB) * (ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB - averageResults.tracks[j].timings[k].bandwidthInB);
			}
		}
	}

	// finish the standard deviation calculation for the whole profiler
	varianceResults.elapsed /= _repetitionCount;
	varianceResults.elapsedSec /= _repetitionCount;
	for (IT_TRACKS_TYPE j = 0; j < varianceResults.trackCount; ++j)
	{
		// finish the standard deviation calculation for the current track
		varianceResults.tracks[j].elapsed /= _repetitionCount;
		varianceResults.tracks[j].elapsedSec /= _repetitionCount;
		varianceResults.tracks[j].proportionInTotal /= _repetitionCount;
		for (IT_TIMINGS_TYPE k = 0; k < varianceResults.tracks[j].blockCount; ++k)
		{
			// finish the standard deviation calculation for the current block
			varianceResults.tracks[j].timings[k].elapsed /= _repetitionCount;
			varianceResults.tracks[j].timings[k].elapsedSec /= _repetitionCount;
			varianceResults.tracks[j].timings[k].hitCount /= _repetitionCount;
			varianceResults.tracks[j].timings[k].pageFaultCountTotal /= _repetitionCount;
			varianceResults.tracks[j].timings[k].processedByteCount /= _repetitionCount;
			varianceResults.tracks[j].timings[k].proportionInTrack /= _repetitionCount;
			varianceResults.tracks[j].timings[k].proportionInTotal /= _repetitionCount;
			varianceResults.tracks[j].timings[k].bandwidthInB /= _repetitionCount;
		}
	}
}

void Profile::RepetitionProfiler::FindMaxResults(u64 _repetitionCount) noexcept
{
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
				MaxAssign(maxResults.tracks[j].timings[k].elapsedSec, ptr_repetitionResults[i].tracks[j].timings[k].elapsedSec);
				MaxAssign(maxResults.tracks[j].timings[k].hitCount, ptr_repetitionResults[i].tracks[j].timings[k].hitCount);
				MaxAssign(maxResults.tracks[j].timings[k].pageFaultCountTotal, ptr_repetitionResults[i].tracks[j].timings[k].pageFaultCountTotal);
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
				MinAssign(minResults.tracks[j].timings[k].elapsedSec, ptr_repetitionResults[i].tracks[j].timings[k].elapsedSec);
				MinAssign(minResults.tracks[j].timings[k].hitCount, ptr_repetitionResults[i].tracks[j].timings[k].hitCount);
				MinAssign(minResults.tracks[j].timings[k].pageFaultCountTotal, ptr_repetitionResults[i].tracks[j].timings[k].pageFaultCountTotal);
				MinAssign(minResults.tracks[j].timings[k].processedByteCount, ptr_repetitionResults[i].tracks[j].timings[k].processedByteCount);
				MinAssign(minResults.tracks[j].timings[k].proportionInTrack, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTrack);
				MinAssign(minResults.tracks[j].timings[k].proportionInTotal, ptr_repetitionResults[i].tracks[j].timings[k].proportionInTotal);
				MinAssign(minResults.tracks[j].timings[k].bandwidthInB, ptr_repetitionResults[i].tracks[j].timings[k].bandwidthInB);
			}
		}
	}
}

void Profile::RepetitionProfiler::BestPerfSearchRepetitionTesting(u16 _repetitionTestTimeOut, bool _reset, bool _clear, u16 _globalTimeOut)
{
	Profiler* ptr_profiler = GetProfiler();

	u64 nextTestTimeOut = 0;

	//The inline comparison is equivalent to a max function.
	//It's the only place where I need a max function so I didn't bother to implement one (and even less to include <algorithm> for it).
	u64 testGlobalTimeOut = Timer::GetCPUTimer() + (_repetitionTestTimeOut >= _globalTimeOut ? _repetitionTestTimeOut : _globalTimeOut) * Timer::GetEstimatedCPUFreq();

	u16 repetitionTestsCount = (u16)repetitionTests.size();
	u64* bestPerfs = (u64*)malloc(repetitionTestsCount * sizeof(u64));

	if (bestPerfs)
	{
		for (int i = 0; i < repetitionTestsCount; i++)
		{
			bestPerfs[i] = 0xffffffffffffffffull;
		}

		while (Timer::GetCPUTimer() < testGlobalTimeOut)
		{
			for (u32 i = 0; i < repetitionTestsCount; i++)
			{
				if (_reset && !_clear)
				{
					ptr_profiler->Reset();
					Reset(repetitionTestsCount);
				}
				else if (_clear)
				{
					ptr_profiler->Clear();
					Clear(repetitionTestsCount);

					if (repetitionTests[i]->name)
					{
						ptr_profiler->SetProfilerName(repetitionTests[i]->name);
					}
					else
					{
						ptr_profiler->SetProfilerNameFmt("Best Perf Search Repetition Test %d", i);
					}

					//Give default names to the tracks in the profiler
					for (NB_TRACKS_TYPE i = 0; i < NB_TRACKS; i++)
					{
						ptr_profiler->SetTrackNameFmt(i, "Track %d", i);
					}
				}

				printf("\n");
				nextTestTimeOut = Timer::GetCPUTimer() + _repetitionTestTimeOut * Timer::GetEstimatedCPUFreq();
				// Run the test as as long as we find a new profile that has a better performance
				// than the previous one.
				while (Timer::GetCPUTimer() < nextTestTimeOut)
				{
					ptr_profiler->ResetTracks();
					ptr_profiler->Initialize();
					(*repetitionTests[i])();
					ptr_profiler->End();

					// If we find a better performance
					if (ptr_profiler->elapsed < bestPerfs[i])
					{
						// Reset the test time out
						printf("New best performance found for test %s: %llu", ptr_profiler->name, ptr_profiler->elapsed);
						nextTestTimeOut = Timer::GetCPUTimer() + _repetitionTestTimeOut * Timer::GetEstimatedCPUFreq();
						bestPerfs[i] = ptr_profiler->elapsed;

						// bring the cursor back to the beginning of the console line
						printf("\r");
					}
				}
				ptr_profiler->Report();
			}
		}
		std::printf("\nExited BestPerfSearchRepetitionTesting due to global time out.\n");
	}
	else
	{
		std::printf("ERROR: Insufficiant memory to run BestPerfSearchRepetitionTesting.");
	}

	free(bestPerfs);

}

void Profile::RepetitionProfiler::ExportToCSV(const char* _path, u64 _repetitionCount) noexcept
{
	// Export the average results to a CSV file
	// Make the file name
	char path[256];
	sprintf(path, "%s/Summary/Average.csv", _path);
	averageResults.ExportToCSV(path);

	// Export the max results to a CSV file
	sprintf(path, "%s/Summary/Max.csv", _path);
	maxResults.ExportToCSV(path);

	// Export the min results to a CSV file
	sprintf(path, "%s/Summary/Min.csv", _path);
	minResults.ExportToCSV(path);

	// Export the variance results to a CSV file
	sprintf(path, "%s/Summary/Variance.csv", _path);
	varianceResults.ExportToCSV(path);

	// Export the repetition results to a CSV file
	for (u64 i = 0; i < _repetitionCount; ++i)
	{
		sprintf(path, "%s/Repetitions/%llu.csv", _path, i);
		ptr_repetitionResults[i].ExportToCSV(path);
	}
}

void Profile::RepetitionProfiler::FixedCountRepetitionTesting(u64 _repetitionCount, bool _reset, bool _clear)
{
	Profiler* ptr_profiler = GetProfiler();

	for (u32 i = 0; i < repetitionTests.size(); i++)
	{
		if (_reset && !_clear)
		{
			ptr_profiler->Reset();
			Reset(_repetitionCount);
		}
		else if (_clear)
		{
			ptr_profiler->Clear();
			Clear(_repetitionCount);

			if (repetitionTests[i]->name)
			{
				ptr_profiler->SetProfilerName(repetitionTests[i]->name);
			}
			else
			{
				ptr_profiler->SetProfilerNameFmt("Fixed Count Repetition Test %d", i);
			}

			averageResults.name = ptr_profiler->name;
			maxResults.name = ptr_profiler->name;
			minResults.name = ptr_profiler->name;
			varianceResults.name = ptr_profiler->name;

			//Give default names to the tracks in the profiler
			for (NB_TRACKS_TYPE i = 0; i < NB_TRACKS; i++)
			{
				ptr_profiler->SetTrackNameFmt(i, "Track %d", i);
			}
		}

		for (u64 j = 0; j < _repetitionCount; ++j)
		{
			ptr_profiler->Initialize();
			(*repetitionTests[i])();
			ptr_profiler->End();
			ptr_repetitionResults[j].Capture(ptr_profiler);
			ptr_profiler->ResetTracks();
		}
		
		Report(_repetitionCount);
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
	printf("\n---- Estimated CPU Frequency: %llu ----\n", Timer::GetEstimatedCPUFreq());
	printf("---- Repetition Profiler Report: %s ({%f, %f(+/-)%f, %f}ms) ----\n",
		averageResults.name,
		1000 * minResults.elapsedSec, 1000 * averageResults.elapsedSec, 1000 * std::sqrt(varianceResults.elapsedSec), 1000 * maxResults.elapsedSec);
	for (IT_TRACKS_TYPE i = 0; i < averageResults.trackCount; ++i)
	{
		if (averageResults.tracks[i].name != nullptr)
		{
			printf("---- Profile Track Results : % s({% f,% f(+/ -) % f,% f}ms; { % .2f, % .2f(+/ -) % .2f, % .2f }%% of total) ----\n",
				averageResults.tracks[i].name,
				1000 * minResults.tracks[i].elapsedSec, 1000 * averageResults.tracks[i].elapsedSec,	1000 * std::sqrt(varianceResults.tracks[i].elapsedSec), 1000 * maxResults.tracks[i].elapsedSec,
				minResults.tracks[i].proportionInTotal, averageResults.tracks[i].proportionInTotal,	std::sqrt(varianceResults.tracks[i].proportionInTotal), maxResults.tracks[i].proportionInTotal);

			for (IT_TIMINGS_TYPE j = 0; j < averageResults.tracks[i].blockCount; ++j)
			{
				if (averageResults.tracks[i].timings[j].hitCount > 0)
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
					if ((f64)averageResults.tracks[i].timings[j].pageFaultCountTotal > 0.0)
					{
						printf("; {%.llu, %.llu(+/-)%.f, %.llu}PF ({%0.4f, %0.4f(+/-)%0.4f, %0.4f}KB/fault); Page size is %llu bytes",
							minResults.tracks[i].timings[j].pageFaultCountTotal, averageResults.tracks[i].timings[j].pageFaultCountTotal, std::sqrt(varianceResults.tracks[i].timings[j].pageFaultCountTotal), maxResults.tracks[i].timings[j].pageFaultCountTotal,
							(f64)minResults.tracks[i].timings[j].processedByteCount / ((f64)minResults.tracks[i].timings[j].pageFaultCountTotal * 1024.0), (f64)averageResults.tracks[i].timings[j].processedByteCount / ((f64)averageResults.tracks[i].timings[j].pageFaultCountTotal * 1024.0), varianceResults.tracks[i].timings[j].pageFaultCountTotal == 0 ? 0.0 :std::sqrt((f64)varianceResults.tracks[i].timings[j].processedByteCount / ((f64)varianceResults.tracks[i].timings[j].pageFaultCountTotal * 1024.0)), (f64)maxResults.tracks[i].timings[j].processedByteCount / ((f64)maxResults.tracks[i].timings[j].pageFaultCountTotal * 1024.0),
							Surveyor::GetOSPageSize());
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