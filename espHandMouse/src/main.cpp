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
  Wire.setClock(200000);

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

  // Configuration pour le Gaming
  // On veut une réponse rapide : 2000 degrés/seconde
  IMU.setGyroRange(2000); 

  Mouse.begin();
  USB.begin();
  
  Serial.println("FastIMU prête ! Calibrage automatique...");
  // Optionnel : tu peux ajouter une routine de calibrage ici
}


// --- VARIABLES GLOBALES ---
float accumulatorX = 0;
float accumulatorY = 0;
// --- Nouveaux réglages pour éliminer les saccades ---
float sensitivity = 0.1; // On remonte un peu pour éviter le blocage
float smoothing = 0.4;   // Très haut pour "étirer" le pixel dans le temps
float softX = 0, softY = 0;


int k=0;

void loop() {
  IMU.update();
  IMU.getGyro(&data);

  // 1. On calcule la force brute
  float rawX = 0, rawY = 0;
  int dz = 0.1; // Deadzone stable

//   //2026-05-10 12:53:45 - Breadboard horizontal, usb port pointent vers la droite
//   if (abs(data.gyroZ) > dz) rawX = (-data.gyroZ) * sensitivity;
//   if (abs(data.gyroY) > dz) rawY = (-data.gyroY) * sensitivity;

  //2026-05-10 12:53:45 - Droitier, Breadboard Vertical, usb port pointent vers le haut
  if (abs(data.gyroX) > dz) rawX = -(data.gyroX) * sensitivity;
  if (abs(data.gyroZ) > dz) rawY = (data.gyroZ) * sensitivity;


  // 2. FILTRE PASSE-BAS (C'est ça qui enlève les saccades)
  // On crée une moyenne glissante très lourde
  softX = (softX * smoothing) + (rawX * (1.0 - smoothing));
  softY = (softY * smoothing) + (rawY * (1.0 - smoothing));

  // 3. Accumulation de précision
  accumulatorX += softX;
  accumulatorY += softY;

  int mX = (int)accumulatorX;
  int mY = (int)accumulatorY;

  if (mX != 0 || mY != 0) {
    Mouse.move(mX, mY);
    accumulatorX -= mX;
    accumulatorY -= mY;
  }

  // On ralentit un tout petit peu pour laisser le buffer USB respirer sous Manjaro
  if(k++>=10)
  {
    k=0;
    delay(1); 
  }
}