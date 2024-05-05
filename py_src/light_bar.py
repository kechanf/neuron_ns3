from machine import Pin, I2C, Timer
from time import sleep
import ssd1306
from rand import get_random_int
import machine
from oled_display import OLEDGraphDisplay
from neopixel import NeoPixel

pin = Pin(15, Pin.OUT)   # 设置引脚GPIO0来驱动 NeoPixels
np = NeoPixel(pin, 8)   # 在GPIO0上创建一个 NeoPixel对象，包含8个灯珠
np[0] = (255, 255, 255) # 设置第一个灯珠显示数据为白色
np[3] = (255, 0, 0)
np.write()              # 写入数据
