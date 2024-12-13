#pragma once

#include <array> // for the timings and tracks arrays
#include <cstdio> // for printf
#include <cstdarg> // for va_list
#include <vector> // for storing the functions that will undergo the repetition testing
#include <type_traits> // for std::conditional_t in U_SIZE_ADAPTER

#include "OSStatistics.hpp" // also includes Types.hpp and Export.hpp

namespace Profile
{
#ifndef NB_TIMINGS //Possibly defined as compilation variable
	#define NB_TIMINGS 256 
#endif // !NB_TIMINGS

#ifndef NB_TRACKS //Possibly defined as compilation variable
	#define NB_TRACKS 2
#endif // !NB_TRACKS

#ifndef PROFILE_BLOCK_NAME_LENGTH //Possibly defined as compilation variable
#define PROFILE_BLOCK_NAME_LENGTH 32
#endif // !PROFILE_BLOCK_NAME_LENGTH

#ifndef PROFILE_TRACK_NAME_LENGTH //Possibly defined as compilation variable
	#define PROFILE_TRACK_NAME_LENGTH 32
#endif // !PROFILE_TRACK_NAME_LENGTH

#ifndef PROFILER_NAME_LENGTH //Possibly defined as compilation variable
	#define PROFILER_NAME_LENGTH 32
#endif // !PROFILER_NAME_LENGTH

/*!
@brief Expands to adapt the type of the unsigned integer to be
		u8, u16, u32, or u64 depending on the value of @p x.
		@details For example, if 0 <= x < 256, it will return u8; if 256 <= x < 65532
		it will return u16; and so on.
*/
#define U_SIZE_ADAPTER(x) \
	std::conditional_t<(x < (1<<8)), Profile::u8, \
	std::conditional_t<(x < (1<<16)), Profile::u16, \
	std::conditional_t<(x < (1ULL<<32)), Profile::u32, Profile::u64>>>

/*!
@brief The macro used to adapt the type of the integer used to iterate over the
		Profile Blocks based on the value of the max number of profile blocks (NB_TIMINGS).
*/
#define IT_TIMINGS_TYPE U_SIZE_ADAPTER(NB_TIMINGS)

/*!
@brief The macro used to adapt the type of the integer used to iterate over the
		Profile Tracks based on the value of the max number of profile tracks (NB_TRACKS).
*/
#define IT_TRACKS_TYPE U_SIZE_ADAPTER(NB_TRACKS)

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


#if PROFILER_ENABLED
/*!
@brief DO NOT USE in code. Prefer using PROFILE_BLOCK_TIME_BANDWIDTH, PROFILE_FUNCTION_TIME_BANDWIDTH,
		PROFILE_BLOCK_TIME, or PROFILE_FUNCTION_TIME depending on your situation.
		The final macro expanding to generate the unique profile block index
		as well as the profile block opbject itself. 
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileBlockRecorderIdx, byteCount, file, line)\
	static NB_TIMINGS_TYPE profileBlockRecorder_##profileBlockRecorderIdx = Profile::Profiler::GetProfileBlockRecorderIndex(trackIdx, file, line, blockName); \
	Profile::ProfileBlock ProfiledBlock_##profileBlockRecorderIdx(trackIdx, profileBlockRecorder_##profileBlockRecorderIdx, byteCount)

/*!
@brief DO NOT USE in code. Prefer using PROFILE_BLOCK_TIME_BANDWIDTH, PROFILE_FUNCTION_TIME_BANDWIDTH,
		PROFILE_BLOCK_TIME, or PROFILE_FUNCTION_TIME depending on your situation.
		The intermediate macro expanding PROFILE_BLOCK_TIME_BANDWIDTH__. Used to handle
		the VA_ARGS and the profileBlockRecorderIdx parameters and to pass values of __FILE__ and __LINE__
		by default.
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, profileBlockRecorderIdx, byteCount) PROFILE_BLOCK_TIME_BANDWIDTH__(blockName, trackIdx, profileBlockRecorderIdx, byteCount, __FILE__, __LINE__)

/*!
@brief USE in code. The macro to profile an arbitrary block of code with a name
		you can choose. This macro also accepts a number of bytes in parameter
		to monitor data throughput as well.
*/
#define PROFILE_BLOCK_TIME_BANDWIDTH(blockName, trackIdx, byteCount) PROFILE_BLOCK_TIME_BANDWIDTH_(blockName, trackIdx, __LINE__, byteCount)

/*!
@brief USE in code. The macro to profile an arbitrary block of code with a name
		you can choose. This expands to PROFILE_BLOCK_TIME_BANDWIDTH with byteCount=0.
		So, you this when you only wish to profile processing time.
*/
#define PROFILE_BLOCK_TIME(blockName, trackIdx) PROFILE_BLOCK_TIME_BANDWIDTH(#blockName, trackIdx, 0)

/*!
@brief USE in code. The macro to profile a function. This macro also accepts a
		number of bytes in parameter to monitor data throughput as well. This
		expands to PROFILE_BLOCK_TIME_BANDWIDTH_ with the function's name as the
		blockName (i.e., using __FUNCTION__).
*/
#define PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, byteCount) PROFILE_BLOCK_TIME_BANDWIDTH_(__FUNCTION__, trackIdx, __LINE__, byteCount)

/*!
@brief USE in code. The macro to profile a function. This expands to PROFILE_FUNCTION_TIME_BANDWIDTH
		with byteCount=0. So, you this when you only wish to profile processing time
		of the function.
*/
#define PROFILE_FUNCTION_TIME(trackIdx,...) PROFILE_FUNCTION_TIME_BANDWIDTH(trackIdx, 0)

#else // PROFILER_ENABLED

//In case the profiler is disabled, the macros are defined as empty.
//Some of the macros are not supposed to be used, but we define them anyway in case users do.

#define PROFILE_BLOCK_TIME_BANDWIDTH__(...)
#define PROFILE_BLOCK_TIME_BANDWIDTH_(...)
#define PROFILE_BLOCK_TIME_BANDWIDTH(...)
#define PROFILE_BLOCK_TIME(...)
#define PROFILE_FUNCTION_TIME_BANDWIDTH(...)
#define PROFILE_FUNCTION_TIME(...)

#endif // PROFILER_ENABLED

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
	return res;
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
	@brief The name of the block.
	*/
	char blockName[PROFILE_BLOCK_NAME_LENGTH] = { 0 };

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
	@brief The number of page faults at the start of the block.
	*/
	u64 pageFaultCountStart = 0;

	/*
	@brief The total number of page faults over all executions of the block.
	*/
	u64 pageFaultCountTotal = 0;

	/*!
	@brief The number of bytes processed by the block.
	*/
	u64 processedByteCount = 0;

	/*!
	@brief Sets the name of the block (i.e., ::blockName).
	@param _name The new name of the block.
	@see ::SetBlockNameFmt
	@remarks The name will be truncated if it is longer than PROFILE_BLOCK_NAME_LENGTH
			 characters.
	*/
	PROFILE_API inline void SetBlockName(const char* _name) noexcept
	{
		SetBlockNameFmt(_name);
	}

	/*!
	@brief Sets the name of the block with a format string.
	@param _fmt The format string of the name of the block.
	@param ... The arguments to the format string.
	@see ::SetBlockName
	@remarks The name will be truncated if it is longer than PROFILE_BLOCK_NAME_LENGTH
			 characters.
	*/
	PROFILE_API void SetBlockNameFmt(const char* _fmt, ...);

	/*!
	@brief Clears the values of the block.
	@remarks There is no difference between this and ::Reset. We are keeping
			 both for consistency with the other structs.
	*/
	PROFILE_API void Clear() noexcept;

	/*!
	@brief Update the profiling statistics of the block upon completion.
	@return The time increment since the block was opened.
	*/
	inline u64 Close()
	{
		u64 increment = Timer::GetCPUTimer() - start;
		elapsed += increment;
		pageFaultCountTotal += (Surveyor::GetOSPageFaultCount() - pageFaultCountStart);
		return increment;
	}

	/*!
	@brief Update the profiling statistics of the block upon execution.
	@param _byteCount The number of bytes processed by the block.
	*/
	inline void Open(u64 _byteCount)
	{
		start = Timer::GetCPUTimer();
		pageFaultCountStart = Surveyor::GetOSPageFaultCount();
		hitCount++;
		processedByteCount += _byteCount;
	}

	/*!
	@brief Resets the values of the block.
	@remarks There is no difference between this and ::Clear. We are keeping
			 both for consistency with the other structs.
	*/
	PROFILE_API void Reset() noexcept;
};

/*!
@brief A mirror of the ProfileBlockRecorder struct to store statistics
		matching  a profiled block and a few more to help with the reporting.
*/
struct ProfileBlockResult
{
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
	@brief The total number of page faults over all executions of the block.
	@details Mirrors ProfileBlockRecorder::pageFaultCountTotal.
	*/
	u64 pageFaultCountTotal = 0;

	/*!
	@brief The number of bytes processed by the block.
	@details Mirrors ProfileBlockRecorder::processedByteCount.
	*/
	u64 processedByteCount = 0;

	/*!
	@brief The proportion of the block's time in its track's time.
	@details No equivalent in ProfileBlockRecorder.
	*/
	f32 proportionInTrack = 0.f;

	/*!
	@brief The proportion of the block's time in the total time of the profiler.
	@details No equivalent in ProfileBlockRecorder.
	*/
	f32 proportionInTotal = 0.f;

	/*!
	@brief The bandwidth in bytes per second.
	@details No equivalent in ProfileBlockRecorder.
	*/
	f32 bandwidthInB = 0.f;

	ProfileBlockResult() = default;

	/*!
	@brief Captures the statistics of a Profile::ProfileBlockRecorder.
	@details It will effectively assign or compute the values of all the
				member variables of this struct.
	*/
	PROFILE_API void Capture(ProfileBlockRecorder& _record, NB_TRACKS_TYPE _trackIdx,
				NB_TIMINGS_TYPE _profileBlockRecorderIdx, u64 _trackElapsedReference, u64 _totalElapsedReference) noexcept;

	/*!
	@brief Clears the values of the block.
	@remarks This resets even the ::blockName.
			 If you are looking to reset the values without changing the name,
			 use ::Reset.

	*/
	PROFILE_API void Clear() noexcept;

	/*!
	@brief Outputs the profiling statistics of the block.
	*/
	PROFILE_API void Report() noexcept;

	/*!
	@brief Resets the values of this struct.
	@details Resetting do not change the ::blockName.
	*/
	PROFILE_API void Reset() noexcept;
};

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
	char name[PROFILE_TRACK_NAME_LENGTH] = {0};

	/*!
	@brief The accumulated time from all blocks in the track.
	*/
	u64 elapsed = 0;

	/*!
	@brief The profiling statistics of the blocks in the track.
	*/
	std::array<ProfileBlockRecorder, NB_TIMINGS> timings;

	/*
	@brief Clears the values of the track and its blocks.
	@remarks This resets even the ::name and ::elapsed.
			 If you are looking to reset the values without changing the name,
			 use ::Reset.
			 If you are looking to reset the timings only, use ::ResetTimings.
	*/
	PROFILE_API void Clear() noexcept;

	/*
	@brief Clears the values of all blocks in ::timings.
	@remarks This also resets ::hasBlock to false.
	*/
	PROFILE_API void ClearTimings() noexcept;

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
	PROFILE_API void Capture(ProfileTrack& _track, u64 _trackIdx, u64 _totalElapsedReference) noexcept;

	/*!
	@brief Clears the values of member variables of this struct and up to
			::blockCount Profile::ProfileBlockResults in the ::timings array.
	@remarks This resets even the ::name. 
			 If you are looking to reset the values without changing the name,
			 use ::Reset.
	*/
	PROFILE_API void Clear() noexcept;

	/*!
	@brief Outputs the profiling statistics of the track.
	*/
	PROFILE_API void Report() noexcept;

	/*!
	@brief Resets the values of member variables of this struct and up to
			::blockCount Profile::ProfileBlockResults in the ::timings array.
	@details Resetting do not change the name.
	*/
	PROFILE_API void Reset() noexcept;
};

/*!
@brief A struct to manage the profiling of a program.
*/
struct Profiler
{
	/*!
	@brief The name of the profiler.
	*/
	char name[PROFILER_NAME_LENGTH] = { 0 };

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

	/*!
	@brief The longest name of a block in the profiler.
	@details This is used to align the output of the statistics in ::Report.
	*/
	u16 longestBlockName = 0;

	Profiler() = default;

	/*!
	@brief Gets an index for a profile result.
	@details The index is determined by the hash of the file name and line number.
	@param _trackIdx The index of the track the profile result belongs to.
	@param _fileName The name of the file where the block is located.
	@param _lineNumber The line number in the file where the block is located.
	@param _blockName The name of the block.
	@return The index of the profile result.
	*/
	PROFILE_API static NB_TIMINGS_TYPE GetProfileBlockRecorderIndex(NB_TRACKS_TYPE _trackIdx, const char* _fileName, u32 _lineNumber, const char* _blockName);

	/*!
	@brief Sets the name of the profiler.
	@param _name The name of the profiler.
	@see ::SetProfilerNameFmt
	@remarks The name will be truncated if it is longer than PROFILER_NAME_LENGTH
			 characters.
	*/
	PROFILE_API inline void SetProfilerName(const char* _name) noexcept
	{
		SetProfilerNameFmt(_name);
	}

	/*!
	@brief Sets the name of the profiler with a format string.
	@param _fmt The format string of the name of the profiler.
	@param ... The arguments to the format string.
	@see ::SetProfilerName
	@remarks The name will be truncated if it is longer than PROFILER_NAME_LENGTH
			 characters.
	*/
	PROFILE_API void SetProfilerNameFmt(const char* _fmt, ...);

	/*!
	@brief Sets the name of a track.
	@param _trackIdx The index of the track.
	@param _name The name of the track.
	@see ::SetTrackNameFmt
	@remarks The name will be truncated if it is longer than PROFILE_TRACK_NAME_LENGTH
			 characters.
	*/
	PROFILE_API inline void SetTrackName(NB_TRACKS_TYPE _trackIdx, const char* _name) noexcept
	{
		if (_trackIdx < NB_TRACKS)
		{
			SetTrackNameFmt(_trackIdx, _name);
		}
	}
	
	/*!
	@brief Sets the name of a track with a format string.
	@param _trackIdx The index of the track.
	@param _fmt The format string of the name of the track.
	@param ... The arguments to the format string.
	@see ::SetTrackName
	@remarks The name will be truncated if it is longer than PROFILE_TRACK_NAME_LENGTH
			 characters.
	*/
	PROFILE_API void SetTrackNameFmt(NB_TRACKS_TYPE _trackIdx, const char* _fmt, ...);

	/*!
	@brief Clears the profiler's values as well as all its initialized tracks.
	@remarks This resets even the ::name.
			 If you are looking to reset the values without changing the name,
			 use ::Reset.
			 If you are looking to reset the tracks only, use ::ClearTracks.
	*/
	PROFILE_API void Clear() noexcept;

	/*!
	@brief Clears all used blocks of all used tracks in the profiler.
	@remarks If you are looking to reset the values without changing the names,
			 use ::ResetTracks.
	*/
	PROFILE_API void ClearTracks() noexcept;

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
	@brief Exports the profiling statistics of the profiler to a CSV file.
	@details The logic to create the directories where the file is stored MUST be handled outside
			 before calling this function. The file will be overwritten if it already exists.
			 The file starts with two lines giving the headers for statistics of the profiler
			 followed by the associated numbers. Then, a line defines new headers for the
			 statistics of the blocks in the tracks. The following lines give the statistics
			 of the blocks in the tracks. The statistics of the blocks are ordered by the
			 order of the tracks in the profiler, and then by the order of the blocks in the tracks.
	@param _path The path of the CSV file.
	*/
	PROFILE_API void ExportToCSV(const char* _path) noexcept;

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
		//The track is used if a block is added.
		//This is used to avoid outputting the track's statistics, or reseting its data if it has none.
		//But it is a WRITE operation wasted for every time that is not the first time.
		tracks[_trackIdx].hasBlock = true;

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
	PROFILE_API void ResetTracks() noexcept;
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
@details In this mirror, the tracks are packed at the beginning of the array to
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
	PROFILE_API void Capture(Profiler* _profiler) noexcept;

	/*!
	@brief Clears the values of member variables of this struct and up to
			::trackCount Profile::ProfileTrackResults in the ::tracks array.
	@remarks This resets even the ::name.
			 If you are looking to reset the values without changing the name,
			 use ::Reset.
	*/
	PROFILE_API void Clear() noexcept;

	/*!
	@brief Exports the profiling statistics of the profiler to a CSV file.
	@details The logic to create the directories where the file is stored MUST be handled outside
			 before calling this function. The file will be overwritten if it already exists.
			 The file starts with two lines giving the headers for statistics of the profiler
			 followed by the associated numbers. Then, a line defines new headers for the
			 statistics of the blocks in the tracks. The following lines give the statistics
			 of the blocks in the tracks. The statistics of the blocks are ordered by the
			 order of the tracks in the profiler, and then by the order of the blocks in the tracks.
	@param _path The path of the CSV file.
	@remarks This is equivalent to calling ::ExportToCSV on the profiler if you have captured
			 the statistics with ::Capture.
	*/
	PROFILE_API void ExportToCSV(const char* _path) noexcept;

	/*!
	@brief Outputs the profiling statistics of the profiler.
	*/
	PROFILE_API void Report() noexcept;

	/*!
	@brief Resets the values of member variables of this struct and up to
			::trackCount Profile::ProfileTrackResults in the ::tracks array.
	@details Resetting do not change the name.
	*/
	PROFILE_API void Reset() noexcept;
};

/*!
@brief A functor to wrap around code that will be profiled multiple times
		via the Profile::RepetitionProfiler.
@see ::RepetitionProfiler::FixedCountRepetitionTesting
*/
struct RepetitionTest
{	
	const char* name = nullptr;

	RepetitionTest() = default;

	/*!
	@brief Constructs a RepetitionTest with a name.
	@param _name The name of the test.
	*/
	RepetitionTest(const char* _name) : name(_name){}
	~RepetitionTest() = default;

	/*!
	@brief The operator to override to wrap around the code to profile.
	*/
	PROFILE_API virtual void operator()() = 0;
};


/*!
@brief A wrapper to test the performance of a function by running it a number of times.
*/
struct RepetitionProfiler
{
	RepetitionProfiler() = default;

	/*!
	@brief The pointer to the ProfilerResults storing the results of the
			repeated profiling.
	*/
	ProfilerResults* ptr_repetitionResults = nullptr;
	
	/*!
	@brief A result structure to store the average of the repeated profiling.
	*/
	ProfilerResults averageResults;

	/*!
	@brief A result structure to store the maximum of the repeated profiling.
	*/
	ProfilerResults maxResults;

	/*!
	@brief A result structure to store the minimum of the repeated profiling.
	*/
	ProfilerResults minResults;

	/*!
	@brief A result structure to store the variance of the repeated profiling.
	*/
	ProfilerResults varianceResults;

	/*!
	@brief Sets the pointer for ::ptr_repetitionResults.
	@param _repetitionResults The pointer to the ProfilerResults storing the 
			results of the repeated profiling.
	*/
	inline void SetRepetitionResults(ProfilerResults* _repetitionResults) noexcept
	{
		ptr_repetitionResults = _repetitionResults;
	}

private:

	/*!
	@brief The vector of the function wrappers to profile multiple times.
	@details The functions will be profiled in the order they were added.
			 It will be used in ::FixedCountRepetitionTesting, and ::BestPerfSearchRepetitionTesting
	@see ::PushBackRepetitionTest, ::ClearRepetitionTests, ::RemoveRepetitionTest,
		 ::PopBackRepetitionTest
	*/
	std::vector<RepetitionTest*> repetitionTests;

	/*!
	@brief A function to assign the maximum of two values to the first one.
	@details The function is overloaded for u64, f32, and f64. It's used to
			 lighten the code in ::FindMaxResults.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MaxAssign(u64& _a, u64& _b) noexcept
	{
		if (_a < _b)
		{
			_a = _b;
		}
	}

	/*!
	@brief An overloaded function to assign the maximum of two values to the first one.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MaxAssign(f32& _a, f32& _b) noexcept
	{
		if (_a < _b)
		{
			_a = _b;
		}
	}

	/*!
	@brief An overloaded function to assign the maximum of two values to the first one.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MaxAssign(f64& _a, f64& _b) noexcept
	{
		if (_a < _b)
		{
			_a = _b;
		}
	}

	/*!
	@brief A function to assign the minimum of two values to the first one.
	@details The function is overloaded for u64, f32, and f64. It's used to
			 lighten the code in ::FindMinResults.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MinAssign(u64& _a, u64& _b) noexcept
	{
		if (_a == 0)
		{
			_a = _b;
		}

		else if (_a > _b)
		{
			_a = _b;
		}
	}

	/*!
	@brief An overloaded function to assign the minimum of two values to the first one.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MinAssign(f32& _a, f32& _b) noexcept
	{
		if (_a == 0.0f)
		{
			_a = _b;
		}

		else if (_a > _b)
		{
			_a = _b;
		}
	}

	/*!
	@brief An overloaded function to assign the minimum of two values to the first one.
	@param _a The first value.
	@param _b The second value.
	*/
	inline void MinAssign(f64& _a, f64& _b) noexcept
	{
		if (_a == 0.0)
		{
			_a = _b;
		}

		else if (_a > _b)
		{
			_a = _b;
		}
	}

public:

	/*!
	@brief Pushes back a wrapper of a function to profile multiple times to
			::repetitionTests.
	*/
	PROFILE_API inline void PushBackRepetitionTest(RepetitionTest* _repetitionTest) noexcept
	{
		repetitionTests.push_back(_repetitionTest);
	}

	/*!
	@brief Clears all function wrappers in ::repetitionTests.
	*/
	PROFILE_API inline void ClearRepetitionTests() noexcept
	{
		repetitionTests.clear();
	}

	/*
	@brief Removes a function wrapper from ::repetitionTests.
	@param _index The index of the function wrapper to remove.
	*/
	PROFILE_API inline void RemoveRepetitionTest(u16 _index) noexcept
	{
		if (_index < repetitionTests.size())
		{
			repetitionTests.erase(repetitionTests.begin() + _index);
		}
	}

	/*!
	@brief Removes the function wrapper at the back of ::repetitionTests.
	*/
	PROFILE_API inline void PopBackRepetitionTest() noexcept
	{
		repetitionTests.pop_back();
	}

	/*!
	@brief Clears all the stored and computed results of the repeated profiling.
	@details The function will clear the values of ::averageResults, ::varianceResults,
			 ::maxResults, and ::minResults. It also clears everything in the
			 ProfilerResults pointed by ::ptr_repetitionResults.
	@param _repetitionCount The number of repetitions.
	@see Profile::ProfilerResults::Clear
	@remarks If you don't want to clear the names of all the data concerned by the
			 repeated profiling, you can use ::Reset.
	*/
	PROFILE_API void Clear(u64 _repetitionCount) noexcept;

	/*!
	@brief Computes the average of the repeated profiling.
	@param _repetitionCount The number of repetitions.
	@see ::averageResults
	*/
	PROFILE_API void ComputeAverageResults(u64 _repetitionCount) noexcept;

	/*!
	@brief Computes the variance of the repeated profiling.
	@param _repetitionCount The number of repetitions.
	@see ::varianceResults
	*/
	PROFILE_API void ComputeVarianceResults(u64 _repetitionCount) noexcept;

	/*!
	@brief Goes through the repeated profiling to find the maximum results.
	@details In the current implementation, there is no guarentee that the
			 maximum results all come from the same repetition. For example,
			 for some reason, It's possible a block was hit more times in a
			 repetition r1 than r2, but the maximum time was recorded in r2.
	@param _repetitionCount The number of repetitions.
	@see ::maxResults
	*/
	PROFILE_API void FindMaxResults(u64 _repetitionCount) noexcept;

	/*!
	@details Goes through the repeated profiling to find the minimum results.
	@details In the current implementation, there is no guarentee that the
			 minimum results all come from the same repetition. For example,
			 for some reason, It's possible a block was hit more times in a
			 repetition r1 than r2, but the minimum time was recorded in r2.
	@param _repetitionCount The number of repetitions.
	@see ::minResults
	*/
	PROFILE_API void FindMinResults(u64 _repetitionCount) noexcept;

	/*!
	@brief Repeatedly tests all functions wrapped in ::repetitionTests for as long
			as allowed by the time out parameters and reports only about the best
			performance.
	@details The function will keep running until the @p _globalTimeOut is reached.
			 We garentee that all functions will be tested at least once. In
			 particular, in case there is a user mistake and @p _globalTimeOut
			 is shorter than @p _repetitionTestTimeOut, the latter will be used
			 such that at least one test is done for each function. When a new
			 best performance is found, the time out is reset by @p _repetitionTestTimeOut.
			 The best profiling results of each function will be stored in the
			 ProfilerResults pointed by ::ptr_repetitionResults. In this case,
			 ::ptr_repetitionResults must be set before calling this function
			 and must be an array of at least of the size of ::repetitionTests.
	@param _repetitionTestTimeOut The time out in seconds for each repetition test.
	@param _reset Whether to reset the results before testing. Default is false.
				  See ::Reset, ::Profiler::Reset, and ::Profiler::ResetTracks.
	@param _clear Whether to clear the results before testing. Default is true.
					See ::Clear, ::Profiler::Clear, and ::Profiler::ClearTracks.
	@param _globalTimeOut The time out in seconds for the whole testing.
	*/
	PROFILE_API void BestPerfSearchRepetitionTesting(u16 _repetitionTestTimeOut, bool _reset = false, bool _clear = true, u16 _globalTimeOut = 0xFFFFu);

	/*!
	@brief Exports the profiling statistics of the repeated profiling to a CSV file.
	@details Different from ::Profiler::ExportToCSV, the @p _path must be a directory
			 where the files will be stored and not directly the name of a file.
			 The files will be named after the nature of the profiling statistics
			 they contain. Namely, the files will be named "Average.csv", "Variance.csv",
			 "Max.csv", and "Min.csv". The individual profiling statistics of each
			 repetition will also be exported to CSV files named after the repetition
			 number. The summary statistics will be in the directory "_path/Summary"
			 and the individual statistics in the directory "_path/Repetitions".
	@param _path The path of the directory where the CSV files will be stored.
	@param _repetitionCount The number of repetitions to export. It cannot be greater
			than the size of ::ptr_repetitionResults.
	*/
	PROFILE_API void ExportToCSV(const char* _path, u64 _repetitionCount) noexcept;

	/*!
	@brief Repeatedly tests all functions wrapped in ::repetitionTests and consecutively
			reports the profiling statistics.
	@details The function will be called @p _repetitionCount times. The profiling
			 statistics of each repetition will be stored in the ProfilerResults
			 pointed by ::ptr_repetitionResults. In this case, ::ptr_repetitionResults
			 must be set before calling this function and must be an array of at least
			 of size @p _repetitionCount.
	@param _repetitionCount The number of repetitions.
	@param _reset Whether to reset the results before testing. Default is true.
				  See ::Reset, ::Profiler::Reset, and ::Profiler::ResetTracks.
	@param _clear Whether to clear the results before testing. Default is false.
				  See ::Clear, ::Profiler::Clear, and ::Profiler::ClearTracks.
	*/
	PROFILE_API void FixedCountRepetitionTesting(u64 _repetitionCount, bool _reset = true, bool _clear = false);

	/*!
	@brief Prints the results of the repeated profiling.
	@details If none have been computed yet (i.e., the corresponding Profile::ProfilingResults
			 have their names set to nullptr), the function will compute the
			 average, the variance, the maximum, and the minimum of
			 the repeated profiling before outputting the results.
	@param _repetitionCount The number of repetitions.
	*/
	PROFILE_API void Report(u64 _repetitionCount) noexcept;

	/*!
	@brief Resets all the stored and computed results of the repeated profiling. 
	@details The function will reset the values of ::averageResults, ::varianceResults,
			 ::maxResults, and ::minResults. It also resets everything in the
			 ProfilerResults pointed by ::ptr_repetitionResults.
	@param _repetitionCount The number of repetitions.
	@see Profile::ProfilerResults::Reset
	*/
	PROFILE_API void Reset(u64 _repetitionCount) noexcept;
};
} // namespace Profile