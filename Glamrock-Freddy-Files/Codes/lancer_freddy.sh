#!/bin/bash
sleep 5
chown root:gpio /dev/gpiochip0
chown root:gpio /dev/gpiochip1
chmod g+rw /dev/gpiochip0
chmod g+rw /dev/gpiochip1
cd /home/freddy/animatronic
source bin/activate
export SDL_AUDIODRIVER=alsa
export AUDIODEV=hw:2,0
python telecommande.py