import network
import config

def get_mac_address():
    wlan = network.WLAN(network.STA_IF)  # 创建一个 station 接口
    wlan.active(True)  # 激活接口
    mac = wlan.config('mac')  # 获取MAC地址
    mac_address = ':'.join(['%02x' % b for b in mac])  # 格式化MAC地址
    return mac_address

def do_connect(wifi_essid = 'brain_sys', wifi_password = 'penglabpenglab'):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect(wifi_essid, wifi_password)
        while not wlan.isconnected():
            pass

    mac = wlan.config('mac')  # 获取MAC地址
    mac_address = ':'.join(['%02x' % b for b in mac])  # 格式化MAC地址

    print(mac_address)
    device_info = config.devices.get(mac_address)

    # 设置固定的IP地址
    wlan.ifconfig((device_info['ip_address'], '255.255.255.0', '192.168.1.1', '192.168.1.1'))
    print('network config:', wlan.ifconfig())

    return wlan.ifconfig()[0], mac_address


