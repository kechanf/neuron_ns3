import json
import network
import ubinascii


devices = {
    "08:f9:e0:66:d6:23": {
        "ip_address": "192.168.1.100",
        "device_name": "Device1",
        "listen_port": 12345,
        "available_pins": [],
        "oled_scl_pin": 5,
        "oled_sda_pin": 4,
        "hx711_pin_dt": -1,
        "hx711_pin_sck": -1,
        "echo_trig": -1,
        "echo_echo": -1,
    },
    "a4:cf:12:fd:19:5a": {
        "ip_address": "192.168.1.101",
        "device_name": "Device2",
        "listen_port": 12345,
        "available_pins": [],
        "oled_scl_pin": 12,
        "oled_sda_pin": 14,
        "hx711_pin_dt": -1,
        "hx711_pin_sck": -1,
        "echo_trig": -1,
        "echo_echo": -1,
    },
    "08:f9:e0:66:d6:25": {
        "ip_address": "192.168.1.102",
        "device_name": "Device3",
        "listen_port": 12345,
        "available_pins": [],
        "oled_scl_pin": 12,
        "oled_sda_pin": 14,
        "hx711_pin_dt": 0,
        "hx711_pin_sck": 2,
        "echo_trig": 13,
        "echo_echo": 12,
    },
}

