#include <Arduino.h>
// On teste sur la broche Touch 1 (qui correspond souvent à GPIO 1 sur le S3, à vérifier selon ton modèle)
const int touchPin = 5; 
int valeurTouch = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Test du capteur tactile démarré !");
}

void loop() {
  
  valeurTouch = touchRead(touchPin); 
  
  Serial.print("Value Read : ");
  Serial.println(valeurTouch);
  
  // Si la valeur descend en dessous d'un certain seuil (ex: 30000 ou moins selon le S3), 
  // c'est que ton doigt est dessus.
  if (valeurTouch > 20000) { //2026-05-17 12:51:04 - for instance 17605 when touched
    Serial.println("-> Touching !! !");
  }else
  {
    Serial.println("Released");
  }
  
  delay(200);
}