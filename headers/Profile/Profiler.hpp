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
#ifndef NB_TIMINGS //Possibly defined at compile time
	#define NB_TIMINGS 1024
#endif // !NB_TIMINGS

#ifndef NB_TRACKS //Possibly defined at compile time, cannot exceed 256 (u8)
	#define NB_TRACKS 2
#endif // !NB_TRACKS

#if PROFILER_ENABLED

	/*!
	@brief A struct to profile a block of code.
	*/
	struct PROFILE_API ProfileBlock
	{
		/*!
		@brief The name of the block.
		*/
		const char* name;

		/*!
		@brief The start time of the block.
		*/
		u64 start;

		/*!
		@brief The accumulated time every the program went through the block.
		*/
		u64 elapsedBuffer;

		/*!
		@brief The index of the profiling track this block belongs to.
		*/
		u16 trackIdx;

		/*!
		@brief The index of this block in the profiling track.
		*/
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

	/*!
	@brief A struct to store the profiling statistices of a profiled block.
	*/
	struct ProfileResult
	{
		/*!
		@brief The name of the block.
		*/
		const char* name = nullptr;

		/*!
		@brief The accumulated time the block was executed.
		*/
		u64 elapsed = 0;

		/*!
		@brief The number of times the block was executed.
		*/
		u64 hitCount = 0;

		/*!
		@brief The number of bytes processed by the block.
		*/
		u64 processedByteCount = 0;
	};

	/*!
	@brief A struct to define a track in the profiler that will contain profile
			blocks.	
	*/
	struct ProfileTrack
	{
		/*!
		@brief The name of the track.
		*/
		const char* name = nullptr;

		/*!
		@brief The start time of the track.
		*/
		u64 start = 0;

		/*!
		@brief The accumulated time since the track was initialized (when
				::Initialize was called).
		*/
		u64 elapsed = 0;

		/*!
		@brief The profiling statistics of the blocks in the track.
		*/
		std::array<ProfileResult, NB_TIMINGS> timings;

		/*!
		@brief Adds (or updates) the profiling statistics of a block at index
				@p _timingIdx in ::timings.
		@param _name The name of the block. (Unfortunatly, this is added every
					 time a result is added).
		@param _elapsed The total time the block took to execute, including every
						previous execution.
		@param _timingIdx The index of the block in ::timings.
		*/
		inline void AddResult(const char* _name, u64 _elapsed, u16 _timingIdx)
		{
			timings[_timingIdx].name = _name;
			timings[_timingIdx].elapsed = _elapsed;
			++timings[_timingIdx].hitCount;
		}
		
		/*!
		@brief Starts the track.
		@details Sets ::start to the current time.
		*/
		PROFILE_API void Initialize() noexcept;

		/*!
		@brief Ends the track.
		@details Sets ::elapsed to the time since ::Initialize was called.
		*/
		PROFILE_API void End() noexcept;

		/*!
		@brief Outputs the profiling statistics of all blocks in the track.
		*/
		PROFILE_API void Report(u64 _totalElapsedReference) noexcept;
	};

	/*!
	@brief A struct to manage the profiling of a program.
	*/
	struct Profiler
	{
		/*!
		@brief The name of the profiler.
		*/
		const char* name = nullptr;

		/*!
		@brief The time when the profiler was initialized (when ::Initialize
				was called).
		*/
		u64 start = 0;

		/*!
		@brief The time since the profiler was initialized.
		@details This is updated only when ::End is called.
		*/
		u64 elapsed = 0;

		/*!
		@brief The number of tracks in the profiler.
		@details This counts the tracks in ::tracks that have been given a name. 
		@remarks It cannot exceed NB_TRACKS.
		*/
		u8 trackCount = 0;

		/*!
		@brief The tracks in the profiler.
		*/
		std::array<ProfileTrack, NB_TRACKS> tracks;

		Profiler(const char* _name) : name(_name)
		{

		}

		/*!
		@brief Adds a track to the profiler.
		@details The track will be added only if the profiler has not reached
				 the maximum number of tracks (NB_TRACKS). Effectively, "adding
				 a track" means giving a name to the first track in ::tracks
				 that has not been given one yet.
		@param _name The name of the track.
		*/
		PROFILE_API bool AddTrack(const char* _name);

		/*!
		@brief Ends the profiler.
		@details Sets ::elapsed to the time since ::Initialize was called.
		*/
		PROFILE_API void End() noexcept;

		/*!
		@brief Starts the profiler.
		@details Sets ::start to the current time.
		*/
		PROFILE_API void Initialize() noexcept;

		/*!
		@brief Outputs the profiling statistics of all tracks in the profiler.
		*/
		PROFILE_API void Report() noexcept;
	};

	/*!
	@brief A global function to set the pointer to the profiler that will be
			used by the profiling macros.
	*/
	extern PROFILE_API void SetProfiler(Profiler* _profiler);
}