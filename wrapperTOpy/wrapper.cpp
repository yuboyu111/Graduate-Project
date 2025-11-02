#include "pic.hpp"

PYBIND11_MODULE(DVS, m)
{
    pybind11::class_<DVSWrapper>(m, "DVSWrapper")
        .def(pybind11::init<std::string, std::string, std::string, int, int, bool>(),
             pybind11::arg("DefautON1") = "true",
             pybind11::arg("RsesON1") = "false",
             pybind11::arg("SetMode1") = "TIME",
             pybind11::arg("TimeSpan1") = 15000,
             pybind11::arg("CountNum1") = 20000,
             pybind11::arg("isRBorGB") = "true")
        .def("Getframe", &DVSWrapper::wrapped_Getframe);
}
