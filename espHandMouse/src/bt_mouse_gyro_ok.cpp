#include <Arduino.h>
#include <Wire.h>
#include <FastIMU.h> // La lib principale

//#include <USBHIDMouse.h>
#define USE_NIMBLE // <--- TRÈS IMPORTANT : À mettre AVANT l'include !
#include <BleMouse.h>

//2026-05-17 13:13:42 - Sensor definition
BMI160 IMU; 
#define USE_BLE 1

#if USE_BLE == 1
  typedef BleMouse TypeMouse;
  // On donne un nom à ta souris. C'est ce nom qui apparaîtra sur ton PC sous Manjaro
  BleMouse Mouse("Souris Gyro ESP32", "Hobbyist", 100);
#endif


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

  //2026-05-17 13:14:38 - USB Hid 
  // Mouse.begin();
  // USB.begin();

  //2026-05-17 13:14:44 - BT hid
  Serial.println("Démarrage du Bluetooth...");  
  // On lance le service Bluetooth de l'ESP32
  Mouse.begin();

  
  Serial.println("FastIMU prête ! Calibrage automatique...");
  // Optionnel : tu peux ajouter une routine de calibrage ici
}//setup




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

void loopMouse(TypeMouse Mouse)
{

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
}//loopMouse



void loop() 
{

  #if USE_BLE == 1
    if (Mouse.isConnected()) 
    {
  #endif


    loopMouse(Mouse);
  
    #if USE_BLE == 1
      delay(2); //BleMouse config
    #else
        // On ralentit un tout petit peu pour laisser le buffer USB respirer sous Manjaro
      if(k++>=5)
      {
        k=0;
        delay(1); 
      }
    #endif

  #if USE_BLE == 1
    }//mouseConnected
    else {
      // Si l'ordinateur n'est pas connecté ou s'est déconnecté
      Serial.println("En attente de connexion Bluetooth avec le PC...");
      delay(1000); 
    }
  #endif
}//loop

