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
#ifndef NB_TIMINGS //Possibly defined at compile time, cannot exceed 256 (u8)
	#define NB_TIMINGS 256 
#endif // !NB_TIMINGS

#ifndef NB_TRACKS //Possibly defined at compile time, cannot exceed 256 (u8)
	#define NB_TRACKS 2
#endif // !NB_TRACKS

#if PROFILER_ENABLED

#define NAME_CONCAT2(a, b) a##b
#define NAME_CONCAT(a, b) NAME_CONCAT2(a, b)

#define PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileResultIdx, byteCount, ...)                                        \
    static Profile::u8 profileResult_##profileResultIdx = Profile::Profiler::GetProfileResultIndex(trackIdx, __FILE__, __LINE__, blockName); \
    Profile::ProfileBlock ProfiledBlock_##profileResultIdx(trackIdx, profileResult_##profileResultIdx, byteCount, ## __VA_ARGS__)
#define PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, profileResultIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileResultIdx, byteCount, ## __VA_ARGS__)

#define PROFILE_BLOCK_TIME_BANDWIDTH(blockName, trackIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, __LINE__, ## __VA_ARGS__)
#define PROFILE_BLOCK_TIME(blockName, trackIdx, ...) PROFILE_BLOCK_TIME_BANDWIDTH(#blockName, trackIdx, 0, ## __VA_ARGS__)

#define PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH_(__FUNCTION__, trackIdx, __LINE__, byteCount, ## __VA_ARGS__)
#define PROFILE_FUNCTION_TIME(trackIdx,...) PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, 0, ## __VA_ARGS__)

	/*!
	@brief A hash function to generate a unique index for a profile result.
	@details The index is determined by the hash of the file name and line number.
	@param _fileName The name of the file where the block is located.
	@param _lineNumber The line number in the file where the block is located.
	*/
	static u64 Hash(const char* _fileName, u32 _lineNumber)
	{
		u64 res = _lineNumber;
		for (const char* At = _fileName; *At; ++At)
		{
			res = res * 65599 + *At;
		}
		res = ((res << 16) ^ (res >> 16)) * 73244475;
		res = ((res << 16) ^ (res >> 16)) * 73244475;
		res = ((res << 16) ^ (res >> 16));
		return res ;
	}

	/*!
	@brief A struct to profile a block of code.
	*/
	struct PROFILE_API ProfileBlock
	{
		/*!
		@brief The index of the profiling track this block belongs to.
		*/
		u8 trackIdx = 0;

		/*!
		@brief The index of this block in the profiling track.
		*/
		u8 profileResultIdx = 0;

		ProfileBlock(u8 _trackIdx, u8 _profileResultIdx, u64 _byteCount);

		~ProfileBlock();
	};

	/*!
	@brief A struct to store the profiling statistices of a profiled block.
	*/
	struct ProfileResult
	{
		/*!
		@brief The name of the file where the block is located.
		*/
		const char* fileName = nullptr;

		/*!
		@brief The line number in the file where the block is located.
		*/
		u32 lineNumber;

		/*!
		@brief The name of the block.
		*/
		const char* blockName = nullptr;

		/*!
		@brief The start time of the block.
		*/
		u64 start = 0;

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

		/*!
		@brief Update the profiling statistics of the block upon execution.
		@param _byteCount The number of bytes processed by the block.
		*/
		inline void Open(u64 _byteCount)
		{
			start = Timer::GetCPUTimer();
			hitCount++;
			processedByteCount += _byteCount;
		}

		/*!
		@brief Update the profiling statistics of the block upon completion.
		*/
		inline void Close()
		{
			elapsed += Timer::GetCPUTimer() - start;
		}
	};

#else // PROFILER_ENABLED

#define PROFILE_BLOCK_TIME_BANDWIDTH(...)
#define PROFILE_BLOCK_TIME(...)
#define PROFILE_FUNCTION_TIME_BANDWIDTH(...)
#define PROFILE_FUNCTION_TIME(...)
#define PROFILER_END_CHECK

#endif // PROFILER_ENABLED

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
		@brief Gets an idex for a profile result.
		@details The index is determined by the hash of the file name and line number.
		@param _trackIdx The index of the track the profile result belongs to.
		@param _fileName The name of the file where the block is located.
		@param _lineNumber The line number in the file where the block is located.
		@param _blockName The name of the block.
		@return The index of the profile result.
		*/
		static u8 GetProfileResultIndex(u8 _trackIdx, const char* _fileName, u32 _lineNumber, const char* _blockName);

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
		@brief Closes a block.
		@param _trackIdx The index of the track the block belongs to.
		@param _profileResultIdx The index of the profile result.
		*/
		PROFILE_API inline void CloseBlock(u8 _trackIdx, u8 _profileResultIdx)
		{
			tracks[_trackIdx].timings[_profileResultIdx].Close();
		}

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
		@brief Opens a block.
		@param _trackIdx The index of the track the block belongs to.
		@param _profileResultIdx The index of the profile result.
		@param _byteCount The number of bytes processed by the block.
		*/
		PROFILE_API inline void OpenBlock(u8 _trackIdx, u8 _profileResultIdx, u64 _byteCount)
		{
			tracks[_trackIdx].timings[_profileResultIdx].Open(_byteCount);
		}

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