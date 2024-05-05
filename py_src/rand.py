import uos


def get_random_int(min_val, max_val):
    """生成一个指定范围的随机整数。

    参数:
    min_val (int): 随机数的最小值。
    max_val (int): 随机数的最大值。

    返回:
    int: 在 min_val 和 max_val 之间的随机整数。
    """
    # 生成一个随机字节
    random_byte = uos.urandom(1)

    # 将随机字节转换为整数
    random_number = int.from_bytes(random_byte, 'little')

    # 将随机数映射到指定的范围
    range_size = max_val - min_val + 1
    return min_val + (random_number % range_size)
