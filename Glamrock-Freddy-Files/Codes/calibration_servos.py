# -*- coding: utf-8 -*-
"""
Script de calibration des servos.
Permet de tester un canal a la fois en entrant un angle (0-180)
pour trouver empiriquement les bornes min/max/centre reelles.
Utilisation:
  python calibration_servos.py
Commandes:
  - Entrer un nombre (0-180) : positionne le servo a cet angle
  - "c" : change de canal
  - "q" : quitte
"""
from adafruit_servokit import ServoKit
kit = ServoKit(channels=16)
# Elargir la plage de pulse width pour ne pas etre limite
# par les valeurs par defaut (1000-2000us) pendant la calibration
PULSE_MIN = 500
PULSE_MAX = 2500
NOMS_CANAUX = {
    0: "Oeil gauche",
    1: "Oeil droit",
    2: "Paupiere",
    3: "Oreille droite",
    4: "Oreille gauche",
}
def configurer_canal(canal):
    kit.servo[canal].set_pulse_width_range(PULSE_MIN, PULSE_MAX)
    kit.servo[canal].actuation_range = 180
def main():
    print("=== Calibration servos ===")
    print("Canaux disponibles :")
    for c, nom in NOMS_CANAUX.items():
        print(f"  {c} : {nom}")
    print()
    while True:
        try:
            canal_str = input("Quel canal tester ? (0-4, ou 'q' pour quitter) : ").strip()
        except (EOFError, KeyboardInterrupt):
            break
        if canal_str.lower() == "q":
            break
        try:
            canal = int(canal_str)
            if canal not in NOMS_CANAUX:
                print("Canal invalide (0-4).")
                continue
        except ValueError:
            print("Entree invalide.")
            continue
        configurer_canal(canal)
        nom = NOMS_CANAUX[canal]
        print(f"\n--- Canal {canal} ({nom}) ---")
        print("Entre un angle (0-180), 'c' pour changer de canal, 'q' pour quitter.\n")
        while True:
            try:
                val = input(f"[{nom}] Angle (0-180) > ").strip()
            except (EOFError, KeyboardInterrupt):
                return
            if val.lower() == "q":
                return
            if val.lower() == "c":
                break
            try:
                angle = float(val)
            except ValueError:
                print("Entree invalide, recommence.")
                continue
            if angle < 0 or angle > 180:
                print("Angle hors limites (0-180), recommence.")
                continue
            kit.servo[canal].angle = angle
            print(f"  -> Canal {canal} ({nom}) positionne a {angle} degres")
if __name__ == "__main__":
    main()