import sys
import cv2
import time

sys.path.append('/home/yu/in/DVS/Camera_4_commerial/wrapperTOpy/build')# 添加模块路径（根据你的实际路径调整）
import DVS # 导入编译的 C++ 模块

# 创建 DVSWrapper 对象
dvs_wrapper = DVS.DVSWrapper(DefautON1="true", RsesON1="false", SetMode1="TIME", TimeSpan1=10000, CountNum1=20000,isRBorGB=True)

# while True:
count = 0
xixi2 = 0
start = time.time()
while count <= 5000:

    # 获取
    count = count + 1
    
    frame = dvs_wrapper.Getframe()

end = time.time()
xixi = end - start

print(xixi/5000)

