# -*- coding: utf-8 -*-
import os
import threading
import pygame
from flask import Flask, request, jsonify, send_from_directory
import servos
import leds
import ia_vocale
app = Flask(__name__)
SOUNDS_DIR = os.path.expanduser("~/animatronic/sounds")
pygame.mixer.init()
leds.init()
servos.init()
# --- Sons ---
@app.route("/son/<int:numero>")
def jouer_son(numero):
    sons = {
        1: "Bonjour_Nom_FR.mp3",
        2: "Des_Bonbon_Ou_Je_Vous_Mors.mp3",
        3: "Rattache_Ma_Tete.mp3",
        4: "Bonjour_Parle_FR.mp3",
        5: "Way_To_Go_Superstar.mp3",
        6: "Crise_Existentielle.mp3",
        7: "Daisy_Bell.mp3",
        8: "Let_s_Groove.mp3",
        9: "I_Am_Your_Friend.mp3",
        10: "Felicitation.mp3",
        11: "Hack_Jumpscare.mp3",
        12: "Come_Back.mp3",
        13: "Evil_Superstar.mp3",
        14: "Why_Do_You_Hide.mp3",
        15: "I_Cant_Control_Myself.mp3",
        16: "Never_Be_Alone.mp3",
        17: "Freddy_Song.mp3",
        18: "Nightcall.mp3",
        19: "FNAF_1_Song.mp3",
        20: "Blinding_Lights.mp3",
        21: "Harder_Better_Faster_Stronger.mp3",
        22: "Nose_Boop.mp3",
        23: "Its_been_so_long.mp3"
    }
    if numero in sons:
        chemin = os.path.join(SOUNDS_DIR, sons[numero])
        if os.path.exists(chemin):
            pygame.mixer.music.stop()
            pygame.mixer.music.load(chemin)
            pygame.mixer.music.play()
            return jsonify({"status": "ok", "son": sons[numero]})
    return jsonify({"status": "erreur"}), 404
@app.route("/stop")
def stop_son():
    pygame.mixer.music.stop()
    return jsonify({"status": "ok"})
@app.route("/volume/<int:val>")
def volume(val):
    val = max(0, min(100, val))
    pygame.mixer.music.set_volume(val / 100)
    return jsonify({"status": "ok", "volume": val})
# --- LEDs ---
@app.route("/couleur/<nom>")
def changer_couleur(nom):
    leds.couleur(nom)
    return jsonify({"status": "ok", "couleur": nom})
# --- Servos ---
@app.route("/servo/gauche")
def servo_gauche():
    servos.yeux_gauche()
    return jsonify({"status": "ok"})
@app.route("/servo/droite")
def servo_droite():
    servos.yeux_droite()
    return jsonify({"status": "ok"})
@app.route("/servo/centre")
def servo_centre():
    servos.yeux_centre()
    return jsonify({"status": "ok"})
@app.route("/servo/cligner")
def servo_cligner():
    servos.cligner()
    return jsonify({"status": "ok"})
@app.route("/servo/oreilles/<position>")
def servo_oreilles(position):
    if position == "haut":
        servos.oreilles_haut()
    elif position == "bas":
        servos.oreilles_bas()
    else:
        servos.oreilles_centre()
    return jsonify({"status": "ok"})
@app.route("/autonome/<etat>")
def autonome(etat):
    if etat == "on":
        servos.demarrer_autonome()
    else:
        servos.arreter_autonome()
    return jsonify({"status": "ok", "autonome": etat})
# --- IA vocale ---
@app.route("/ia/<etat>")
def ia(etat):
    if etat == "on":
        ia_vocale.demarrer_ia()
    else:
        ia_vocale.arreter_ia()
    return jsonify({"status": "ok", "ia": etat, "active": ia_vocale.ia_active})
@app.route("/ia/status")
def ia_status():
    return jsonify({"active": ia_vocale.ia_active})
# --- Page principale ---
@app.route("/")
def index():
    return send_from_directory("static", "index.html")
@app.route("/static/<path:filename>")
def static_files(filename):
    return send_from_directory("static", filename)
def demarrer():
    app.run(host="0.0.0.0", port=5000, debug=False)
if __name__ == "__main__":
    demarrer()