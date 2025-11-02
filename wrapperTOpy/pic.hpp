#ifndef PIC_H
#define PIC_H

#include "DVS.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h> // numpy 支持
#include <ctime>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

class DVSWrapper
{
public:
    DVS dvs;
    
    bool defaut_pic_flag, rses_pic_flag;

    DVSWrapper(std::string DefautON1, std::string RsesON1, std::string SetMode1, int TimeSpan1, int CountNum1, bool isRBorGB);
    ~DVSWrapper();
    pybind11::array mat_to_numpy(const cv::Mat &mat);
    cv::Mat Getframe();
    pybind11::array wrapped_Getframe();
};

#endif // PIC_H