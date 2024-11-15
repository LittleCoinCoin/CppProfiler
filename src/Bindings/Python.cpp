#include "pybind11/pybind11.h"
#include "Profile/Profiler.hpp"

using namespace pybind11::literals;

PYBIND11_MODULE(PyProfile, m)
{
    m.doc() = "Python binding module to the a profiling library on C++";

	m.attr("__nbTimings__") = NB_TIMINGS;
	m.attr("__nbTracks__") = NB_TRACKS;


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