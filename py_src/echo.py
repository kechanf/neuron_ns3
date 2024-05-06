from machine import Pin
import time
# HC-SR04超声波模块测距原理是：给模块1个最少10us的高电平，模块接受到高电平后开始发射8个40KHz的声波，
# echo脚会由0变为1,MCU开始计时，当超声波模块接收到返回的声波时，echo由1变为0,MCU停止计时，
# 这时间差就是测距总时间，在乘声音的传播速度340米/秒，除2就是距离。

#定义IO口模式，以及初始状态
# trig = Pin(13,Pin.OUT)
# echo = Pin(12, Pin.IN)
# trig.off()
# echo.off()
#构建函数
def read_echo(trig, echo):
  #触发HC-SR04超声波模块测距
  trig.on()
  time.sleep_us(10)
  trig.off()
  #检测回响信号，为低时，测距完成
  while(echo.value() == 0):
    pass
  #开始不断递增的微秒计数器 1
    t1 = time.ticks_us()
  #检测回响信号，为高时，测距开始
  while(echo.value() == 1):
    pass
  #开始不断递增的微秒计数器 2
    t2 = time.ticks_us()
  #计算两次调用 ticks_ms(), ticks_us(), 或 ticks_cpu()之间的时间，这里是ticks_us()
  t3 = time.ticks_diff(t2,t1)/10000
  #返回一个值给调用方，不带表达式的return相当于返回 None。
  #这里返回的是：开始测距的时间减测距完成的时间*声音的速度/2（来回）
  return t3*340/2

# try:
#   while 1:
#     print("JuLiWei:%0.2f cm" %fasong())
#     time.sleep(1)
# except KeyboardInterrupt:
#   pass
