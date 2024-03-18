#include "Profile/Profiler.hpp"

static Profile::Profiler* s_Profiler = nullptr;

PROFILE_API void Profile::SetProfiler(Profiler* _profiler)
{
	s_Profiler = _profiler;
}

#if PROFILER_ENABLED

Profile::ProfileBlock::ProfileBlock(const char* _name, u16 _trackIdx, u16 _selfIdx, u64 _byteCount) :
	name(_name), trackIdx(_trackIdx), selfIdx(_selfIdx)
{
	start = Timer::GetCPUTimer();
	s_Profiler->tracks[trackIdx].timings[selfIdx].processedByteCount += _byteCount;
	elapsedBuffer = s_Profiler->tracks[trackIdx].timings[selfIdx].elapsed;
}

Profile::ProfileBlock::~ProfileBlock()
{
	if (trackIdx < NB_TRACKS)
	{
		s_Profiler->tracks[trackIdx].AddResult(name, elapsedBuffer + Timer::GetCPUTimer() - start, selfIdx);
	}
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
		if (result.name != nullptr)
		{
			printf("%s[%llu]: %llu (%.2f%% of track; %.2f%% of total",
				result.name, result.hitCount, result.elapsed, 100.0f * (f64)result.elapsed / (f64)elapsed,
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

bool Profile::Profiler::AddTrack(const char* _name)
{
	for (u16 trackIdx = 0; trackIdx < NB_TRACKS; ++trackIdx)
	{
		if (tracks[trackIdx].name == nullptr)
		{
			tracks[trackIdx].name = _name;
			return true;
		}
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