from machine import Pin, I2C, Timer
from time import sleep
import ssd1306
from rand import get_random_int
import machine
from oled_display import OLEDGraphDisplay
from neopixel import NeoPixel
import network
import usocket
import utime
from neuron import Neuron
from pressure import read_hx711
from echo import read_echo
from connect_config import do_connect
import config


# led = machine.Pin(2, machine.Pin.OUT)
# pin15 = Pin(15, Pin.OUT)   # 设置引脚GPIO0来驱动 NeoPixels
# np = NeoPixel(pin15, 5)   # 在GPIO0上创建一个 NeoPixel对象，包含8个灯珠
# np.fill((0,0,0))


# i2c = I2C(scl=Pin(5), sda=Pin(4))
# i2c = I2C(scl=Pin(12), sda=Pin(14))

# display = OLEDGraphDisplay(scl_pin=5, sda_pin=4, timer=Timer(-1))

# tim = Timer(1)
# tim.init(period=2000, mode=Timer.PERIODIC, callback=lambda t:print(1)) #1次


def send_packet(ip, port):
    # 创建一个socket
    sock = usocket.socket(usocket.AF_INET, usocket.SOCK_DGRAM)

    # 构建要发送的数据
    data = b'Hello, this is a packet!'

    # 发送数据到指定的IP地址和端口
    sock.sendto(data, (ip, port))

    # 关闭socket
    sock.close()

def sender():
    ip, mac_addr = do_connect()
    device_info = config.devices.get(mac_addr)
    print(device_info)
    if(device_info['oled_scl_pin'] == -1):
        display_switch = False
    else:
        oled_dis = OLEDGraphDisplay(scl_pin=device_info["oled_scl_pin"], sda_pin=device_info["oled_sda_pin"])
        oled_dis.display_ip(ip)
        display_switch = True


    freq = 10

    neuron = Neuron(tau=100, dt=100/freq)

    target_ip = '192.168.1.100'  # 目标设备的IP地址
    target_port = 12345  # 目标设备的端口号，确保目标设备在此端口监听
    target_delay = 0.9 # 延时
    target_pin = Pin(15, Pin.OUT) # 灯条引脚
    target_fiber = NeoPixel(target_pin, int(target_delay / (1 / freq)))
    neuron.add_fire_target((target_ip, target_port, int(target_delay / (1 / freq)), target_fiber))

    target_ip = '192.168.1.101'  # 目标设备的IP地址
    target_port = 12345  # 目标设备的端口号，确保目标设备在此端口监听
    target_delay = 0.5 # 延时
    target_pin = Pin(14, Pin.OUT) # 灯条引脚
    target_fiber = NeoPixel(target_pin, int(target_delay / (1 / freq)))
    neuron.add_fire_target((target_ip, target_port, int(target_delay / (1 / freq)), target_fiber))

    hx711_pin_dt = Pin(device_info['hx711_pin_dt'], Pin.IN)  # DT引脚连接到GPIO5
    hx711_pin_sck = Pin(device_info['hx711_pin_sck'], Pin.OUT)  # SCK引脚连接到GPIO4

    echo_trig = Pin(device_info['echo_trig'], Pin.OUT)
    echo_echo = Pin(device_info['echo_echo'], Pin.IN)
    echo_trig.off()
    echo_echo.off()

    while True:
        neuron.check_fire_act()
        neuron.check_potential()

        # receive data
        # ...

        # neuron.decay_potential()

        weight = read_hx711(hx711_pin_dt, hx711_pin_sck)
        dist = read_echo(echo_trig, echo_echo)
        neuron.sensor_fire(weight, dist)
        # neuron.rand_fire()

        if(display_switch):
            oled_dis.update_display(neuron.potential)
        machine.sleep(int(1000/freq))

def receiver():
    ip, mac_addr = do_connect()
    device_info = config.devices.get(mac_addr)
    if(device_info['oled_scl_pin'] == -1):
        display_switch = False
    else:
        oled_dis = OLEDGraphDisplay(scl_pin=device_info["oled_scl_pin"], sda_pin=device_info["oled_sda_pin"])
        oled_dis.display_ip(ip)
        display_switch = True

    freq = 10

    listen_port = 12345  # 监听端口
    neuron = Neuron(tau=100, dt=100 / freq, listen_port=listen_port)

    while True:
        neuron.check_fire_act()
        neuron.check_potential()

        neuron.receive_signal()
        neuron.decay_potential()

        #neuron.rand_fire()

        if(display_switch):
            oled_dis.update_display(neuron.potential, neuron.received, neuron.sent)
        # 等待一段时间后继续下一次循环
        machine.sleep(int(1000/freq))


# sender()
receiver()

