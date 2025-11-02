#include "pic.hpp"

DVSWrapper::DVSWrapper(std::string DefautON1 = "true", std::string RsesON1 = "false", std::string SetMode1 = "TIME", int TimeSpan1 = 15000, int CountNum1 = 20000,bool isRBorGB = "true")
{
    const std::string DefautON = DefautON1;
    const std::string RsesON = RsesON1;
    const std::string SetMode = SetMode1;
    const unsigned int TimeSpan = TimeSpan1;
    const unsigned int CountNum = CountNum1;

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
        dvs.run(); // 启动后台线程
    }
    else
    {
        std::cout << "no connection" << std::endl;
    }
}

DVSWrapper::~DVSWrapper() {};

// 把 cv::Mat 转换成 numpy array 的工具函数
pybind11::array DVSWrapper::mat_to_numpy(const cv::Mat &mat)
{
    if (mat.empty())
    {
        throw std::runtime_error("Received empty cv::Mat");
    }

    // 只处理 CV_8UC1 和 CV_8UC3 类型，你可以按需添加其他格式支持
    if (mat.type() == CV_8UC1)
    {
        return pybind11::array_t<uint8_t>({mat.rows, mat.cols}, mat.data);
    }
    else if (mat.type() == CV_8UC3)
    {
        return pybind11::array_t<uint8_t>({mat.rows, mat.cols, 3}, mat.data);
    }
    else
    {
        throw std::runtime_error("Unsupported cv::Mat type in mat_to_numpy");
    }
}

cv::Mat DVSWrapper::Getframe()
{
    cv::Mat frame_1, frame_2;

    if (defaut_pic_flag == rses_pic_flag)
    {
        return cv::Mat();
    }
    if (defaut_pic_flag)
    {
        frame_1 = dvs.getFrame(0); // 获取帧
    }
    if (rses_pic_flag)
    {
        frame_2 = dvs.getFrame(1); // 获取帧
    }

    return frame_1;
}

// 你原来的 Getframe 函数是 cv::Mat，我们包装一层转 numpy
pybind11::array DVSWrapper::wrapped_Getframe()
{
    cv::Mat mat = Getframe();
    return mat_to_numpy(mat); // 返回
}
