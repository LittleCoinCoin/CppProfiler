#include "pybind11/pybind11.h"
#include "Profile/Profiler.hpp"

#include <pybind11/functional.h>

using namespace pybind11::literals;

struct LineAndFile
{
	std::string file;
	int line;
};

LineAndFile getLineAndFile()
{
	pybind11::object inspect = pybind11::module::import("inspect");
	pybind11::object frameInfo = inspect.attr("getframeinfo")(inspect.attr("currentframe")());
	return LineAndFile{
		frameInfo.attr("filename").cast<std::string>(),
		frameInfo.attr("lineno").cast<int>() };
}


void ProfileBlockTimeBandwidth__(std::string _blockName, int _trackIdx, int _profileBlockRecorderIdx, int _byteCount)
{
	static LineAndFile lineAndFile = getLineAndFile();
	PROFILE_BLOCK_TIME_BANDWIDTH__(_blockName.c_str(), _trackIdx, _profileBlockRecorderIdx, _byteCount, lineAndFile.file.c_str(), lineAndFile.line);
}

PYBIND11_MODULE(PyProfile, m)
{
    m.doc() = "Python binding module to the a profiling library on C++";

	m.attr("__nbTimings__") = NB_TIMINGS;
	m.attr("__nbTracks__") = NB_TRACKS;

	m.def("SetProfiler", &Profile::SetProfiler, "Sets the profiler to be used.", "_profiler"_a);
	m.def("GetProfiler", &Profile::GetProfiler, "Returns the profiler being used.");
	m.def("ProfileBlockTimeBandwidth__", &ProfileBlockTimeBandwidth__, "Profiles a block of code in terms of time and bandwidth.", "_blockName"_a, "_trackIdx"_a, "_profileBlockRecorderIdx"_a, "_byteCount"_a);
    
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