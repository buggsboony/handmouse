#include <Arduino.h>
#include <Wire.h>
#include <FastIMU.h> // La lib principale
#include <USB.h>
#include <USBHIDMouse.h>

// On définit le capteur
BMI160 IMU; 
USBHIDMouse Mouse;

// Structure pour stocker les données
calData calib = { 0 }; 
GyroData data; 

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA, SCL pour ton S3
  //Wire.setClock(400000);

  Wire.setClock(  300000); //2026-05-10 13:24:34 - Debug drift


  // Initialisation : FastIMU utilise directement l'adresse
  // 0x68 (SAD au GND) ou 0x69 (SAD au 3V3)
  int err = IMU.init(calib, 0x69);
  if (err != 0) {
    Serial.print("Erreur initialisation: ");
    Serial.println(err);
    while (1){
        Serial.print("."); delay(100);
    }
  }

  
   // IMU.setGyroRange(2000); // On veut une réponse rapide : 2000 degrés/seconde
    IMU.setGyroRange(500); // Plus précis pour les petits mouvements
    Serial.println("Calibration... ne bougez plus !");
    delay(1000);


  Mouse.begin();
  USB.begin();
  
  Serial.println("FastIMU prête ! Calibrage automatique...");
  // Optionnel : tu peux ajouter une routine de calibrage ici
}

// --- Ajoute cette variable globale pour compenser ---
float biasY = 0; // On va la calculer au début
// --- VARIABLES GLOBALES ---
float accumulatorX = 0;
float accumulatorY = 0;
// --- Nouveaux réglages pour éliminer les saccades ---
float sensitivity = 0.4; // On remonte un peu pour éviter le blocage
float smoothing = 0.4;   // Très haut pour "étirer" le pixel dans le temps
float softX = 0, softY = 0;


float accumY =0;
float smoothY;


int k=0;
void loop() {
  IMU.update();
  IMU.getGyro(&data);

  float rawY = -data.gyroY;
  float targetY = 0;

  // 1. DEADZONE PLUS LARGE
  // Le BMI160 a souvent un bruit de fond qui monte à 60-80
  int threshold = 1; 

  if (abs(rawY) > threshold) {
    targetY = rawY * sensitivity;
  } else {
    // FORCE LE ZÉRO : Si on est sous le seuil, on vide les mémoires
    // C'est ça qui tue le drift lent.
    targetY = 0;
    smoothY = 0;
    accumY = 0;
  }

  // 2. LISSAGE ET ACCUMULATION (uniquement si mouvement réel)
  if (targetY != 0) {
    smoothY = (targetY * (1.0 - smoothing)) + (smoothY * smoothing);
    accumY += smoothY;
  }

  int mY = (int)accumY;

  if (mY != 0) {
    Mouse.move(0, mY);
    accumY -= mY; // On garde le reste
  }

  delay(2);
}