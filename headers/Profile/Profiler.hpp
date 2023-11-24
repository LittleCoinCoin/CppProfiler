#pragma once

#include <array>
#include <vector>

#include "Export.hpp"
#include "Timing.hpp"
#include "Types.hpp"

#ifdef _PROFILER_ENABLED
#define PROFILER_ENABLED 1
#else
#define PROFILER_ENABLED 0
#endif // _PROFILER_ENABLED

namespace Profile
{
#define NB_TIMINGS 1024
#define NB_TRACKS 2

#if PROFILER_ENABLED

	struct PROFILE_API ProfileBlock
	{
		const char* name;
		u64 start;
		u64 elapsedBuffer;
		u16 trackIdx;
		u16 selfIdx;

		ProfileBlock(const char* _name, u16 _trackIdx, u16 _selfIdx, u64 _byteCount);

		~ProfileBlock();

	};

#define NAME_CONCAT2(a, b) a##b
#define NAME_CONCAT(a, b) NAME_CONCAT2(a, b)

#define PROFILE_BLOCK_TIME_BANDWIDTH(name, trackIdx, byteCount) Profile::ProfileBlock NAME_CONCAT(Block, __LINE__)(name, trackIdx, __COUNTER__ + 1, byteCount)
#define PROFILE_BLOCK_TIME(name, trackIdx) PROFILE_BLOCK_TIME_BANDWIDTH(name, trackIdx, 0)

#define PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, byteCount) PROFILE_BLOCK_TIME_BANDWIDTH(__FUNCTION__, trackIdx, byteCount)
#define PROFILE_FUNCTION_TIME(trackIdx) PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, 0)

#define PROFILER_END_CHECK static_assert(__COUNTER__ < NB_TIMINGS, "Number of profile blocks exceeds size of ProfileTrack::timings array")

#else // PROFILER_ENABLED

#define PROFILE_BLOCK_TIME_BANDWIDTH(...)
#define PROFILE_BLOCK_TIME(...)
#define PROFILE_FUNCTION_TIME_BANDWIDTH(...)
#define PROFILE_FUNCTION_TIME(...)
#define PROFILER_END_CHECK

#endif // PROFILER_ENABLED

	struct ProfileResult
	{
		const char* name = nullptr;
		u64 elapsed = 0;
		u64 hitCount = 0;
		u64 processedByteCount = 0;
	};

	struct ProfileTrack
	{
		const char* name = nullptr;
		u64 start = 0;
		u64 elapsed = 0;
		std::array<ProfileResult, NB_TIMINGS> timings;

		inline void AddResult(const char* _name, u64 _elapsed, u16 _timingIdx)
		{
			timings[_timingIdx].name = _name;
			timings[_timingIdx].elapsed = _elapsed;
			++timings[_timingIdx].hitCount;
		}

		PROFILE_API void Initialize() noexcept;

		PROFILE_API void End() noexcept;

		PROFILE_API void Report(u64 _totalElapsedReference) noexcept;
	};

	struct Profiler
	{
		const char* name = nullptr;
		u64 start = 0;
		u64 elapsed = 0;
		std::array<ProfileTrack, NB_TRACKS> tracks;

		Profiler(const char* _name) : name(_name)
		{

		}

		PROFILE_API bool AddTrack(const char* _name);

		PROFILE_API void End() noexcept;

		PROFILE_API void Initialize() noexcept;

		PROFILE_API void Report() noexcept;
	};

	extern PROFILE_API void SetProfiler(Profiler* _profiler);
}