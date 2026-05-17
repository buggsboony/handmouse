#include <Arduino.h>
#include <Wire.h>
#include <FastIMU.h> // La lib principale
#include <USB.h>
#include <USBHIDMouse.h>

//2026-05-17 13:13:42 - Sensor definition
BMI160 IMU; 
USBHIDMouse Mouse;

// Structure pour stocker les données
calData calib = { 0 }; 
GyroData data; 

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA, SCL pour ton S3
  Wire.setClock(300000);

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
  //IMU.setGyroRange(2000); 
  IMU.setGyroRange(980); 

  Mouse.begin();
  USB.begin();
  
  Serial.println("FastIMU prête ! Calibrage automatique...");
  // Optionnel : tu peux ajouter une routine de calibrage ici
}


// --- VARIABLES GLOBALES ---
float accumulatorX = 0;
float accumulatorY = 0;
// --- Nouveaux réglages pour éliminer les saccades ---
float sensitivity = 0.07; // 0.2 = descent, 0.8 = Incontrollable (trop rapide)
float smoothing = 0.4;   // 0.4 = descent Très haut pour "étirer" le pixel dans le temps
float softX = 0, softY = 0;
float accumY =0;
float accumX =0;
float smoothY;
float smoothX;

int k=0;

void loop() {
  IMU.update();
  IMU.getGyro(&data);

  // 1. On calcule la force brute

  //int dz = 0.1; // Deadzone stable


  float targetX = 0;
  float targetY = 0;

//   //2026-05-10 15:12:18 - Breadboard horizontal, usb port pointent vers la droite
//   float rawX = -(data.gyroZ);
//   float rawY = (data.gyroX);
  
//2026-05-10 15:12:18 - Breadboard horizontal, usb port pointent vers la droite
  float rawX = -(data.gyroX);
  float rawY = (data.gyroZ);


//   //2026-05-10 12:53:45 - Breadboard horizontal, usb port pointent vers la droite
//   if (abs(data.gyroZ) > dz) rawX = (-data.gyroZ) * sensitivity;
//   if (abs(data.gyroY) > dz) rawY = (-data.gyroY) * sensitivity;

//   //2026-05-10 12:53:45 - Droitier, Breadboard Vertical, usb port pointent vers le haut
//   if (abs(data.gyroX) > dz) rawX = -(data.gyroX) * sensitivity;
//   if (abs(data.gyroZ) > dz) rawY = (data.gyroZ) * sensitivity;




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


  if (abs(rawX) > threshold) {
    targetX = rawX * sensitivity;
  } else {
    // FORCE LE ZÉRO : Si on est sous le seuil, on vide les mémoires
    // C'est ça qui tue le drift lent.
    targetX = 0;
    smoothX = 0;
    accumX = 0;
  }

  // 2. LISSAGE ET ACCUMULATION (uniquement si mouvement réel)
  if (targetY != 0) {
    smoothY = (targetY * (1.0 - smoothing)) + (smoothY * smoothing);
    accumY += smoothY;
  }

    // 2. LISSAGE ET ACCUMULATION (uniquement si mouvement réel)
  if (targetX != 0) {
    smoothX = (targetX * (1.0 - smoothing)) + (smoothX * smoothing);
    accumX += smoothX;
  }


  int mY = (int)accumY;
  int mX = (int)accumX;

  if( (mX != 0) || (mY != 0) )
  {
    Mouse.move(mX,mY);
    accumX -= mX; // On garde le reste
    accumY -= mY; // On garde le reste
  }

  
//   int mY = (int)accumY;

//   if (mY != 0) {
//     Mouse.move(0, mY);
//     accumY -= mY; // On garde le reste
//   }


//   int mX = (int)accumX;

//   if (mX != 0) {
//     Mouse.move(mX,0);
//     accumX -= mX; // On garde le reste
//   }

  // On ralentit un tout petit peu pour laisser le buffer USB respirer sous Manjaro
  if(k++>=10)
  {
    k=0;
    delay(1); 
  }
}