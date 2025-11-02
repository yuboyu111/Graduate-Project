#include "DVS.h"
#include <ctime>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

int main()
{
    DVS dvs;

    cv::Mat resize;
    time_t raw_time;
    cv::Mat frame_1, frame_2;
    cv::Mat RotatedFrame_1, RotatedFrame_2;
    bool defaut_pic_flag, rses_pic_flag;

    // 读取json配置文件
    std::ifstream file_config("../command_line.json");
    if (!file_config.is_open())
    {
        std::cerr << "Failed to open commamd_line.json" << std::endl;
        return -1;
    }

    nlohmann::json config; // 实例化config
    file_config >> config; // 把文件读到config对象

    const std::string DefautON = config["defaut"];
    const std::string RsesON = config["rses"];
    const std::string SetMode = config["mode"];
    const unsigned int TimeSpan = config["timespan"];
    const unsigned int CountNum = config["countnum"];
    const bool isRBorGB = (config["isRBorGB"]) == "true";

    dvs.addModule("default");
    dvs.addModule("RSES");
    dvs.Get_Choice(DefautON, RsesON, defaut_pic_flag, rses_pic_flag);

    if (SetMode == "TIME")
    {
        dvs.setMode(DVS::TIME);
    }
    else if (SetMode == "COUNT")
    {
        dvs.setMode(DVS::COUNT);
    }
    dvs.set_isRBorGB(isRBorGB);

    dvs.setTimeSpan(TimeSpan);
    dvs.setCountNum(CountNum);

    if (dvs.isConnected())
    {
        dvs.run();
        // std::thread recording_thread(recording_with_time, std::ref(dvs));
        // std::thread log_thread = std::thread(&DVS::log, &dvs);
        while (1)
        {
            // 假设调用 addModule("defaut")，然后调用 addModule("RSES")，此时 module_list 中的元素顺序为：[DefautModule, RSESModule]。调用 getFrame(1)，其中 1 是 RSESModule 在 module_list 中的索引
            if (!defaut_pic_flag && !rses_pic_flag)
                break;

            if (defaut_pic_flag)
            {
                frame_1 = dvs.getFrame(0);                         // 获取帧并存储在变量中
                RotatedFrame_1 = dvs.Do_Rotation(frame_1, 180, 1.5); // 旋转角度和放大比例,如(frame_1, 180.0, 1.5)
                cv::imshow("frame_1", RotatedFrame_1);             // 显示图像
                // cv::resizeWindow("frame_1", 960, 720);             // 固定窗口大小
                // cv::moveWindow("frame_1", 50, 50);                 // 设置窗口左上角位置，避免和回放窗口重叠
            }
            if (rses_pic_flag)
            {
                frame_2 = dvs.getFrame(1);                         // 获取帧并存储在变量中
                RotatedFrame_2 = dvs.Do_Rotation(frame_2, 180, 1.5); // 旋转角度和放大比例
                cv::imshow("frame_2", RotatedFrame_2);             // 显示图像
                cv::moveWindow("frame_2", 50, 50);                 // 设置窗口左上角位置，避免和回放窗口重叠
            }

            // cv::resize(dvs.getFrame(0), resize, {960, 720});
            // cv::imshow("frame", resize);
            auto key = cv::waitKey(5);

            if (key == 27)
                break;

            else if (key == 't')
            {
                dvs.TakePhoto(frame_1, frame_2);
            }

            else if (key == 'r')
            {
                dvs.replayPastFrames(!isRBorGB);
            }

            else if (key == 's')
            {
                time(&raw_time);
                auto info = localtime(&raw_time);
                std::string name = "../save/save_dat/" + std::to_string(info->tm_year + 1900) + "_" + std::to_string(info->tm_mon + 1) + "_" + std::to_string(info->tm_mday) + "_" + std::to_string(info->tm_hour) + "_" + std::to_string(info->tm_min) + "_" + std::to_string(info->tm_sec) + ".dat";
                dvs.startRecord(name);
                std::cout << "start recording .dat document" << std::endl;
            }
            else if (key == 'd')
            {
                dvs.stopRecord();
                std::cout << "stop recording .dat document" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "no connection" << std::endl;
    }
    // if (recording_thread.joinable()) {
    //     recording_thread.join();
    // }
    // if (log_thread.joinable())
    // {
    //     log_thread.join();
    // }
    return 0;
}
