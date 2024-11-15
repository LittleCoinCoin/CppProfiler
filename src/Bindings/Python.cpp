#include "pybind11/pybind11.h"
#include "Profile/Profiler.hpp"


PYBIND11_MODULE(PyProfile, m)
{
    m.doc() = "Python binding module to the a profiling library on C++";

	m.attr("__nbTimings__") = NB_TIMINGS;
	m.attr("__nbTracks__") = NB_TRACKS;


    //Add the bindings to the Profile::Profiler class
	pybind11::class_<Profile::Profiler>(m, "Profiler")
		.def_property("name",
			[](Profile::Profiler& _self) { return _self.name; },
			&Profile::Profiler::SetProfilerName)
		.def(pybind11::init<>());
}