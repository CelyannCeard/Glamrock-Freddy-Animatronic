# -*- coding: utf-8 -*-
import os
import subprocess
import tempfile
import pygame
import whisper
import sounddevice as sd
import scipy.io.wavfile as wav
import numpy as np
import webrtcvad
import collections
from groq import Groq

# --- Config ---
SOUNDS_DIR = os.path.expanduser("~/animatronic/sounds")
PIPER_MODEL = os.path.expanduser("~/piper-voices/fr_FR-siwis-medium.onnx")
#Je masque ma clé pour le rendu
GROQ_API_KEY = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
SAMPLE_RATE = 16000
VAD_AGGRESSIVENESS = 1  # 0 à 3, plus élevé = moins sensible au bruit
SILENCE_DURATION = 1.5  # secondes de silence avant de couper

SYSTEM_PROMPT = """Tu es Glamrock Freddy, un animatronique rockstar sympathique et théâtral inspiré de FNAF.
Tu parles toujours en français, avec enthousiasme et humour.
Tes réponses sont TRÈS COURTES : maximum 2 phrases.
Tu ne parles jamais de ce que tu es techniquement."""

# --- Init ---
pygame.mixer.init()
client = Groq(api_key=GROQ_API_KEY)
vad = webrtcvad.Vad(VAD_AGGRESSIVENESS)

print("Chargement de Whisper...")
whisper_model = whisper.load_model("tiny")
print("Whisper pret !")

def jouer_mp3(nom_fichier):
    chemin = os.path.join(SOUNDS_DIR, nom_fichier)
    if os.path.exists(chemin):
        pygame.mixer.music.load(chemin)
        pygame.mixer.music.play()
        while pygame.mixer.music.get_busy():
            pygame.time.wait(100)

def parler(texte):
    # Nettoyer le texte pour éviter les problèmes avec les guillemets
    texte = texte.replace('"', '').replace("'", ' ')
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
        tmp_wav = f.name
    subprocess.run(
        f'echo "{texte}" | piper --model {PIPER_MODEL} --output_file {tmp_wav}',
        shell=True,
        stderr=subprocess.DEVNULL
    )
    pygame.mixer.music.load(tmp_wav)
    pygame.mixer.music.play()
    while pygame.mixer.music.get_busy():
        pygame.time.wait(100)
    os.unlink(tmp_wav)

def ecouter_avec_vad():
    """Enregistre uniquement quand quelqu'un parle, s'arrête automatiquement au silence."""
    print("En ecoute... (parlez !)")
    
    frame_duration = 30  # ms
    frame_size = int(SAMPLE_RATE * frame_duration / 1000)
    
    # Nombre de frames de silence avant d'arrêter
    silence_frames = int(SILENCE_DURATION * 1000 / frame_duration)
    
    ring_buffer = collections.deque(maxlen=silence_frames)
    triggered = False
    voiced_frames = []
    pre_buffer = collections.deque(maxlen=10)  # frames avant la parole
    
    stream = sd.InputStream(samplerate=SAMPLE_RATE, channels=1, dtype='int16', blocksize=frame_size)
    stream.start()
    
    try:
        while True:
            frame, _ = stream.read(frame_size)
            frame_bytes = frame.tobytes()
            
            is_speech = vad.is_speech(frame_bytes, SAMPLE_RATE)
            
            if not triggered:
                pre_buffer.append(frame)
                if is_speech:
                    triggered = True
                    print("Parole detectee !")
                    voiced_frames.extend(list(pre_buffer))
                    voiced_frames.append(frame)
            else:
                voiced_frames.append(frame)
                ring_buffer.append(is_speech)
                
                # Si on a assez de silence, on arrête
                if len(ring_buffer) == ring_buffer.maxlen:
                    if not any(ring_buffer):
                        print("Fin de parole detectee.")
                        break
    finally:
        stream.stop()
        stream.close()
    
    if not voiced_frames:
        return None
    
    audio = np.concatenate(voiced_frames, axis=0)
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
        tmp_wav = f.name
    wav.write(tmp_wav, SAMPLE_RATE, audio)
    return tmp_wav

def transcrire(chemin_audio):
    result = whisper_model.transcribe(chemin_audio, language="fr", fp16=False)
    return result["text"].strip()

def demander_groq(texte):
    response = client.chat.completions.create(
        model="llama-3.3-70b-versatile",
        messages=[
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": texte}
        ],
        max_tokens=80,
        temperature=0.7
    )
    return response.choices[0].message.content.strip()

# --- Boucle principale ---
print("Freddy est pret ! Je vous ecoute...")
jouer_mp3("Bonjour_Nom_FR.mp3")

while True:
    try:
        audio_path = ecouter_avec_vad()
        if audio_path is None:
            continue
        
        texte = transcrire(audio_path)
        os.unlink(audio_path)
        
        if not texte or len(texte) < 3:
            continue
            
        print(f"Vous avez dit : {texte}")
        reponse = demander_groq(texte)
        print(f"Freddy : {reponse}")
        parler(reponse)
        
    except KeyboardInterrupt:
        print("\nFreddy s'eteint...")
        break
    except Exception as e:
        print(f"Erreur : {e}")
        continue