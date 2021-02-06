import ctypes
import pathlib
import time
import random

if __name__ == "__main__":
    # Load the shared library into ctypes
    libname = pathlib.Path().absolute() / "go2wrapper.so"
    c_lib = ctypes.CDLL(libname)

'''
 todo

 - ctypes - structure for analog
 - makefile
 - python module
 - ros bridge

 https://gist.github.com/hidsh/7065820

'''

def makeColor(red, green, blue):
    return ((int(red / 255 * 31) << 11) | (int(green / 255 * 63) << 5) | (int(blue / 255 * 31)))


def c_str(string):
        return ctypes.c_char_p(string.encode('utf-8')) 

resWidth = 854
resHeight = 480

SMOOTHING_FACTOR = 0.99
MAX_FPS = 10000
avg_fps = -1
last_tick = time.time()

c_lib.initDevice()

while True:
    buttons = c_lib.readButtons()

    c_lib.fillColor(0)
    c_lib.drawLine(10, 10, resWidth-10, resHeight-10, makeColor(255,0,0))
    c_lib.drawText(10,450,c_str("FPS: " + str(int(avg_fps)) + ", Buttons: " + str(buttons) + ", Battery: " + str(c_lib.readBatteryLevel()) ), makeColor(255,255,255))

    c_lib.flipScreen()

    current_tick = time.time()
    # Ensure we don't get crazy large frame rates, by capping to MAX_FPS
    current_fps = 1.0 / max(current_tick - last_tick, 1.0/MAX_FPS)
    last_tick = current_tick
    if avg_fps < 0:
        avg_fps = current_fps
    else:
        avg_fps = (avg_fps * SMOOTHING_FACTOR) + (current_fps * (1-SMOOTHING_FACTOR))


c_lib.destroyDevice()