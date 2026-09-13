<h1 align="center">🐻 Glamrock Freddy - AI-Powered Animatronic Head</h1>

<p align="center">
  <i>A fully autonomous, conversational animatronic robot powered by a NVIDIA Jetson Nano and LLMs.</i>
</p>

<p align="center">
  <i>(⚙️Still in development🛠️)</i>
</p>

## 📝 Overview
This repository contains the software architecture and control code for a life-size, fully functional animatronic head based on **Glamrock Freddy** (From Five Nights at Freddy's: Security Breach). 

<p align="center">
<img width="616" height="353" alt="capsule_616x353" src="https://github.com/user-attachments/assets/979dfbc3-8aa0-48f0-93e9-e9473cbf4cb6" />
</p>

Originally developed as a homemade personal project and later as a 4th-year Embedded Systems engineering project at Polytech Lyon, this project evolved from a simple Arduino-controlled mask into a complex robotic system. The current version acts as a standalone autonomous agent capable of:
*   **Real-time Conversational AI:** Listening to users, processing speech-to-text, generating LLM-based responses, and synthesizing realistic voice replies.
*   **Autonomous Movements:** Procedurally generated eye tracking, blinking, and ear animations.
*   **Remote Web Control:** A Flask-based web interface to manually trigger actions, sounds, and LED colors over Wi-Fi.

<br>

## 🧠 Software Architecture (The AI Pipeline)
The brain of the animatronic is a **NVIDIA Jetson Nano 4GB** running a highly concurrent Python 3.8 environment. 
The conversational AI pipeline runs in a dedicated thread to prevent blocking the hardware control loops:

1.  **Voice Activity Detection (VAD):** `webrtcvad` detects human speech and triggers audio recording.
2.  **Speech-to-Text (STT):** `openai-whisper` transcribes the audio into text.
3.  **Language Model (LLM):** The transcribed text is sent to the **Groq API** (running `LLaMA 3.3-70B`) to generate a fast, context-aware, and character-accurate response.
4.  **Text-to-Speech (TTS):** `Piper TTS` (running locally) synthesizes the French audio response (`fr_FR-siwis-medium`).

<br>

## ⚙️ Hardware & Electronics Stack
The mechanical structure was fully designed in **Creo Parametric** (based on modified 3D-printed shells). The electronics are securely housed inside the head.

| Component | Description |
| :--- | :--- |
| **Main Compute** | NVIDIA Jetson Nano 4GB (JetPack 4.6 / Ubuntu 18.04) |
| **PWM Controller** | PCA9685 (16-channel, I2C bus) |
| **Actuators** | 5x Miuzei 15 kg-cm Servomotors (Eyes, Eyelids, Ears) |
| **Lighting** | 2x RGB LEDs (Common cathode) |
| **Power Supply** | Konect LiPo 2S (7.4V 5600mAh) + Dollatek 300W/20A DC-DC converter (5V) |
| **Audio & Network** | Jabra Evolve2 USB Mic, 3Ω Speaker, ASUS USB Wi-Fi Dongle |

<p align="center">
<img width="776" height="713" alt="image" src="https://github.com/user-attachments/assets/f76fea76-6e6d-48f4-9cb6-64e46fad9005" />
</p>

<br>

## 🎨 3D Design & Mechanical Modeling

* **Exterior Shells:** The outer head design, jaw, and hat models were 3D printed, polished, and painted using STL files adapted from cosplay content creator LittleJem (YouTube).
* **Custom Mechanics:** All internal mechanical components—including servomotor mounts, eye rotation mechanisms, and linkage systems for ear and eyelid articulation—were fully modeled from scratch in **PTC Creo (Creo Parametric)** to adapt the original shell and optimize kinematic performance.

<p align="center">
  <img width="230" height="307" alt="IMG_20250820_175437" src="https://github.com/user-attachments/assets/37b342b0-4491-499c-99dc-f6ce012d260b" />
  <img width="230" height="307" alt="IMG_20251202_235558" src="https://github.com/user-attachments/assets/43e2dc1d-bd05-4f5e-a646-ea6b0d1a92dc" />
  <img width="230" height="307" alt="IMG_20251205_154508" src="https://github.com/user-attachments/assets/c0560b96-a9f5-4084-9312-a3b03e87e16c" />
</p>



<br>

## 📁 Repository Structure
The project relies on a modular, multithreaded architecture:

*   `telecommande.py`: Central orchestrator. Launches the Flask server and initializes all threads.
*   `ia_vocale.py`: Encapsulates the entire VAD -> Whisper -> Groq -> Piper TTS pipeline in a background thread.
*   `servos.py`: Handles PCA9685 I2C communication via `adafruit-servokit`. Includes the autonomous random movement loop.
*   `leds.py`: Controls the RGB LED states and color mixing via PWM.
*   `calibration_servos.py`: Interactive CLI tool to empirically calibrate the min/max/center physical angles of each servomotor.
*   `static/index.html`: The remote control web interface.

<br>

## 🚀 Future Perspectives
While the current system is fully functional, the following upgrades and extensions are planned:

*   **Custom LED Rings:** Replacing current lighting with custom-made LED rings through precise cutting and soldering of LED strips.
*   **Microphone & Audio Upgrade:** Upgrading the microphone setup and integrating a PAM8403 amplifier for a more robust standalone speaker output.
*   **Distributed Architecture with ROS 2:** Offloading the heavy AI reasoning workload to a host PC, establishing a **ROS 2** communication bridge between the laptop and the Jetson Nano.
*   **Computer Vision:** Integrating a camera inside the eyes coupled with face-tracking algorithms to allow the animatronic to track and follow its interlocutor.
*   **Mechanical & Aesthetic Refinements:** Improving movement fluidity (Remanufacture of the eyes and their mechanism), extending animated body parts, and touching up the paint on degraded sections (such as the eyelids).
*   **Enhancement:** Replacing all jumper wires and breadboard connections with custom PCBs and permanent solder joints.

<br>

## 📹 Demonstrations

Full demonstration on You Tube (In French): https://youtu.be/uBhqnxuMKF0

https://github.com/user-attachments/assets/7f98c9fb-036b-433f-b017-530b4dbab740



---
*Developed by Celyann CEARD.*
