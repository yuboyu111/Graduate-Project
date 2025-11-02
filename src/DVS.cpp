#include "../include/DVS.h"
#include <chrono>
using namespace std::chrono;

DVS::DVS()
{
    MODE = TIME;
    TIME_SPAN = 30000;
    COUNT_NUM = 5000;
    BATCH_LEN = 4096;
    RECORD_FLAG.store(false);
    RECEIVE_PAUSE_FLAG.store(false);
    REPLAY_FLAG.store(false);
    RUN_FLAG.store(false);
    PROCESS_FLAG.store(false);
    RECEIVE_FLAG.store(false);
    DECODE_FLAG.store(false);
    Record_ON.store(false);
    IS_REPLAYING_FLAG.store(false);
}

DVS::~DVS()
{
}

void DVS::run()
{
    RUN_FLAG.store(true);
    receive_thread = std::thread(&DVS::receiveData, this);
    decode_thread = std::thread(&DVS::decodeData, this);
    process_thread = std::thread(&DVS::processData, this);
    // record_thread = std::thread(&DVS::record_data, this);
    // record_thread2 = std::thread(&DVS::record_data2, this);

    if (receiver.getFlag())
    {
        std::cout << "receive detach" << std::endl;
        RECEIVE_PAUSE_FLAG.store(true);
        RECEIVE_FLAG.store(true);
        receive_thread.detach();

        std::cout << "decode detach" << std::endl;
        DECODE_FLAG.store(true);
        decode_thread.detach();

        std::cout << "process detach" << std::endl;
        PROCESS_FLAG.store(true);
        process_thread.detach();

        // std::cout << "record_thread detach" << std::endl;
        // record_thread.detach();

        // std::cout << "record_thread2 detach" << std::endl;
        // record_thread2.detach();
    }
}

void DVS::setTimeSpan(int time)
{
    TIME_SPAN = time;
}

void DVS::setCountNum(int count)
{
    COUNT_NUM = count;
}

void DVS::setMode(Mode mode)
{
    MODE = mode;
}

void DVS::setBatchLen(int len)
{
    BATCH_LEN = len;
}

void DVS::set_isRBorGB(bool isRBorGB)
{
    this->is_RBorGB = isRBorGB;
}

void DVS::receiveData()
{
    while (RUN_FLAG.load())
    {
        if (RECEIVE_PAUSE_FLAG.load())
        {
            std::shared_ptr<unsigned char> buffer(new unsigned char[BATCH_LEN]);
            int f = receiver.receiveData(buffer.get(), BATCH_LEN);
            if (f)
                std::cout << "Time out" << std::endl;
            data_queue.push(buffer);
        }
        else
            std::this_thread::yield();
    }

    RECEIVE_FLAG.store(false);
}

void DVS::decodeData()
{
    unsigned int timestamp = 0, cursor = 0;
    std::vector<Event> packet;
    std::vector<Event> packet2;
    std::vector<Event> packet3;
    std::shared_ptr<unsigned char> data;

    time_check = std::chrono::high_resolution_clock::now();

    while (RUN_FLAG.load())
    {
        data_queue.wait_pop(data);

        if (RECORD_FLAG.load())
            file_out.write((char *)(data.get()), BATCH_LEN);

        if (MODE == COUNT)
        {
            while (cursor < BATCH_LEN)
            {
                if (decoder.decodeDataCount(data.get(), packet, timestamp, count, COUNT_NUM, time_cycle, cursor, BATCH_LEN))
                {
                    packet2 = packet;

                    packet_queue.push(packet);
                    packet.clear();
                    count = 0;

                    auto first_check = std::chrono::high_resolution_clock::now();
                    double elapsed = std::chrono::duration<double>(first_check - time_check).count();
                    time_check = first_check;
                    double event_rate = COUNT_NUM / (elapsed * 1000000);
                    // std::cout << "event_rate=" << event_rate << std::endl;

                    packet3 = packet2;
                    if (event_rate > THRESHOLD)
                        record_queue.push2(packet2);

                    replay_queue.push_replay(packet3, 300);

                    packet2.clear();
                    packet3.clear();
                }
            }
            cursor = 0;
        }
        else if (MODE == TIME)
        {
            while (cursor < BATCH_LEN)
            {

                if (decoder.decodeDataTime(data.get(), packet, timestamp, timestamp_prev_raw, timestamp_prev, timestamp_prev + TIME_SPAN, time_cycle, cursor, BATCH_LEN))
                {

                    packet2 = packet;
                    unsigned int count_check = packet.size();
                    packet_queue.push(packet);
                    packet.clear();
                    timestamp_prev = timestamp_prev + TIME_SPAN;
                    count = 0;

                    double event_rate = static_cast<double>(count_check) / TIME_SPAN;
                    // std::cout << "event_rate=" << event_rate << std::endl;

                    packet3 = packet2;

                    if (event_rate > THRESHOLD)
                        record_queue.push2(packet2);

                    replay_queue.push_replay(packet3, 300);
                    packet2.clear();
                    packet3.clear();
                }
            }
            cursor = 0;
        }
    }
    DECODE_FLAG.store(false);
}

void DVS::processData()
{
    std::vector<Event> packet;
    while (RUN_FLAG.load())
    {
        packet_queue.wait_pop(packet);
        for (auto &module : module_list)
        {
            module.get()->convertEvent(packet, is_RBorGB);
        }
    }
    PROCESS_FLAG.store(false);
}

cv::Mat DVS::getFrame(int index)
{
    return module_list[index]->getFrame();
}

// cv::Mat DVS::get_replay_Frame(int index)
// {
//     return module_list[index]->get_replay_Frame();
// }

void DVS::startRecord(std::string name)
{
    if (!RECORD_FLAG.load())
    {
        file_out.open(name, std::ios::out | std::ios::binary);
        if (file_out.is_open())
            RECORD_FLAG.store(true);
    }
}

void DVS::stopRecord()
{
    if (RECORD_FLAG.load())
    {
        RECORD_FLAG.store(false);
        if (file_out.is_open())
            file_out.close();
    }
}

void DVS::pause()
{
    RECEIVE_PAUSE_FLAG.store(false);
    data_queue.clear();
    packet_queue.clear();
}

void DVS::restart()
{
    if (REPLAY_FLAG.load())
        stopReplay();
    data_queue.clear();
    packet_queue.clear();
    if (receiver.getFlag())
        RECEIVE_PAUSE_FLAG.store(true);
}

void DVS::stop()
{

    stopRecord();
    stopReplay();

    RUN_FLAG.store(false);
    while (RECEIVE_FLAG.load() || DECODE_FLAG.load() || PROCESS_FLAG.load())
    {
    }
    data_queue.clear();
    packet_queue.clear();
    module_list.clear();
}

void DVS::startReplay(std::string name)
{
    REPLAY_FLAG.store(true);
    replay_thread = std::thread(&DVS::replay, this, name);
    replay_thread.detach();
}

void DVS::replay(std::string name)
{
    unsigned int count = 0, timestamp = 0, timestamp_prev = 0, cursor = 0, time_cycle = 0;
    std::vector<Event> packet;
    unsigned char *data;
    file_in.open(name, std::ios::in | std::ios::binary);
    if (file_in.is_open())
    {

        while (REPLAY_FLAG.load() && !file_in.eof())
        {
            data = new unsigned char[BATCH_LEN];
            file_in.read((char *)data, BATCH_LEN);
            if (MODE == COUNT)
            {
                while (cursor < BATCH_LEN)
                {
                    if (decoder.decodeDataCount(data, packet, timestamp, count, COUNT_NUM, time_cycle, cursor, BATCH_LEN))
                    {
                        packet_queue.push(packet);
                        packet = std::vector<Event>();
                        count = 0;
                    }
                }
                cursor = 0;
            }
            else if (MODE == TIME)
            {
                while (cursor < BATCH_LEN)
                {
                    if (decoder.decodeDataTime(data, packet, timestamp, timestamp_prev_raw, timestamp_prev, timestamp_prev + TIME_SPAN, time_cycle, cursor, BATCH_LEN))
                    {
                        packet_queue.push(packet);
                        packet = std::vector<Event>();
                        timestamp_prev = timestamp_prev + TIME_SPAN;
                        count = 0;
                    }
                }
                cursor = 0;
            }
            delete[] data;
            data = nullptr;
        }
        file_in.close();
    }
}

void DVS::replayPastFrames(bool isBlack)
{
    // 如果已经在回放，直接返回
    if (IS_REPLAYING_FLAG.load())
        return;

    IS_REPLAYING_FLAG.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 启动独立线程回放
    std::thread([this, isBlack]() {
        std::vector<Event> packet;
        std::vector<std::vector<Event>> all_packets;

        // 一次性取出队列里所有帧
        while (replay_queue.try_pop(packet)) // 非阻塞取出
        {
            if (!packet.empty())
                all_packets.push_back(packet);
        }

        if (all_packets.empty())
        {
            IS_REPLAYING_FLAG.store(false);
            return; // 队列为空，直接退出
        }

        // 创建显示窗口
        cv::namedWindow("Replay", cv::WINDOW_NORMAL);

        for (auto &pkt : all_packets)
        {
            cv::Mat _frame;

            if (isBlack)
            // 黑底色
            {
                _frame = cv::Mat::zeros(480, 640, CV_8UC3);
                // 绘制事件
                for (auto &event : pkt)
                {
                    _frame.ptr<cv::Vec3b>(event.y)[event.x][event.p] = 255;
                }
            }
            else
            // 白底色
            {
                _frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));
                for (auto &event : pkt)
                {
                    cv::Vec3b &pixel = _frame.ptr<cv::Vec3b>(event.y)[event.x];
                    // 事件极性为 true -> 红色；false -> 蓝色
                    if (event.p)
                        pixel = cv::Vec3b(0, 0, 255); // 红色：BGR格式
                    else
                        pixel = cv::Vec3b(255, 0, 0); // 蓝色：BGR格式
                }
            }

            // 显示慢放
            cv::imshow("Replay", _frame);
            // cv::resizeWindow("Replay", 960, 720);
            cv::moveWindow("Replay", 1100, 280); // 避开实时窗口

            cv::waitKey(50); // 设置慢放程度
        }

        cv::destroyWindow("Replay");
        IS_REPLAYING_FLAG.store(false); // 回放结束
    })
        .detach();
}

void DVS::stopReplay()
{
    REPLAY_FLAG.store(false);
}
bool DVS::isConnected()
{
    return receiver.getFlag();
}

void DVS::addModule(std::string name)
{
    std::shared_ptr<Module> m(nullptr);
    if (name == "default")
        m = std::shared_ptr<Module>(new DefaultModule(HEIGHT, WIDTH));
    else if (name == "RSES")
        m = std::shared_ptr<Module>(new RSESModule(HEIGHT, WIDTH));
    if (m.get())
        module_list.push_back(m);
}

void DVS::log()
{
    while (RUN_FLAG.load())
    {

        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "bin size: " << data_queue.size() << " packet size: " << packet_queue.size();
        for (auto &module : module_list)
        {
            std::cout << " frame size: " << module.get()->getSize();
        }
        std::cout << std::endl;
        std::cout << timestamp_prev << " " << timestamp_prev_raw << " " << time_cycle << " " << count << std::endl;
    }
}

void DVS::TakePhoto(cv::Mat frame_1, cv::Mat frame_2)
{
    if (DEFAUT_PIC_FLAG.load())
    {
        static int frame_count_defaut = 1;
        time_t raw_time;             // 定义时间变量
        struct tm *info;             // 使用指针保存本地时间结构
        time(&raw_time);             // 获取当前时间
        info = localtime(&raw_time); // 获取本地时间
        std::string filename = "../save/save_default/defaut_frame_" +
                               std::to_string(info->tm_year + 1900) + "_" +
                               std::to_string(info->tm_mon + 1) + "_" +
                               std::to_string(info->tm_mday) + "_" +
                               std::to_string(info->tm_hour) + "_" +
                               std::to_string(info->tm_min) + "_" +
                               std::to_string(info->tm_sec) + "__" +
                               std::to_string(frame_count_defaut) + ".png";
        frame_count_defaut++;
        cv::imwrite(filename, frame_1);
    }

    if (RSES_PIC_FLAG.load())
    {
        static int frame_count_rses = 1;
        // std::string filename = "../save/save_rses/rses_frame_" + std::to_string(frame_count_rses++) + ".png";
        time_t raw_time;             // 定义时间变量
        struct tm *info;             // 使用指针保存本地时间结构
        time(&raw_time);             // 获取当前时间
        info = localtime(&raw_time); // 获取本地时间
        std::string filename = "../save/save_rses/rses_frame_" +
                               std::to_string(info->tm_year + 1900) + "_" +
                               std::to_string(info->tm_mon + 1) + "_" +
                               std::to_string(info->tm_mday) + "_" +
                               std::to_string(info->tm_hour) + "_" +
                               std::to_string(info->tm_min) + "_" +
                               std::to_string(info->tm_sec) + ".png";
        cv::imwrite(filename, frame_2);
    }
}

void DVS::Get_Choice(const std::string &str1, const std::string &str2, bool &defaut_pic_flag, bool &rses_pic_flag)
{
    DEFAUT_PIC_FLAG = (str1 == "true" || str1 == "1");
    defaut_pic_flag = DEFAUT_PIC_FLAG;

    RSES_PIC_FLAG = (str2 == "true" || str2 == "1");
    rses_pic_flag = RSES_PIC_FLAG;
}

// 每隔一段时间记录.dat文件
void DVS::recording_timing()
{
    while (RUN_FLAG.load())
    {
        time_t raw_time;             // 定义时间变量
        struct tm *info;             // 使用指针保存本地时间结构
        time(&raw_time);             // 获取当前时间
        info = localtime(&raw_time); // 获取本地时间
        std::string name = "../save_dat/regular/frame_" +
                           std::to_string(info->tm_year + 1900) + "_" +
                           std::to_string(info->tm_mon + 1) + "_" +
                           std::to_string(info->tm_mday) + "_" +
                           std::to_string(info->tm_hour) + "_" +
                           std::to_string(info->tm_min) + "_" +
                           std::to_string(info->tm_sec) + ".dat";
        startRecord(name);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        stopRecord();
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

// 控制成像的旋转与缩放
cv::Mat DVS::Do_Rotation(cv::Mat frame, double angle, double ratio)
{
    int h = frame.rows;
    int w = frame.cols;
    // printf("h = %d  w = %d\n", h, w);
    cv::Point2f center(w / 2.0f, h / 2.0f);
    double scale = 1.0;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, scale);
    cv::Mat rotatedImage;
    cv::warpAffine(frame, rotatedImage, rotationMatrix, frame.size());

    cv::Mat resizedImage;
    cv::Size targetSize(ratio * w, ratio * h);
    cv::resize(rotatedImage, resizedImage, targetSize);

    return resizedImage;
}

void DVS::record_data()
{
    while (RUN_FLAG.load())
    {
        if (Record_ON.load() == true)
        {
            TakePhoto(Do_Rotation(getFrame(0), 0, 1.0), getFrame(1));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            Record_ON.store(false);
        }
    }
}

void DVS::record_data2()
{
    std::vector<Event> packet;
    while (RUN_FLAG.load())
    {
        record_queue.wait_pop(packet);

        cv::Mat _frame = cv::Mat::zeros(480, 640, CV_8UC3);
        for (auto &event : packet)
        {
            _frame.ptr<cv::Vec3b>(event.y)[event.x][event.p] = 255;
        }

        static int frame_count_default = 1;
        static int foldernum = 0;
        std::string foldername;

        time_t raw_time;             // 定义时间变量
        struct tm *info;             // 使用指针保存本地时间结构
        time(&raw_time);             // 获取当前时间
        info = localtime(&raw_time); // 获取本地时间

        if (frame_count_default > 500 * foldernum)
        {
            foldernum++;
            foldername = "../save/save_default_" + std::to_string(foldernum);
            std::filesystem::create_directory(foldername);
        }

        else
        {
            foldername = "../save/save_default_" + std::to_string(foldernum);
        }

        std::string filename = foldername + "/defaut_frame_" +
                               std::to_string(info->tm_year + 1900) + "_" +
                               std::to_string(info->tm_mon + 1) + "_" +
                               std::to_string(info->tm_mday) + "_" +
                               std::to_string(info->tm_hour) + "_" +
                               std::to_string(info->tm_min) + "_" +
                               std::to_string(info->tm_sec) + "__" +
                               std::to_string(frame_count_default) + ".png";
        frame_count_default++;

        cv::imwrite(filename, _frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
