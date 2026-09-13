# -*- coding: utf-8 -*-
try:
    import board
    import busio
    from adafruit_pca9685 import PCA9685
    PCA_DISPONIBLE = True
    print("PCA9685 detecte !")
except Exception:
    PCA_DISPONIBLE = False
    print("PCA9685 non disponible - mode simulation")
# --- Canaux PCA9685 ---
CANAL_R = 7
CANAL_G = 6
CANAL_B = 5
# --- Couleurs predefinies ---
COULEURS = {
    "bleu_ciel":  (0, 150, 255),
    "bleu_fonce": (0, 0, 255),
    "vert":       (0, 255, 0),
    "jaune":      (255, 255, 0),
    "rouge":      (255, 0, 0),
    "violet":     (255, 0, 255),
    "blanc":      (255, 255, 255),
    "eteint":     (0, 0, 0)
}
pca = None
def init():
    global pca
    if not PCA_DISPONIBLE:
        return
    i2c = busio.I2C(board.SCL, board.SDA)
    pca = PCA9685(i2c)
    pca.frequency = 50
    eteindre()
    print("LEDs initialisees !")
def set_couleur(r, g, b):
    if not PCA_DISPONIBLE or pca is None:
        return
    duty_r = int(r / 255 * 65535)
    duty_g = int(g / 255 * 65535)
    duty_b = int(b / 255 * 65535)
    pca.channels[CANAL_R].duty_cycle = duty_r
    pca.channels[CANAL_G].duty_cycle = duty_g
    pca.channels[CANAL_B].duty_cycle = duty_b
def couleur(nom):
    if nom in COULEURS:
        r, g, b = COULEURS[nom]
        set_couleur(r, g, b)
def eteindre():
    set_couleur(0, 0, 0)
def cleanup():
    eteindre()