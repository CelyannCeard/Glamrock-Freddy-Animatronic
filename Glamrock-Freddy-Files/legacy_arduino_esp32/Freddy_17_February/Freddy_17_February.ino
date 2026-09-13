#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// --- Configuration WiFi ---
const char* ssid = "ESP32_LED_Control";
const char* password = "123456789";
WebServer server(80);

// --- Valeurs servo ---
#define SERVO_MIN 330 // droite
#define SERVO_MAX 580 // gauche
#define SERVO_CENTRE 455
#define PAUPIERE_FERME 600
#define PAUPIERE_OUVERT 460
#define PAUPIERE_SERIEUX 530
#define OREILLE_DROITE_MAX 365
#define OREILLE_DROITE_MIN 340
#define OREILLE_GAUCHE_MAX 445
#define OREILLE_GAUCHE_MIN 425
#define OREILLE_DROITE_CENTRE 352
#define OREILLE_GAUCHE_CENTRE 435

// --- Liste des MP3 ---
/*
1 : Bonjour_Nom_FR
2 : Rattache_Ma_Tete
3 : Des_Bonbon_Ou_Je_Vous_Mors
4 : Bonjour_Parle_FR
5 : Way_To_Go_Superstar
6 : Crise_Existentielle
7 : Daisy_Bell
8 : Let_s_Groove
9 : I_Am_Your_Friend
10 : Felicitation
11 : Hack_Jumpscare
12 : Come_Back
13 : Evil_Superstar
14 : Why_Do_You_Hide
15 : I_Cant_Control_Myself
16 : Never_Be_Alone
17 : Freddy_Song
18 : Nightcall
19 : FNAF_1_Song
20 : Blinding_Lights
21 : Harder_Better_Faster_Stronger
22 : Nose_Boop
23 : Its_been_so_long
*/

// --- LEDs ---
#define R1 2
#define B1 4
#define G1 15
#define BUSY 13

// --- DFPlayer ---
#define RXD2 16
#define TXD2 17

int HPtrack = 1;
int dernierePisteJouee = 0;
int LEDtrack = 1; // Commence à 1 = Bleu ciel
bool enCoursLecture = false;

DFRobotDFPlayerMini fxPlayer;

// ----- Tableau des couleurs RGB (8 couleurs) -----
const int couleurs[8][3] = {
  {0, 150, 255},    // 1 - Bleu ciel
  {0, 0, 255},      // 2 - Bleu foncé
  {0, 255, 0},      // 3 - Vert
  {255, 255, 0},    // 4 - Jaune
  {255, 0, 0},      // 5 - Rouge
  {255, 0, 255},    // 6 - Violet
  {255, 255, 255},  // 7 - Blanc
  {0, 0, 0}         // 8 - Éteint
};

// ----- Fonctions servo -----
void regardeGauche() { pwm.setPWM(0, 0, SERVO_MAX); pwm.setPWM(1, 0, SERVO_MAX); }
void regardeDroite() { pwm.setPWM(0, 0, SERVO_MIN); pwm.setPWM(1, 0, SERVO_MIN); }
void regardeCentre() { pwm.setPWM(0, 0, SERVO_CENTRE); pwm.setPWM(1, 0, SERVO_CENTRE); }
void fermerPaupieres() { pwm.setPWM(2, 0, PAUPIERE_FERME); }
void ouvrirPaupieres() { pwm.setPWM(2, 0, PAUPIERE_OUVERT); }
void oreillesHaut() { pwm.setPWM(3, 0, OREILLE_DROITE_MAX); pwm.setPWM(4, 0, OREILLE_GAUCHE_MIN); }
void oreillesBas() { pwm.setPWM(3, 0, OREILLE_DROITE_MIN); pwm.setPWM(4, 0, OREILLE_GAUCHE_MAX); }
void oreillesCentre() { pwm.setPWM(3, 0, OREILLE_DROITE_CENTRE); pwm.setPWM(4, 0, OREILLE_GAUCHE_CENTRE); }

void oreillesAnimation() {
  oreillesBas(); delay(500);
  oreillesHaut(); delay(300);
  oreillesCentre();
}

void clignerYeux() {
  fermerPaupieres();
  delay(400);
  ouvrirPaupieres();
}

// ----- Fonction pour changer la couleur des LEDs (8 couleurs) -----
void changerCouleur(int nouvelleCouleur) {
  LEDtrack = nouvelleCouleur;
  if (LEDtrack < 1) LEDtrack = 1;
  if (LEDtrack > 8) LEDtrack = 8;
  
  Serial.print("Changement couleur vers: ");
  switch(LEDtrack) {
    case 1: Serial.println("Bleu ciel"); break;
    case 2: Serial.println("Bleu foncé"); break;
    case 3: Serial.println("Vert"); break;
    case 4: Serial.println("Jaune"); break;
    case 5: Serial.println("Rouge"); break;
    case 6: Serial.println("Violet"); break;
    case 7: Serial.println("Blanc"); break;
    case 8: Serial.println("Éteint"); break;
  }
  
  int couleurIndex = LEDtrack - 1;
  analogWrite(R1, couleurs[couleurIndex][0]);
  analogWrite(G1, couleurs[couleurIndex][1]);
  analogWrite(B1, couleurs[couleurIndex][2]);
}

// ----- Gestion des requêtes web -----
void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Erreur: fichier HTML non trouvé");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleCSS() {
  File file = LittleFS.open("/style.css", "r");
  if (!file) {
    server.send(404, "text/plain", "CSS non trouvé");
    return;
  }
  server.streamFile(file, "text/css");
  file.close();
}

void handleImage() {
  String path = server.uri();
  if (path == "/sky_blue_button.png" || path == "/blue_button.png" || 
      path == "/green_button.png" || path == "/yellow_button.png" ||
      path == "/purple_button.png" || path == "/red_button.png" ||
      path == "/light_button.png" || path == "/dark_button.png" ||
      path == "/sound_button.png") {
    File file = LittleFS.open(path, "r");
    if (!file) {
      server.send(404, "text/plain", "Image non trouvée");
      return;
    }
    server.streamFile(file, "image/png");
    file.close();
  } else {
    server.send(404, "text/plain", "Image non trouvée");
  }
}

// ----- Handlers pour les couleurs (8 couleurs) -----
void handleCouleur1() { changerCouleur(1); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur2() { changerCouleur(2); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur3() { changerCouleur(3); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur4() { changerCouleur(4); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur5() { changerCouleur(5); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur6() { changerCouleur(6); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur7() { changerCouleur(7); server.sendHeader("Location", "/"); server.send(303); }
void handleCouleur8() { changerCouleur(8); server.sendHeader("Location", "/"); server.send(303); }

// ----- Handlers pour les sons (23 sons) -----
void handleSon1() {
  // Arrêter toute lecture en cours si nécessaire
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50); // Petit délai pour permettre l'arrêt
  }
  // Jouer la piste 1
  HPtrack = 1;
  dernierePisteJouee = 1;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 1: Bonjour Nom FR");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon2() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 2;
  dernierePisteJouee = 2;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 2: Rattache Ma Tête");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon3() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 3;
  dernierePisteJouee = 3;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 3: Des Bonbon Ou Je Vous Mors");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon4() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 4;
  dernierePisteJouee = 4;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 4: Bonjour Parle FR");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon5() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 5;
  dernierePisteJouee = 5;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 5: Way To Go Superstar");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon6() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 6;
  dernierePisteJouee = 6;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 6: Crise Existentielle");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon7() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 7;
  dernierePisteJouee = 7;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 7: Daisy Bell");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon8() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 8;
  dernierePisteJouee = 8;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 8: Let's Groove");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon9() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 9;
  dernierePisteJouee = 9;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 9: I Am Your Friend");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon10() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 10;
  dernierePisteJouee = 10;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 10: Félicitation");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon11() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 11;
  dernierePisteJouee = 11;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 11: Hack Jumpscare");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon12() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 12;
  dernierePisteJouee = 12;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 12: Come Back");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon13() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 13;
  dernierePisteJouee = 13;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 13: Evil Superstar");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon14() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 14;
  dernierePisteJouee = 14;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 14: Why Do You Hide");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon15() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 15;
  dernierePisteJouee = 15;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 15: I Can't Control Myself");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon16() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 16;
  dernierePisteJouee = 16;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 16: Never Be Alone");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon17() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 17;
  dernierePisteJouee = 17;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 17: Freddy Song");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon18() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 18;
  dernierePisteJouee = 18;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 18: Nighcall");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon19() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 19;
  dernierePisteJouee = 19;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 19: FNAF 1 Song");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon20() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 20;
  dernierePisteJouee = 20;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 20: Blinding Lights");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon21() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 21;
  dernierePisteJouee = 21;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 21: Harder Better Faster Stronger");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon22() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 22;
  dernierePisteJouee = 22;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 22: Nose Boop");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSon23() {
  if (enCoursLecture) {
    fxPlayer.stop();
    enCoursLecture = false;
    delay(50);
  }
  HPtrack = 23;
  dernierePisteJouee = 23;
  fxPlayer.play(HPtrack);
  enCoursLecture = true;
  Serial.println("Son 23: It's Been So Long");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

// ----- Handler pour le volume -----
void handleVolume() {
  if (server.hasArg("value")) {
    int vol = server.arg("value").toInt();
    vol = constrain(vol, 0, 30);
    fxPlayer.volume(vol);
    Serial.print("Volume changé à: ");
    Serial.println(vol);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ----- Animation de démarrage -----
void animationDemarrage() {
  Serial.println("Animation de démarrage des LEDs...");
  analogWrite(R1, 0); analogWrite(G1, 0); analogWrite(B1, 0); delay(100);
  analogWrite(R1, 0); analogWrite(G1, 150); analogWrite(B1, 255); delay(100);
  analogWrite(R1, 0); analogWrite(G1, 0); analogWrite(B1, 0); delay(200);
  analogWrite(R1, 0); analogWrite(G1, 150); analogWrite(B1, 255);
  Serial.println("Animation terminée - LEDs en Bleu ciel");
}

// ----- Gestion audio non-bloquante corrigée -----
void gestionAudio() {
  if (enCoursLecture) {
    // Vérifier si la lecture est terminée (BUSY HIGH = lecture terminée)
    if (digitalRead(BUSY) == HIGH) {
      enCoursLecture = false;
      Serial.print("Audio terminé. Dernière piste jouée: ");
      Serial.println(dernierePisteJouee);
      // NE PAS incrémenter automatiquement HPtrack ici
    }
  }
}

// ----- Séquence des yeux -----
void sequenceYeux() {
  static unsigned long dernierMouvement = 0;
  static unsigned long dernierClignement = 0;
  static unsigned long dernierMvtOreilles = 0;
  static byte etape = 0;
  static byte variante = 0;
  
  if (millis() - dernierClignement > 4000) {
    clignerYeux();
    dernierClignement = millis();
  }
  
  if (millis() - dernierMvtOreilles > random(8000, 15000)) {
    int choix = random(0, 100);
    if (choix < 40) {
      if (choix < 20) { oreillesHaut(); delay(600); oreillesCentre(); }
      else if (choix < 35) { oreillesBas(); delay(600); oreillesCentre(); }
      else { oreillesAnimation(); }
      Serial.println("Mouvement des oreilles");
    }
    dernierMvtOreilles = millis();
  }
  
  unsigned long delaisBase[] = {2000, 2000, 3000, 1000, 1000, 1000, 5000};
  unsigned long delaiActuel = delaisBase[etape];
  
  if (variante == 1 && etape == 2) delaiActuel = 5000;
  if (variante == 2 && (etape == 3 || etape == 5)) delaiActuel = 1500;
  
  if (millis() - dernierMouvement > delaiActuel) {
    
    if (random(0, 100) < 15) variante = random(0, 3);
    
    switch(etape) {
      case 0: regardeGauche(); Serial.print("Regarde à gauche"); break;
      case 1: regardeDroite(); Serial.print("Regarde à droite"); break;
      case 2: 
        regardeCentre(); 
        Serial.print("Regarde au centre");
        if (variante == 1 && random(0, 100) < 30) { delay(1500); Serial.print(" (longue pause)"); }
        break;
      case 3: 
        if (variante == 2) { regardeGauche(); Serial.print("Regarde à gauche"); }
        else { regardeDroite(); Serial.print("Regarde à droite"); }
        break;
      case 4: regardeCentre(); Serial.print("Regarde au centre"); break;
      case 5: 
        if (variante == 2) { regardeDroite(); Serial.print("Regarde à droite"); }
        else { regardeGauche(); Serial.print("Regarde à gauche"); }
        break;
      case 6: 
        regardeCentre(); 
        Serial.print("Regarde au centre");
        if (random(0, 100) < 25) { delay(200); clignerYeux(); Serial.print(" (double clignement)"); }
        break;
    }
    
    switch(variante) {
      case 0: Serial.println(" - Normale"); break;
      case 1: Serial.println(" - Pause longue"); break;
      case 2: Serial.println(" - Altérée"); break;
    }
    
    etape++;
    if (etape > 6) { etape = 0; variante = random(0, 3); }
    dernierMouvement = millis();
  }
}

// ----- Vérification DFPlayer -----
void verifierDFPlayer() {
  if (fxPlayer.available()) {
    if (fxPlayer.readType() == DFPlayerError) {
      Serial.println("Erreur DFPlayer détectée");
    }
  }
}

// ----- Setup -----
void setup() {
  Serial.begin(921600);
  Serial.println("Initialisation ESP32...");

  // LEDs
  pinMode(R1, OUTPUT);
  pinMode(G1, OUTPUT);
  pinMode(B1, OUTPUT);
  pinMode(BUSY, INPUT);

  // Initialisation aléatoire
  randomSeed(analogRead(0));

  // PCA9685
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(60);
  Serial.println("Contrôleur servo initialisé");
  delay(10);

  // Séquence paupières
  Serial.println("Séquence de démarrage des paupières...");
  fermerPaupieres(); delay(2000);
  ouvrirPaupieres(); Serial.println("Paupières ouvertes");

  // Yeux et oreilles position initiale
  regardeCentre(); Serial.println("Yeux au centre");
  oreillesCentre(); Serial.println("Oreilles au centre");
  
  // DFPlayer
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(100);
  if (!fxPlayer.begin(Serial2)) {
    Serial.println("Erreur DFPlayer - Vérifiez les connexions");
  } else {
    fxPlayer.volume(15); // Volume initial à 15/30
    Serial.println("DFPlayer initialisé");
  }
  
  // Animation LEDs
  animationDemarrage();

  // --- WiFi et serveur web ---
  if (!LittleFS.begin()) {
    Serial.println("Erreur LittleFS. Formatage...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("Échec LittleFS. Arrêt.");
      return;
    }
  }
  Serial.println("LittleFS OK");

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("WiFi IP: ");
  Serial.println(IP);

  // Routes web
  server.on("/", handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/sky_blue_button.png", handleImage);
  server.on("/blue_button.png", handleImage);
  server.on("/green_button.png", handleImage);
  server.on("/yellow_button.png", handleImage);
  server.on("/purple_button.png", handleImage);
  server.on("/red_button.png", handleImage);
  server.on("/light_button.png", handleImage);
  server.on("/dark_button.png", handleImage);
  server.on("/sound_button.png", handleImage);
  
  // Routes couleurs (8 couleurs)
  server.on("/couleur1", handleCouleur1);
  server.on("/couleur2", handleCouleur2);
  server.on("/couleur3", handleCouleur3);
  server.on("/couleur4", handleCouleur4);
  server.on("/couleur5", handleCouleur5);
  server.on("/couleur6", handleCouleur6);
  server.on("/couleur7", handleCouleur7);
  server.on("/couleur8", handleCouleur8);
  
  // Routes sons (23 sons)
  server.on("/son1", handleSon1);
  server.on("/son2", handleSon2);
  server.on("/son3", handleSon3);
  server.on("/son4", handleSon4);
  server.on("/son5", handleSon5);
  server.on("/son6", handleSon6);
  server.on("/son7", handleSon7);
  server.on("/son8", handleSon8);
  server.on("/son9", handleSon9);
  server.on("/son10", handleSon10);
  server.on("/son11", handleSon11);
  server.on("/son12", handleSon12);
  server.on("/son13", handleSon13);
  server.on("/son14", handleSon14);
  server.on("/son15", handleSon15);
  server.on("/son16", handleSon16);
  server.on("/son17", handleSon17);
  server.on("/son18", handleSon18);
  server.on("/son19", handleSon19);
  server.on("/son20", handleSon20);
  server.on("/son21", handleSon21);
  server.on("/son22", handleSon22);
  server.on("/son23", handleSon23);

  // Route pour le volume
  server.on("/volume", handleVolume);

  server.onNotFound([]() { server.send(404, "text/plain", "404"); });
  server.begin();
  
  Serial.println("Serveur démarré !");
  Serial.println("Connectez-vous au WiFi: " + String(ssid));
  Serial.println("Ouvrez: http://" + IP.toString());
  Serial.println("Système prêt!");
}

void loop() {
  server.handleClient();
  gestionAudio();
  verifierDFPlayer();
  sequenceYeux();
}