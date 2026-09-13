# -*- coding: utf-8 -*-
import time
import threading
import random
try:
    from adafruit_servokit import ServoKit
    kit = ServoKit(channels=16)
    SERVOS_DISPONIBLES = True
    print("PCA9685 detecte !")
except Exception:
    SERVOS_DISPONIBLES = False
    print("PCA9685 non connecte - mode simulation")

# --- Canaux servos ---
OEIL_GAUCHE = 0
OEIL_DROIT = 1
PAUPIERE = 2
OREILLE_DROITE = 3
OREILLE_GAUCHE = 4

# --- Positions (en degres, calibrees empiriquement) ---

# Oeil gauche (canal 0)
OEIL_GAUCHE_DROITE = 45    # regarde a droite
OEIL_GAUCHE_GAUCHE = 120   # regarde a gauche
OEIL_GAUCHE_CENTRE = 82.5

# Oeil droit (canal 1)
OEIL_DROIT_DROITE = 55     # regarde a droite
OEIL_DROIT_GAUCHE = 135    # regarde a gauche
OEIL_DROIT_CENTRE = 95

# Paupieres (canal 2)
PAUPIERE_ETONNE = 125      # plus ouvert que la normale
PAUPIERE_OUVERT = 135      # ouvert normal
PAUPIERE_SERIEUX = 155      # mi-ferme
PAUPIERE_FERME = 180

# Oreille droite (canal 3)
OREILLE_DROITE_BAS = 82
OREILLE_DROITE_HAUT = 88
OREILLE_DROITE_CENTRE = 85

# Oreille gauche (canal 4) - sens inverse de la droite
OREILLE_GAUCHE_HAUT = 112
OREILLE_GAUCHE_BAS = 118
OREILLE_GAUCHE_CENTRE = 115

# --- Etat global ---
autonome = True
_thread = None


def set_servo(canal, angle):
    if not SERVOS_DISPONIBLES:
        return
    angle = max(0, min(180, angle))
    kit.servo[canal].angle = angle


def yeux_centre():
    set_servo(OEIL_GAUCHE, OEIL_GAUCHE_CENTRE)
    set_servo(OEIL_DROIT, OEIL_DROIT_CENTRE)


def yeux_gauche():
    set_servo(OEIL_GAUCHE, OEIL_GAUCHE_GAUCHE)
    set_servo(OEIL_DROIT, OEIL_DROIT_GAUCHE)


def yeux_droite():
    set_servo(OEIL_GAUCHE, OEIL_GAUCHE_DROITE)
    set_servo(OEIL_DROIT, OEIL_DROIT_DROITE)


def paupieres_ouvertes():
    set_servo(PAUPIERE, PAUPIERE_OUVERT)


def paupieres_fermees():
    set_servo(PAUPIERE, PAUPIERE_FERME)


def paupieres_serieux():
    set_servo(PAUPIERE, PAUPIERE_SERIEUX)


def paupieres_etonne():
    set_servo(PAUPIERE, PAUPIERE_ETONNE)


def cligner():
    paupieres_fermees()
    time.sleep(0.3)
    paupieres_ouvertes()


def oreilles_centre():
    set_servo(OREILLE_DROITE, OREILLE_DROITE_CENTRE)
    set_servo(OREILLE_GAUCHE, OREILLE_GAUCHE_CENTRE)


def oreilles_haut():
    set_servo(OREILLE_DROITE, OREILLE_DROITE_HAUT)
    set_servo(OREILLE_GAUCHE, OREILLE_GAUCHE_HAUT)


def oreilles_bas():
    set_servo(OREILLE_DROITE, OREILLE_DROITE_BAS)
    set_servo(OREILLE_GAUCHE, OREILLE_GAUCHE_BAS)


def oreilles_animation():
    oreilles_bas()
    time.sleep(0.5)
    oreilles_haut()
    time.sleep(0.3)
    oreilles_centre()


def sequence_autonome():
    """Boucle de mouvements autonomes des yeux et oreilles."""
    global autonome

    dernier_clignement = time.time()
    dernier_mvt_oreilles = time.time()

    mouvements_yeux = [yeux_gauche, yeux_droite, yeux_centre]
    etape = 0
    delais = [2.0, 2.0, 3.0, 1.0, 1.0, 5.0]

    while autonome:
        now = time.time()

        # Clignement toutes les 4 secondes
        if now - dernier_clignement > 4.0:
            cligner()
            dernier_clignement = now

        # Mouvement des oreilles aleatoire
        if now - dernier_mvt_oreilles > random.uniform(8, 15):
            choix = random.randint(0, 2)
            if choix == 0:
                oreilles_haut()
                time.sleep(0.6)
                oreilles_centre()
            elif choix == 1:
                oreilles_bas()
                time.sleep(0.6)
                oreilles_centre()
            else:
                oreilles_animation()
            dernier_mvt_oreilles = now

        # Mouvement des yeux
        mouvement = mouvements_yeux[etape % len(mouvements_yeux)]
        mouvement()

        delai = delais[etape % len(delais)]
        # Double clignement aleatoire
        if random.randint(0, 100) < 15:
            time.sleep(delai * 0.5)
            cligner()

        time.sleep(delai)
        etape += 1


def demarrer_autonome():
    """Lance la sequence autonome dans un thread separe."""
    global autonome, _thread
    autonome = True
    _thread = threading.Thread(target=sequence_autonome, daemon=True)
    _thread.start()


def arreter_autonome():
    """Arrete la sequence autonome."""
    global autonome
    autonome = False


def init():
    """Initialise les servos en position de depart."""
    print("Initialisation des servos...")
    paupieres_fermees()
    time.sleep(1)
    paupieres_ouvertes()
    yeux_centre()
    oreilles_centre()
    print("Servos prets !")