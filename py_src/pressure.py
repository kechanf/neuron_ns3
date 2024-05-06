from machine import Pin
import time

# 定义连接到HX711的引脚
# pin_dt = Pin(0, Pin.IN)  # DT引脚连接到GPIO5
# pin_sck = Pin(2, Pin.OUT)  # SCK引脚连接到GPIO4

def read_hx711(pin_dt, pin_sck):
    count = 0
    pin_sck.value(0)  # 确保SCK初始为低电平

    # 等待DT引脚低电平，表示数据准备就绪
    while pin_dt.value():
        pass

    # 读取24位数据 + 1位增益和通道位
    for i in range(24):
        pin_sck.value(1)
        count = count << 1  # 左移一个位
        pin_sck.value(0)
        if pin_dt.value():
            count += 1

    # 增益设置（可根据实际使用进行调整）
    # 下面的3个时钟脉冲用于设置增益和通道
    for i in range(1):  # 这里例子设置为增益128，对应1个脉冲
        pin_sck.value(1)
        pin_sck.value(0)

    count ^= 0x800000  # 转换成32位有符号数
    zero_count = 7954600
    return (count - zero_count) / (8040200 - zero_count) * 0.2

# while True:
#     weight = read_hx711()
#     print("Weight data:", weight)
#     time.sleep(0.5)  # 每0.5秒读取一次
