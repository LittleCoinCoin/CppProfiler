#include "pybind11/pybind11.h"
#include "Profile/Profiler.hpp"

#include <pybind11/functional.h>

using namespace pybind11::literals;

struct RestrictedPythonFrameInfo
{
	std::string file;
	int line;
	std::string function;
};

RestrictedPythonFrameInfo getRestrictedPythonFrameInfo()
{
	pybind11::object inspect = pybind11::module::import("inspect");
	pybind11::object frameInfo = inspect.attr("getframeinfo")(inspect.attr("currentframe")());
	return RestrictedPythonFrameInfo{
		frameInfo.attr("filename").cast<std::string>(),
		frameInfo.attr("lineno").cast<int>(),
		frameInfo.attr("function").cast<std::string>()
	};
}

/*!
@brief Profiles a block of code in terms of time and bandwidth. Emulates the macro PROFILE_BLOCK_TIME_BANDWIDTH.
@param _blockName The name of the block to profile.
@param _trackIdx The index of the track to profile.
@param _byteCount The number of bytes to profile.
*/
void ProfileBlockTimeBandwidth(std::string _blockName, int _trackIdx, int _byteCount)
{
	static RestrictedPythonFrameInfo frameInfo = getRestrictedPythonFrameInfo();
	int _line = frameInfo.line;
	PROFILE_BLOCK_TIME_BANDWIDTH__(_blockName.c_str(), _trackIdx, _line, _byteCount, frameInfo.file.c_str(), frameInfo.line);
}

/*!
@brief Profiles a block of code in terms of time. Emulates the macro PROFILE_BLOCK_TIME.
@param _blockName The name of the block to profile.
@param _trackIdx The index of the track to profile.
*/
void ProfileBlockTime(std::string _blockName, int _trackIdx)
{
	static RestrictedPythonFrameInfo frameInfo = getRestrictedPythonFrameInfo();
	int _line = frameInfo.line;
	PROFILE_BLOCK_TIME_BANDWIDTH__(_blockName.c_str(), _trackIdx, _line, 0, frameInfo.file.c_str(), frameInfo.line);
}

/*!
@brief Profiles a function in terms of time and bandwidth. Emulates the macro PROFILE_FUNCTION_TIME_BANDWIDTH.
@brief _trackIdx The index of the track to profile.
@brief _byteCount The number of bytes to profile.
*/
void ProfileFunctionTimeBandwidth(int _trackIdx, int _byteCount)
{
	static RestrictedPythonFrameInfo frameInfo = getRestrictedPythonFrameInfo();
	int _line = frameInfo.line;
	PROFILE_BLOCK_TIME_BANDWIDTH__(frameInfo.function.c_str(), _trackIdx, _line, _byteCount, frameInfo.file.c_str(), frameInfo.line);
}

/*!
@brief Profiles a function in terms of time. Emulates the macro PROFILE_FUNCTION_TIME.
@param _trackIdx The index of the track to profile.
*/
void ProfileFunctionTime(int _trackIdx)
{
	static RestrictedPythonFrameInfo frameInfo = getRestrictedPythonFrameInfo();
	int _line = frameInfo.line;
	PROFILE_BLOCK_TIME_BANDWIDTH__(frameInfo.function.c_str(), _trackIdx, _line, 0, frameInfo.file.c_str(), frameInfo.line);
}

PYBIND11_MODULE(PyProfile, m)
{
    m.doc() = "Python binding module to the a profiling library on C++";

	m.attr("__nbTimings__") = NB_TIMINGS;
	m.attr("__nbTracks__") = NB_TRACKS;

	m.def("SetProfiler", &Profile::SetProfiler, "Sets the profiler to be used.", "_profiler"_a);
	m.def("GetProfiler", &Profile::GetProfiler, "Returns the profiler being used.");
	m.def("ProfileBlockTimeBandwidth", &ProfileBlockTimeBandwidth, "Profiles a block of code in terms of time and bandwidth.", "_blockName"_a, "_trackIdx"_a, "_byteCount"_a);
	m.def("ProfileBlockTime", &ProfileBlockTime, "Profiles a block of code in terms of time.", "_blockName"_a, "_trackIdx"_a);
	m.def("ProfileFunctionTimeBandwidth", &ProfileFunctionTimeBandwidth, "Profiles a function in terms of time and bandwidth.", "_trackIdx"_a, "_byteCount"_a);
	m.def("ProfileFunctionTime", &ProfileFunctionTime, "Profiles a function in terms of time.", "_trackIdx"_a);

	//Add the bindings to the Profile::Profiler class
	pybind11::class_<Profile::Profiler>(m, "Profiler")

		//Add the fields and properties to the class
		.def_property("name",
			[](Profile::Profiler& _self) { return _self.name; },
			&Profile::Profiler::SetProfilerName)

		.def(pybind11::init<>())
		.def("SetTrackName", &Profile::Profiler::SetTrackName, "Sets the name of a track.", "_trackIdx"_a, "_name"_a)
		.def("Clear", &Profile::Profiler::Clear, "Clears the profiler's values as well as all its initialized tracks.")
		.def("ClearTracks", &Profile::Profiler::ClearTracks, "Clears all used blocks of all used tracks in the profiler.")
		.def("End", &Profile::Profiler::End, "Ends the profiler.")
		.def("ExportToCSV", &Profile::Profiler::ExportToCSV, "Exports the profiling statistics of the profiler to a CSV file.", "_path"_a)
		.def("Initialize", &Profile::Profiler::Initialize, "Starts the profiler.")
		.def("Report", &Profile::Profiler::Report, "Outputs the profiling statistics of all tracks in the profiler.")
		.def("Reset", &Profile::Profiler::Reset, "Resets the profiler's values as well as all its initialized tracks.")
		.def("ResetTracks", &Profile::Profiler::ResetTracks, "Resets all used blocks of all used tracks in the profiler.");
		
}