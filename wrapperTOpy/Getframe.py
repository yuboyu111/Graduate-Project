import sys
import cv2
import time

sys.path.append('/home/yu/in/DVS/Camera_4_commerial/wrapperTOpy/build')# 添加模块路径（根据你的实际路径调整）
import DVS # 导入编译的 C++ 模块

# 创建 DVSWrapper 对象
dvs_wrapper = DVS.DVSWrapper(DefautON1="true", RsesON1="false", SetMode1="TIME", TimeSpan1=2000, CountNum1=20000,isRBorGB=True)

# while True:
while True:
    try:
        # 获取 
        frame = dvs_wrapper.Getframe()
        # 检查帧的数据类型和形状
        # print(f"Frame shape: {frame.shape}, dtype: {frame.dtype}")

        # 如果帧是二维的（灰度图），则转为三通道展示
        if frame.ndim == 2:
            frame = cv2.cvtColor(frame, cv2.COLOR_GRAY2BGR)
        elif frame.shape[2] != 3:
            raise ValueError("Unexpected number of channels in frame.")

        # 显示图像
        cv2.imshow("Frame", frame)

        # 按下 q 键退出
        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

    except Exception as e:
        print("Error:", e)
        break

cv2.destroyAllWindows()
