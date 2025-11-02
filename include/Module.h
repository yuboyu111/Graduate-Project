#ifndef MODULE_H
#define MODULE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include "Event.h"
#include "DataQueue.h"

class Module
{
private:
public:
    virtual void convertEvent(std::vector<Event> &packet, bool is_RBorGB) = 0;
    virtual cv::Mat getFrame() = 0;
    // virtual cv::Mat get_replay_Frame() = 0;
    virtual int getSize() = 0;
    // virtual DataQueue<cv::Mat> &getReplayFrameQueue() = 0;
    virtual ~Module() {}

};

// module list
typedef std::vector<std::shared_ptr<Module>> ModuleList;
// default
class DefaultModule : virtual public Module
{
private:
    cv::Mat _frame;
    DataQueue<cv::Mat> _frame_queue;
    DataQueue<cv::Mat> replay_frame_queue;
    int _width;
    int _height;

public:
    DefaultModule(int height, int width);
    ~DefaultModule();
    void convertEvent(std::vector<Event> &packet, bool is_RBorGB) override;
    cv::Mat getFrame();
    // cv::Mat get_replay_Frame();
    int getSize() { return _frame_queue.size(); }
    // DataQueue<cv::Mat> &getReplayFrameQueue() override { return replay_frame_queue; }
};
// RSES
class RSESModule : virtual public Module
{
private:
    cv::Mat _frame;
    DataQueue<cv::Mat> _frame_queue;
    DataQueue<cv::Mat> replay_frame_queue;
    int _batch;
    float _thre;
    float _rate;
    int _down_sample;
    cv::Mat _kernel_1;
    cv::Mat _kernel_2;
    cv::Mat _event_buffer;
    cv::Mat _event_buffer_temp;
    int _width;
    int _height;

public:
    RSESModule(int height, int width, float thre = 1.5, int down_sample = 2, int batch = 2000);
    ~RSESModule();
    void convertEvent(std::vector<Event> &packet, bool is_RBorGB) override;
    cv::Mat get_replay_Frame();
    cv::Mat getFrame();
    int getSize() { return _frame_queue.size(); }
    // DataQueue<cv::Mat> &getReplayFrameQueue() override { return replay_frame_queue; }
};
#endif