#!/usr/bin/python
from usb.core import find as finddev
import subprocess
import sys

usbs = subprocess.Popen(('lsusb'), stdout=subprocess.PIPE)
try:
    output = subprocess.check_output(('grep', 'CH340'), stdin=usbs.stdout).decode()
except:
    print("No connected Arduino was found")
    sys.exit(-1)

vp = output.split(' ')[5].split(':')
vendor = "0x"+vp[0]
product = "0x"+vp[1]

print("Reset Vendor: " + vendor + ", Product: " + product)

try: 
    dev = finddev(idVendor=int(vendor,16), idProduct=int(product,16))
    dev.reset()
except:
    print("Failed")
    sys.exit(-1)

print("Success")
sys.exit(0
