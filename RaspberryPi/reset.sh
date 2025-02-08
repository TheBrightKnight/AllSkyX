#!/bin/bash

echo "Resetting Process";
ps -ef | grep arduino.py | grep -v grep | awk '{print $2}' | xargs kill ;
sleep 3;
ps -ef | grep fans.py | grep -v grep | awk '{print $2}' | xargs kill ;
sleep 3;
cd /home/pi/AllSkyX/RaspberryPi/ ;
/home/pi/AllSkyX/RaspberryPi/arduino.py &> /home/pi/AllSkyX/RaspberryPi/output.log &
/home/pi/AllSkyX/RaspberryPi/fans.py &> /home/pi/AllSkyX/RaspberryPi/fans.log &
echo "Done";
exit 0;

