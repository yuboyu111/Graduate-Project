#include "../include/Module.h"
#include "../include/DataQueue.h"

DefaultModule::DefaultModule(int height, int width) : _height(height), _width(width) {}

DefaultModule::~DefaultModule()
{
}

void DefaultModule::convertEvent(std::vector<Event> &packet, bool is_RBorGB)
{

    if (!is_RBorGB)
    {
        _frame = cv::Mat::zeros(_height, _width, CV_8UC3);
        for (auto &event : packet)
        {
            _frame.ptr<cv::Vec3b>(event.y)[event.x][event.p] = 255;
        }
    }
    else
    {
        _frame = cv::Mat(_height, _width, CV_8UC3, cv::Scalar(255, 255, 255));
        for (auto &event : packet)
        {
            cv::Vec3b &pixel = _frame.ptr<cv::Vec3b>(event.y)[event.x];
            // 事件极性为 true -> 红色；false -> 蓝色
            if (event.p)
                pixel = cv::Vec3b(0, 0, 255); // 红色：BGR格式
            else
                pixel = cv::Vec3b(255, 0, 0); // 蓝色：BGR格式
        }
    }
    _frame_queue.push(_frame);
    // replay_frame_queue.push_replay(_frame, 300);
}

cv::Mat DefaultModule::getFrame()
{
    cv::Mat output;
    _frame_queue.wait_pop(output);
    return output;
}

// cv::Mat DefaultModule::get_replay_Frame()
// {
//     cv::Mat output;
//     replay_frame_queue.wait_pop(output);
//     return output;
// }
