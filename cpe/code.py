"""
import touchio
from board import *

touch1 = touchio.TouchIn(A1)
touch2 = touchio.TouchIn(A7)

# print('Touch thresh ', touch1.threshold)

while True:
    print("%s,%s,%s" % (touch1.raw_value,touch2.raw_value,touch1.raw_value - touch2.raw_value))
"""

# 2
"""
from adafruit_circuitplayground.express import cpx

while True:
    if cpx.touch_A1:
            print('Touched pad A1')
"""
