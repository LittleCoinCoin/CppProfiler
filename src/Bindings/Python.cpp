#include "pybind11/pybind11.h"
#include "Profile/Profiler.hpp"


PYBIND11_MODULE(PyProfile, m)
{
    m.doc() = "Python binding module to the a profiling library on C++";


    //Add the bindings to the Profile::Profiler class
    pybind11::class_<Profile::Profiler>(m, "Profiler")
        .def(pybind11::init<>());
}