#include "../include/Module.h"
#include "../include/DataQueue.h"

RSESModule::RSESModule(int height, int width, float thre, int down_sample, int batch) : _height(height), _width(width), _thre(thre), _down_sample(down_sample), _batch(batch), _rate(0.0)
{
    float *k1 = new float[9]{0, -1, 0, 0, 1, 0, 0, -1, 0};
    float *k2 = new float[9]{0, 0, 0, -1, 1, -1, 0, 0, 0};
    _kernel_1 = cv::Mat(3, 3, CV_32FC1, k1);
    _kernel_2 = cv::Mat(3, 3, CV_32FC1, k2);
    _event_buffer = cv::Mat::zeros(_height / _down_sample, _width / _down_sample, CV_32FC1);
    _event_buffer_temp = cv::Mat::zeros(_height / _down_sample, _width / _down_sample, CV_32FC1);
}

RSESModule::~RSESModule()
{
}

void RSESModule::convertEvent(std::vector<Event> &packet, bool is_RBorGB)
{
    _frame = cv::Mat::zeros(_height, _width, CV_8UC3);
    cv::Mat map_filter1;
    cv::Mat map_filter2;
    cv::Mat mask;
    cv::Mat map_gt0;
    int count = 0;
    for (auto &event : packet)
    {
        _frame.ptr<cv::Vec3b>(event.y)[event.x][event.p] = 255;
        _event_buffer_temp.ptr<float>(event.y / _down_sample)[event.x / _down_sample] = (event.t - packet[0].t) / 30000.0;
        ++count;
        if (count % _batch == 0)
        {
            cv::add(_event_buffer, _event_buffer_temp, _event_buffer);
            cv::filter2D(_event_buffer, map_filter1, -1, _kernel_1);
            cv::filter2D(_event_buffer, map_filter2, -1, _kernel_2);
            mask = (map_filter1 + map_filter2) > 0;
            map_gt0 = (_event_buffer > 0);
            _rate = cv::countNonZero(map_gt0) / (cv::countNonZero(mask) + 1e-6);
            // std::cout<<rate<<std::endl;
            if (_rate > _thre)
            {
                _event_buffer = cv::Mat::zeros(_height / _down_sample, _width / _down_sample, CV_32FC1);
                _event_buffer_temp = cv::Mat::zeros(_height / _down_sample, _width / _down_sample, CV_32FC1);
                break;
            }
            _event_buffer_temp = cv::Mat::zeros(_height / _down_sample, _width / _down_sample, CV_32FC1);
        }
    }
    _frame_queue.push(_frame);
    // replay_frame_queue.push_replay(_frame, 300);
}

cv::Mat RSESModule::getFrame()
{
    cv::Mat output;
    _frame_queue.wait_pop(output);
    return output;
}

// cv::Mat RSESModule::get_replay_Frame()
// {
//     cv::Mat output;
//     replay_frame_queue.wait_pop(output);
//     return output;
// }