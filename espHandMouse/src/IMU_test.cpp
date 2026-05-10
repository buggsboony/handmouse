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
  Wire.setClock(400000);

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

int k=0;
void loop() {
  k++;
  if(k>=1000){k=0;
  Serial.print("!");
  }
  // update() récupère les dernières données du capteur
  IMU.update();
  IMU.getGyro(&data);

  // Rappel : Position "Poignée de main"
  // Horizontal (X souris) = axe Z du gyro
  // Vertical (Y souris) = axe Y du gyro
  
  float sensitivity = 0.5; // Ajuste selon tes DPI
  float deadzone = 0.1;   // Pour éviter que le viseur tremble tout seul

  int moveX = 0;
  int moveY = 0;

  if (abs(data.gyroZ) > deadzone) moveX = (int)(data.gyroZ * sensitivity);
  if (abs(data.gyroY) > deadzone) moveY = (int)(-data.gyroY * sensitivity);

  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, moveY);
  }

  // Pas de delay() pour rester ultra-réactif
  
}