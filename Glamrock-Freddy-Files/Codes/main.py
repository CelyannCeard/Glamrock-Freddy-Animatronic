# -*- coding: utf-8 -*-
import threading
import subprocess
import sys
import os
def lancer_freddy():
    os.system("cd ~/animatronic && source bin/activate && python freddy.py")
def lancer_telecommande():
    import telecommande
    telecommande.demarrer()
if __name__ == "__main__":
    print("Demarrage de Freddy...")
    # Lance la telecommande dans un thread
    t_telecommande = threading.Thread(target=lancer_telecommande, daemon=True)
    t_telecommande.start()
    print("Telecommande demarree sur http://192.168.0.112:5000")
    # Lance freddy.py dans un thread
    t_freddy = threading.Thread(target=lancer_freddy, daemon=True)
    t_freddy.start()
    print("Pipeline vocal demarre !")
    # Garde le programme en vie
    try:
        while True:
            import time
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nArret de Freddy...")
        sys.exit(0)