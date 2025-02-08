#!/usr/bin/env python3

##################################################################
# Name:             commander.py
# Author:           TheBrightKnight
# Version:          1.0 (Dec 2022)
# Description:      This script connects to arduino.py socket
#                   and can send individual commands to Arduino
#                   
#
##################################################################

################
#   Imports
################
import socket
import signal
import sys
import json

################
#   Globals
################

CONFIG_FILE = 'config.json'
try:
    with open(CONFIG_FILE) as f:
        CONFIG = json.load(f)
except Exception as e:
    print("Cannot open configuration file " + str(CONFIG_FILE) + ": " + str(e))
    sys.exit(1)

HOST = CONFIG["socket_host"]
PORT = CONFIG["socket_port"]

##################################################################

# CTRL+C Handler
def signal_handler(sig, frame):
    close()


##################################################################

# Close socket and exit
def close():
	print("Closing...")
	s.close()
	sys.exit(0)


##################################################################


signal.signal(signal.SIGINT, signal_handler)

try:
	s = socket.socket()
	s.connect((HOST, PORT))
except:
	print("Cannot open socket at " + str(HOST) + ":" + str(PORT))
	sys.exit(1)


while True:
    msg = input("Command To Send: ")
    if msg == "close" or msg == "quit":
       close()
    s.send(msg.encode('utf-8'))
