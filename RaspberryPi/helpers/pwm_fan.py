#!/usr/bin/python

##################################################################
# Name:             pwn_fan.py
# Author:           TheBrightKnight
# Version:          1.0 (Dec 2022)
# Description:      Test your PWM fan by selecting speeds 
#                   
#
##################################################################


################
#   Imports
################
import RPi.GPIO as IO 
import subprocess
import signal
import sys

# CTRL+C Handler
def signal_handler(sig, frame):
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
IO.setwarnings(False)
# GPIO Select
GPIO_NUM = int(input("Which GPIO do you want to use? "))
IO.setmode (IO.BCM)
IO.setup(GPIO_NUM,IO.OUT) 
# Set GPIO as a PWM output, with 100Hz frequency 
# (this should match your fans specified PWM frequency)
fan = IO.PWM(GPIO_NUM,100)
fan.start(100)


while True:
    command = input("Select Speed:")
    fan.ChangeDutyCycle(int(command))


sys.exit(0)
