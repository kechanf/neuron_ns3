# import numpy as np
# import ulab as np
import math
from rand import get_random_int
import usocket
import machine

class Neuron:
    def __init__(self, tau=100, dt=100, threshold=1.0, listen_port=12345):
        """
        初始化神经元

        参数:
        tau (float): 时间常数，控制电位衰减的速率。
        threshold (float): 触发动作电位的阈值。
        """
        self.potential = 0.0  # 神经元当前的电位
        self.tau = tau
        self.dt = dt
        self.threshold = threshold
        self.fire_target = [] # [(target_ip, target_port, delay_time, target_fiber)]
        self.fire_act_stack = [] # 存储发放动作电位的时间点和地址
        self.status_led = machine.Pin(2, machine.Pin.OUT)
        self.listen_port = listen_port

        self.sent = False
        self.received = False

        self.sock = usocket.socket(usocket.AF_INET, usocket.SOCK_DGRAM)
        # 绑定端口，选择一个端口号，确保与发送方端口号一致
        self.sock.bind(('0.0.0.0', listen_port))  # 使用'0.0.0.0'表示监听所有可用网络接口

    def check_fire_act(self):
        current_fire_act = []
        for act in self.fire_act_stack:
            target_fiber = act[3]
            target_fiber.fill((0, 0, 0))
            if act[2] <= 0:
                send_packet(act[0], act[1])
            else:
                current_fire_act.append((act[0], act[1], act[2] - 1, target_fiber))
                target_fiber[len(target_fiber) - act[2]] = (10,10,10)
            target_fiber.write()
        self.fire_act_stack = current_fire_act

    def add_fire_target(self, target):
        self.fire_target.append(target)
        target[3].fill((0, 0, 0))
        target[3].write()

    def receive_signal(self):
        """
        处理从指定输入源接收到的信号

        参数:
        signal_strength (float): 接收到的信号强度
        """
        # 清空缓冲区或设置一个空数据标记
        data = None
        self.received = False
        try:
            # 设置非阻塞模式
            self.sock.setblocking(False)
            data, addr = self.sock.recvfrom(1024)  # 1024是缓冲区大小
            # print("Received message:", data, "from", addr)
        except OSError as e:
            # 如果没有数据到达，OSError会被抛出
            pass

        if data:
            signal_strength = 0.3  # 假设接收到的信号强度为 0.2
            self.potential += signal_strength
            self.received = True

    def decay_potential(self):
        """
        电位随时间进行指数衰减

        参数:
        dt (float): 时间步长，表示从上一次更新以来经过的时间
        """
        self.potential = self.potential * math.exp(-self.dt / self.tau)

    def check_potential(self):
        """
        检查电位是否达到或超过阈值
        """
        self.status_led.value(1)
        self.sent = False
        if self.potential >= self.threshold:
            self.fire()
            self.potential = 0

    def fire(self):
        """
        发放动作电位，并重置电位
        """
        # print("Action potential fired! Transmitting signal...")
        self.transmit_signal()
        self.status_led.value(0)
        self.sent = True

    def transmit_signal(self):
        """
        模拟传递信号到其他神经元的行为（可以根据实际需要进行实现）
        """
        # print("Signal transmitted to other neurons.")
        for target in self.fire_target:
            # print("Transmitting to ", target)
            self.fire_act_stack.append(target)

    def rand_fire(self):
        if(get_random_int(0, 100) < 5):
            self.fire()

def send_packet(ip, port):
    # 创建一个socket
    sock = usocket.socket(usocket.AF_INET, usocket.SOCK_DGRAM)

    # 构建要发送的数据
    data = b'Hello, this is a packet!'

    # 发送数据到指定的IP地址和端口
    sock.sendto(data, (ip, port))

    # 关闭socket
    sock.close()


# 示例用法
# neuron = Neuron(tau=100)  # 设定时间常数为 100 毫秒
#
# # 模拟从两个不同的神经元接收信号
# neuron.receive_signal(0.5, 'neuron1')
# neuron.decay_potential(dt=1)  # 假设时间步长为 1 毫秒
# neuron.receive_signal(0.4, 'neuron2')
# neuron.decay_potential(dt=1)
