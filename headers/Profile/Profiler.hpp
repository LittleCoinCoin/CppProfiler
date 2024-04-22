#pragma once

#include <array> // for the timings and tracks arrays
#include <cstdio> // for printf
#include <type_traits> // for std::conditional_t in U_SIZE_ADAPTER

#include "Export.hpp"
#include "Timing.hpp"
#include "Types.hpp"

#ifdef _PROFILER_ENABLED // Possibly defined as compilation variable
#define PROFILER_ENABLED 1 // Just an alias for _PROFILER_ENABLED to be used as `#if PROFILER_ENABLED` instead of `#ifdef _PROFILER_ENABLED`
#else
#define PROFILER_ENABLED 0
#endif // _PROFILER_ENABLED

namespace Profile
{
#ifndef NB_TIMINGS //Possibly defined as compilation variable
	#define NB_TIMINGS 256 
#endif // !NB_TIMINGS

#ifndef NB_TRACKS //Possibly defined as compilation variable
	#define NB_TRACKS 2
#endif // !NB_TRACKS


#if PROFILER_ENABLED

/*!
@brief Expands to adapt the type of the unsigned integer to be
		u8, u16, u32, or u64 depending on the value of @p x.
		@details For example, if 0 <= x < 256, it will return u8; if 256 <= x < 65532
		it will return u16; and so on.
*/
#define U_SIZE_ADAPTER(x) \
	std::conditional_t<(x < (1<<8)), Profile::u8, \
	std::conditional_t<(x < (1<<16)), Profile::u16, \
	std::conditional_t<(x < (1<<32)), Profile::u32, Profile::u64>>>

/*!
@brief The macro used to adapt the type of the different variables used to
		represent the number or index of Profile Blocks based on the value of the 
		max number of profile blocks (NB_TIMINGS).
@details -1 because U_SIZE_ADAPTER is 0 indexed and NB_TIMINGS is a number of elements, not an index.
*/
#define NB_TIMINGS_TYPE U_SIZE_ADAPTER(NB_TIMINGS-1)

/*!
@brief The macro used to adapt the type of the different variables used to
		represent the number or index of Profile Tracks based on the value of the
		max number of profile tracks (NB_TRACKS).
@details -1 because U_SIZE_ADAPTER is 0 indexed and NB_TIMINGS is a number of elements, not an index.
*/
#define NB_TRACKS_TYPE U_SIZE_ADAPTER(NB_TRACKS-1)

/*!
@brief DO NOT USE in code. Prefer using PROFILE_BLOCK_TIME_BANDWIDTH, PROFILE_FUNCTION_TIME_BANDWIDTH,
		PROFILE_BLOCK_TIME, or PROFILE_FUNCTION_TIME depending on your situation.
		The final macro expanding to generate the unique profile block index
		as well as the profile block opbject itself. 
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileBlockRecorderIdx, byteCount, ...)                                        \
	static NB_TIMINGS_TYPE profileBlockRecorder_##profileBlockRecorderIdx = Profile::Profiler::GetProfileBlockRecorderIndex(trackIdx, __FILE__, __LINE__, blockName); \
	Profile::ProfileBlock ProfiledBlock_##profileBlockRecorderIdx(trackIdx, profileBlockRecorder_##profileBlockRecorderIdx, byteCount, ## __VA_ARGS__)

/*!
@brief DO NOT USE in code. Prefer using PROFILE_BLOCK_TIME_BANDWIDTH, PROFILE_FUNCTION_TIME_BANDWIDTH,
		PROFILE_BLOCK_TIME, or PROFILE_FUNCTION_TIME depending on your situation.
		The intermediate macro expanding PROFILE_BLOCK_TIME_BANDWIDTH__. Used to handle
		the VA_ARGS and the profileBlockRecorderIdx parameters.
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, profileBlockRecorderIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileBlockRecorderIdx, byteCount, ## __VA_ARGS__)

/*!
@brief USE in code. The macro to profile an arbitrary block of code with a name
		you can choose. This macro also accepts a number of bytes in parameter
		to monitor data throughput as well.
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH(blockName, trackIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, __LINE__, ## __VA_ARGS__)

/*!
@brief USE in code. The macro to profile an arbitrary block of code with a name
		you can choose. This expands to PROFILE_BLOCK_TIME_BANDWIDTH with byteCount=0.
		So, you this when you only wish to profile processing time.
*/
#define PROFILE_BLOCK_TIME(blockName, trackIdx, ...) PROFILE_BLOCK_TIME_BANDWIDTH(#blockName, trackIdx, 0, ## __VA_ARGS__)

/*!
@brief USE in code. The macro to profile a function. This macro also accepts a
		number of bytes in parameter to monitor data throughput as well. This
		expands to PROFILE_BLOCK_TIME_BANDWIDTH_ with the function's name as the
		blockName (i.e., using __FUNCTION__).
*/
#define PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, byteCount, ...) PROFILE_BLOCK_TIME_BANDWIDTH_(__FUNCTION__, trackIdx, __LINE__, byteCount, ## __VA_ARGS__)

/*!
@brief USE in code. The macro to profile a function. This expands to PROFILE_FUNCTION_TIME_BANDWIDTH
		with byteCount=0. So, you this when you only wish to profile processing time
		of the function.
*/
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
	@brief An object that will live and die within the scope of a target block
			of code to profile.
	@details The object will open the block upon construction and close it upon
			destruction. The result of the profiling will be forwarded to the
			profile track with the index ::trackIdx in the profiler. The index
			in the track that will actually store the profiling statistics is
			::profileBlockRecorderIdx.
	*/
	struct PROFILE_API ProfileBlock
	{
		/*!
		@brief The index of the profiling track this block belongs to.
		*/
		NB_TIMINGS_TYPE trackIdx = 0;

		/*!
		@brief The index of this block in the profiling track.
		*/
		NB_TIMINGS_TYPE profileBlockRecorderIdx = 0;

		ProfileBlock(NB_TRACKS_TYPE _trackIdx, NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _byteCount);

		~ProfileBlock();
	};

	/*!
	@brief A struct to store the profiling statistics of a profiled block.
	*/
	struct ProfileBlockRecorder
	{
		/*!
		@brief The line number in the file where the block is located.
		*/
		u32 lineNumber = 0;

		/*!
		@brief The name of the file where the block is located.
		*/
		const char* fileName = nullptr;

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
		@brief Update the profiling statistics of the block upon completion.
		@return The time increment since the block was opened.
		*/
		inline u64 Close()
		{
			u64 increment = Timer::GetCPUTimer() - start;
			elapsed += increment;
			return increment;
		}
		
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
		@brief Resets the values of the block.
		@details Resetting do not change the ::blockName, the ::fileName, or the ::lineNumber.
		*/
		void Reset() noexcept;
	};

	/*!
	@brief A mirror of the ProfileBlockRecorder struct to store statistics
			matching  a profiled block and a few more to help with the reporting.
	*/
	struct ProfileBlockResult
	{
		/*!
		@brief The line number in the file where the block is located.
		@details Mirrors ProfileBlockRecorder::lineNumber.
		*/
		u32 lineNumber = 0;

		/*!
		@brief The index of the profiling track this block belongs to.
		@details No equivalent in ProfileBlockRecorder.
		*/
		NB_TRACKS_TYPE trackIdx = 0;

		/*!
		@brief The index of this block in the profiling track.
		@details No equivalent in ProfileBlockRecorder.
		*/
		NB_TIMINGS_TYPE profileBlockRecorderIdx = 0;

		/*!
		@brief The name of the file where the block is located.
		@details Mirrors ProfileBlockRecorder::fileName.
		*/
		const char* fileName = nullptr;

		/*!
		@brief The name of the block.
		@details Mirrors ProfileBlockRecorder::blockName.
		*/
		const char* blockName = nullptr;

		/*!
		@brief The accumulated time the block was executed.
		@details Mirrors ProfileBlockRecorder::elapsed.
		*/
		u64 elapsed = 0;

		/*!
		@brief The converted ::elapsed in seconds.
		@details No equivalent in ProfileBlockRecorder.
		*/
		f64 elapsedSec = 0.0;

		/*!
		@brief The number of times the block was executed.
		@details Mirrors ProfileBlockRecorder::hitCount.
		*/
		u64 hitCount = 0;

		/*!
		@brief The number of bytes processed by the block.
		@details No equivalent in ProfileBlockRecorder.
		*/
		u64 processedByteCount = 0;

		/*!
		@brief The proportion of the block's time in its track's time.
		@details No equivalent in ProfileBlockRecorder.
		*/
		f32 proportionInTrack = 0.0;

		/*!
		@brief The proportion of the block's time in the total time of the profiler.
		@details No equivalent in ProfileBlockRecorder.
		*/
		f32 proportionInTotal = 0.0;

		/*!
		@brief The bandwidth in bytes per second.
		@details No equivalent in ProfileBlockRecorder.
		*/
		f32 bandwidthInB = 0.0;

		ProfileBlockResult() = default;

		/*!
		@brief Captures the statistics of a Profile::ProfileBlockRecorder.
		@details It will effectively assign or compute the values of all the 
				 member variables of this struct.
		*/
		void Capture(ProfileBlockRecorder& _record, NB_TRACKS_TYPE _trackIdx,
			NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _trackElapsedReference, u64 _totalElapsedReference) noexcept;

		/*!
		@brief Outputs the profiling statistics of the block.
		*/
		PROFILE_API void Report() noexcept;
	};

#else // PROFILER_ENABLED

#define PROFILE_BLOCK_TIME_BANDWIDTH(...)
#define PROFILE_BLOCK_TIME(...)
#define PROFILE_FUNCTION_TIME_BANDWIDTH(...)
#define PROFILE_FUNCTION_TIME(...)
#define PROFILER_END_CHECK

#endif // PROFILER_ENABLED

/*!
@brief A container for several profiling blocks.
@details The goal is to profile a series of blocks that are related to each other.
*/
struct ProfileTrack
{
	/*!
	@brief Whether the track is used for at least one block.
	@details This is might be set to true only once a block is added to the track.
			 This is used to avoid outputting the track's statistics, or reseting its data
			 if it has none.
	@see Profile::Profiler::GetProfileBlockRecorderIndex(...)
	*/
	bool hasBlock = false;

	/*!
	@brief The name of the track.
	*/
	const char* name = nullptr;

	/*!
	@brief The accumulated time from all blocks in the track.
	*/
	u64 elapsed = 0;

	/*!
	@brief The profiling statistics of the blocks in the track.
	*/
	std::array<ProfileBlockRecorder, NB_TIMINGS> timings;

	/*!
	@brief Closes a block and updates the track's elapsed time.
	@param _profileBlockRecorderIdx The index of block timing in the track.
	@see Profile::ProfileBlockRecorder::Close()
	*/
	PROFILE_API inline void CloseBlock(NB_TIMINGS_TYPE _profileBlockRecorderIdx)
	{
		elapsed += timings[_profileBlockRecorderIdx].Close();
	}

	/*!
	@brief Opens a block of the track.
	@param _profileBlockRecorderIdx The index of the block timing in the track.
	@param _byteCount The number of bytes processed by the block.
	@see Profile::ProfileBlockRecorder::Open(Profile::u64 _byteCount)
	*/
	PROFILE_API inline void OpenBlock(NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _byteCount)
	{
		timings[_profileBlockRecorderIdx].Open(_byteCount);
	}

	/*!
	@brief Outputs the profiling statistics of all blocks in the track.
	*/
	PROFILE_API void Report(u64 _totalElapsedReference) noexcept;

	/*!
	@brief Resets the values of the track and its blocks.
	@details Resetting do not change the names.
	@see ::ResetTimings
	*/
	PROFILE_API void Reset() noexcept;

	/*!
	@brief Resets the values of all blocks in the track that have a name.
	@details Resetting do not change the names.
	*/
	PROFILE_API void ResetTimings() noexcept;
};

/*!
@brief A mirror of the ProfileTrack struct to store statistics matching a profiled
		track and the blocks it contains (thanks to Profile::ProfileBlockResult).
@details In this mirror, the blocks are packed at the beginning of the array to
		avoid having to iterate over all the blocks that might not have been used.
		The number of blocks used in the track is stored in ::blockCount.
*/
struct ProfileTrackResult
{
	/*!
	@brief The name of the track.
	@details Mirrors ProfileTrack::name.
	*/
	const char* name = nullptr;

	/*!
	@brief The accumulated time from all blocks in the track.
	@details Mirrors ProfileTrack::elapsed.
	*/
	u64 elapsed = 0;

	/*!
	@brief The converted ::elapsed in seconds.
	@details No equivalent in ProfileTrack.
	*/
	f64 elapsedSec = 0.0;

	/*!
	@brief The proportion of the track's time in the total time of the profiler.
	@details No equivalent in ProfileTrack.
	*/
	f64 proportionInTotal = 0.0;

	/*!
	@brief The number of blocks used in the track.
	*/
	NB_TIMINGS_TYPE blockCount = 0;

	/*!
	@brief The array of mirrors to the Profile::ProfileBlockRecorder structs originally
			stored in the track.
	@details The mirrored blocks are packed at the beginning of the array to
			avoid having to iterate over all the blocks that might not have
			been used.
	*/
	std::array<ProfileBlockResult, NB_TIMINGS> timings;

	ProfileTrackResult() = default;

	/*!
	@brief Captures the statistics of a Profile::ProfileTrack.
	@details It will effectively assign or compute the values of all the
			 member variables of this struct. In particular, it will fill the
			 ::timings array with the statistics of the Profile::ProfileBlockRecorders
			 of the track that have been used.
	*/
	void Capture(ProfileTrack& _track, u64 _trackIdx, u64 _totalElapsedReference) noexcept;

	/*!
	@brief Outputs the profiling statistics of the track.
	*/
	PROFILE_API void Report() noexcept;
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
	@brief The tracks in the profiler.
	*/
	std::array<ProfileTrack, NB_TRACKS> tracks;

	Profiler(const char* _name) : name(_name)
	{

	}

	/*!
	@brief Gets an index for a profile result.
	@details The index is determined by the hash of the file name and line number.
	@param _trackIdx The index of the track the profile result belongs to.
	@param _fileName The name of the file where the block is located.
	@param _lineNumber The line number in the file where the block is located.
	@param _blockName The name of the block.
	@return The index of the profile result.
	*/
	static NB_TIMINGS_TYPE GetProfileBlockRecorderIndex(NB_TRACKS_TYPE _trackIdx, const char* _fileName, u32 _lineNumber, const char* _blockName);

	/*!
	@brief Sets the name of the profiler.
	@param _name The name of the profiler.
	*/
	PROFILE_API inline void SetProfilerName(const char* _name) noexcept
	{
		name = _name;
	}

	/*!
	@brief Sets the name of a track.
	@param _trackIdx The index of the track.
	@param _name The name of the track.
	*/
	PROFILE_API inline void SetTrackName(NB_TRACKS_TYPE _trackIdx, const char* _name) noexcept
	{
		if (_trackIdx < NB_TRACKS)
		{
			tracks[_trackIdx].name = _name;
		}
	}

	/*!
	@brief Closes a block.
	@param _trackIdx The index of the track the block belongs to.
	@param _profileBlockRecorderIdx The index of the profile result.
	*/
	PROFILE_API inline void CloseBlock(NB_TRACKS_TYPE _trackIdx, NB_TIMINGS_TYPE _profileBlockRecorderIdx)
	{
		tracks[_trackIdx].CloseBlock(_profileBlockRecorderIdx);
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
	@param _profileBlockRecorderIdx The index of the profile result.
	@param _byteCount The number of bytes processed by the block.
	*/
	PROFILE_API inline void OpenBlock(NB_TRACKS_TYPE _trackIdx, NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _byteCount)
	{
		tracks[_trackIdx].OpenBlock(_profileBlockRecorderIdx, _byteCount);
	}

	/*!
	@brief Outputs the profiling statistics of all tracks in the profiler.
	*/
	PROFILE_API void Report() noexcept;

	/*!
	@brief Resets the profiler's values as well as all its initialized tracks.
	@details Resetting do not change the names.
	*/
	PROFILE_API void Reset() noexcept;

	/*!
	@brief Resets the values of all blocks in all tracks that have a name.
	@details Resetting do not change the names.
	*/
	PROFILE_API void ResetExistingTracks() noexcept;
};

/*!
@brief A global function to set the pointer to the global profiler.
*/
extern PROFILE_API void SetProfiler(Profiler* _profiler);

/*!
@brief A global function to get the pointer to the global profiler.
@details The profiler must be set with ::SetProfiler before calling this function.
@remarks Mainly useful to get the profiler in functions defined in the header, given
		 that the profiler is defined in the source file.
@return The pointer to the global profiler.
*/
extern PROFILE_API Profiler* GetProfiler();

/*!
@brief A mirror of the Profiler struct to store all the statistics of a profiler
		and the tracks it contains (thanks to Profile::ProfileTrackResult).
@brief In this mirror, the tracks are packed at the beginning of the array to
		avoid having to iterate over all the tracks that might not have been used.
*/
struct ProfilerResults
{
	/*!
	@brief The name of the profiler.
	@details Mirrors Profiler::name.
	*/
	const char* name = nullptr;

	/*!
	@brief The time between the initialization (when Profiler::Initialize was called)
			and the end (when Profiler::End was called).
	@details Mirrors Profiler::elapsed.
	*/
	u64 elapsed = 0;

	/*!
	@brief The converted ::elapsed in seconds.
	@details No equivalent in Profile::Profiler.
	*/
	f64 elapsedSec = 0.0;

	/*!
	@brief The number of tracks used in the profiler (i.e, which have at least
			one used block).
	@details No equivalent in Profile::Profiler.
	*/
	NB_TRACKS_TYPE trackCount = 0;

	/*!
	@brief The array of mirrors to the Profile::ProfileTrack structs originally
			stored in the profiler.
	@details The mirrored tracks are packed at the beginning of the array to
			avoid having to iterate over all the tracks that might not have
			been used.
	*/
	std::array<ProfileTrackResult, NB_TRACKS> tracks;

	ProfilerResults() = default;

	/*!
	@brief Captures the statistics of a Profile::Profiler.
	@details It will effectively assign or compute the values of all the
			 member variables of this struct. In particular, it will fill the
			 ::tracks array with the statistics of the Profile::ProfileTracks
			 of the profiler that have been used.
	*/
	void Capture(Profiler* _profiler) noexcept;

	/*!
	@brief Outputs the profiling statistics of the profiler.
	*/
	void Report() noexcept;
};


/*!
@brief A wrapper to test the performance of a function by running it a number of times.
*/
struct RepetitionProfiler
{
	RepetitionProfiler() = default;

	
	ProfilerResults averageResults;

	template<typename... Args>
	void FixedCountRepetitionTesting(ProfilerResults* _out_results, u64 _repetitionCount, void* _function, Args... _functionArgs)
	{
		Profiler* profilerPtr = GetProfiler();

		profilerPtr->Reset();

		for (u64 i = 0; i < _repetitionCount; ++i)
		{
			profilerPtr->Initialize();
			((void(*)(Args...))_function)(_functionArgs...);
			profilerPtr->End();
			_out_results[i].Capture(profilerPtr);
			profilerPtr->ResetExistingTracks();
		}
	}

	void Report(ProfilerResults* _results, u64 _repetitionCount) noexcept;
};
} // namespace Profile