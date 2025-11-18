#include <Arduino.h>
#include <ESP32Servo.h> 

// --- Pins ---
const int trigPin = 5;
const int echoPin = 18; 
const int servoPin = 13;  

// --- Servo ---
Servo elservo;

// --- Capteur ---
#define SOUND_SPEED 0.034
long duration;
float distanceCm;

// Prototype de fonction
float getDistance(); 

// =================================
// SETUP
// =================================
void setup() {
  Serial.begin(115200); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  elservo.attach(servoPin, 544, 2400); 
}

// =================================
// LOOP PRINCIPAL (Le Balayage)
// =================================
void loop() {
  // Balayage de 0 à 180 degrés
  for (int pos = 0; pos <= 180; pos += 1) { 
    elservo.write(pos); 
    distanceCm = getDistance(); 
    Serial.print("Angle = ");
    Serial.print(pos);
    Serial.print(", Distance = ");
    Serial.print(distanceCm);
    Serial.println(" cm");
    
    delay(10); 
  }

  // Balayage retour de 180 à 0 degrés
  for (int pos = 180; pos >= 0; pos -= 1) { 
    elservo.write(pos);
    distanceCm = getDistance();
    Serial.print("Angle = ");
    Serial.print(pos);
    Serial.print(", Distance = ");
    Serial.print(distanceCm);
    Serial.println(" cm");

    delay(10); 
  }
}

// =================================
// FONCTION DE MESURE DE DISTANCE
// =================================
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  return duration * SOUND_SPEED / 2;
}