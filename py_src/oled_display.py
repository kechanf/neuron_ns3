from machine import Pin, I2C
import ssd1306
from rand import get_random_int


class OLEDGraphDisplay:
    def __init__(self, scl_pin, sda_pin, oled_width=128, oled_height=64):
        self.i2c = I2C(scl=Pin(scl_pin), sda=Pin(sda_pin))
        self.oled = ssd1306.SSD1306_I2C(oled_width, oled_height, self.i2c)
        self.x = 0
        self.old_y = 63
        self.received = False
        self.sent = False
        self.wifi_connected = True
        self.operating_normally = True

    def display_ip(self, ip = None):
        # Second row: IP Address
        self.oled.fill_rect(0, 16, 128, 16, 0)
        self.oled.text("IP:  ..." + ip[8:], 0, 16, 1)
        self.oled.show()

    def get_data(self):
        return get_random_int(0, 30)

    def update_status(self, received=False, sent=False, wifi_connected=True, operating_normally=True):
        self.oled.fill_rect(0, 0, 128, 16, 0)
        status_width = 32
        status_label_width = 16
        self.received = received
        self.sent = sent
        self.wifi_connected = wifi_connected
        self.operating_normally = operating_normally

        def display_status(index, label, state):
            base_x = index * status_width
            self.oled.text(label, base_x + 2, 4, 1)
            color = 1 if state else 0
            self.oled.fill_rect(base_x + status_label_width, 0, status_label_width, 16, color)

        display_status(0, 'r', self.received)
        display_status(1, 'f', self.sent)
        display_status(2, 'w', self.wifi_connected)
        display_status(3, 'o', self.operating_normally)
        self.oled.show()

    def draw_axes(self):
        self.oled.fill_rect(0, 32, 128, 63, 0)
        self.oled.line(0, 63, 127, 63, 1)
        self.oled.line(0, 32, 0, 63, 1)
        self.oled.show()

    def get_status(self, received=False, sent=False, wifi_connected=True, operating_normally=True):
        self.received = received
        self.sent = sent
        self.wifi_connected = wifi_connected
        self.operating_normally = operating_normally

    def update_display(self, y, received=False, sent=False, wifi_connected=True, operating_normally=True):
        # y = 63 - self.get_data()
        p = 0.5
        y = 63 - int(y*63*p)
        self.update_status(received, sent, wifi_connected, operating_normally)

        if self.x < 127 and self.x > 0:
            self.oled.line(self.x, self.old_y, self.x + 1, y, 1)
        else:
            self.draw_axes()
            self.x = 0
            self.oled.line(self.x, 63, self.x, y, 1)

        self.oled.show()
        self.old_y = y
        self.x += 1
