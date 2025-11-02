#ifndef DVS_H
#define DVS_H

#include <thread>
#include <opencv2/opencv.hpp>
#include <vector>
#include <atomic>
#include <fstream>
#include <memory>
#include <iostream>
#include <filesystem>
#include "Settings.h"
#include "Event.h"
#include "Module.h"
#include "DataReceiver.h"
#include "DataDecoder.h"
#include "DataQueue.h"


class DVS
{
public:
    enum Mode
    {
        TIME,
        COUNT
    };

private:
    Mode MODE;
    unsigned int TIME_SPAN;
    unsigned int COUNT_NUM;
    unsigned int BATCH_LEN;

    std::atomic_bool RECORD_FLAG;
    std::atomic_bool RECEIVE_PAUSE_FLAG;
    std::atomic_bool REPLAY_FLAG;
    std::atomic_bool RUN_FLAG;
    std::atomic_bool RECEIVE_FLAG;
    std::atomic_bool DECODE_FLAG;
    std::atomic_bool PROCESS_FLAG;
    std::atomic_bool DEFAUT_PIC_FLAG;
    std::atomic_bool RSES_PIC_FLAG;
    std::atomic_bool IS_REPLAYING_FLAG;

    bool is_RBorGB;

    unsigned int timestamp_prev = 0;
    unsigned int timestamp_prev_raw = 0;
    unsigned int count = 0;
    unsigned int time_cycle = 0;

    DataDecoder decoder;
    DataReceiver receiver;
    DataQueue<std::shared_ptr<unsigned char>> data_queue;
    DataQueue<std::vector<Event>> packet_queue;
    DataQueue<std::vector<Event>> record_queue;
    DataQueue<std::vector<Event>> replay_queue;

    DataQueue<cv::Mat> _frame_queue2;

    std::thread receive_thread;
    std::thread decode_thread;
    std::thread process_thread;
    std::thread replay_thread;
    std::thread record_thread;
    std::thread record_thread2;

    std::ofstream file_out;
    std::ifstream file_in;

    ModuleList module_list;

    std::chrono::time_point<std::chrono::high_resolution_clock> time_check;
    const double THRESHOLD = 2.0; // 事件率阈值
    std::atomic_bool Record_ON;

public:
    DVS();
    ~DVS();
    void setMode(Mode);
    void setTimeSpan(int);
    void setCountNum(int);
    void setBatchLen(int);
    void set_isRBorGB(bool isRBorGB);
    cv::Mat getFrame(int index);
    // cv::Mat get_replay_Frame(int index);
    void receiveData();
    void decodeData();
    void processData();
    void run();
    void startRecord(std::string name);
    void stopRecord();
    bool isConnected();
    void pause();
    void stop();
    void restart();
    void replay(std::string name);
    void replayPastFrames(bool isBlack);
    void startReplay(std::string name);
    void stopReplay();
    void addModule(std::string module_name);
    void log();
    void TakePhoto(cv::Mat frame_1, cv::Mat frame_2);
    void Get_Choice(const std::string &str1, const std::string &str2, bool &defaut_pic_flag, bool &rses_pic_flag);
    void recording_timing();
    cv::Mat Do_Rotation(cv::Mat frame, double angle, double ratio);
    void record_data();
    void record_data2();
};

#endif
