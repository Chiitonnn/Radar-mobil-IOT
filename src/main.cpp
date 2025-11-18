#include <Arduino.h>
#include <ESP32Servo.h> 
#include <WiFiManager.h>   
#include <MQTTClient.h>
#include <WiFi.h>           
#include <ArduinoJson.h>

// --- wifimanager ---
WiFiManager wm; 
const char* ssid = "espcestgay";
const char* password = "gay12345678"; 

// --- MQTT ---
const char MQTT_BROKER_ADRRESS[] = "broker.emqx.io"; 
const int MQTT_PORT = 1883;
const char MQTT_CLIENT_ID[] = "gayradar-esp32-001"; 
const char MQTT_USERNAME[] = "";                      
const char MQTT_PASSWORD[] = "";   
const char PUBLISH_TOPIC[] = "gay/1";
const char SUBSCRIBE_TOPIC[] = "gay/1/setScan";
const char TOPIC_DISCOVER[] = "gay/1/discover";
const char TOPIC_REGISTER[] = "gay/1/register";

MQTTClient mqtt = MQTTClient(512);
WiFiClient network;

// --- Pins ---
const int trigPin = 5;
const int echoPin = 18; 
const int servoPin = 13;  

// --- Servo ---
Servo elservo;
int scanStart = 0; 
int scanEnd = 180;

// --- Capteur ---
#define SOUND_SPEED 0.034
long duration;
float distanceCm;

// --- Machine à états pour le radar ---
enum RadarState {
  STATE_SCANNING,
  STATE_TRACKING
};
RadarState currentState = STATE_SCANNING; 
int trackingAngle = 90;
const float TRACKING_DISTANCE_CM = 10.0;

// Prototype de fonction
float getDistance(); 
void connectToMQTT();
void sendToMQTT(int angle, float distance); 
void messageHandler(String &topic, String &payload); 
void performScan();    
void performTracking();

// =================================
// SETUP
// =================================
void setup() {
  WiFi.mode(WIFI_STA); 

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n"); 
  if(!wm.autoConnect(ssid, password))  
    Serial.println("Erreur de connexion."); 
  else
    Serial.println("Connexion etablie !"); 
  
  pinMode(trigPin, OUTPUT);// Corrigé de "/n" à "\n"
  pinMode(echoPin, INPUT);
  elservo.attach(servoPin, 544, 2400); 

  connectToMQTT();
}

// ===============
// LOOP PRINCIPAL
// ===============
void loop() {
  if(touchRead(T0) < 60) {                            
      Serial.println("Suppression des reglages et redemarrage ...");
      wm.resetSettings();  
      ESP.restart(); 
  }

  mqtt.loop(); 

  // --- Le "cerveau" de la machine à états ---
  if (currentState == STATE_SCANNING) {
    performScan();
  } else if (currentState == STATE_TRACKING) {
    performTracking();
  }
}

// =================================
// Fonction pour le balayage
// =================================
void performScan() {
  for (int pos = scanStart; pos <= scanEnd; pos += 1) {
    elservo.write(pos); 
    distanceCm = getDistance(); 
    sendToMQTT(pos, distanceCm);
    if (distanceCm < TRACKING_DISTANCE_CM) {
      Serial.println("Objet detecte! Passage en mode POURSUITE.");
      trackingAngle = pos; 
      currentState = STATE_TRACKING; 
      return; 
    }
    
    delay(10); 
  }

  for (int pos = scanEnd; pos >= scanStart; pos -= 1) {
    elservo.write(pos);
    distanceCm = getDistance();
    sendToMQTT(pos, distanceCm);

    // Condition pour passer en mode poursuite
    if (distanceCm < TRACKING_DISTANCE_CM) {
      Serial.println("Objet detecte! Passage en mode POURSUITE.");
      trackingAngle = pos; // On mémorise l'angle
      currentState = STATE_TRACKING; 
      return; 
    }
    
    delay(10); 
  }
}

// =================================
// Fonction pour la poursuite
// =================================
void performTracking() {
  elservo.write(trackingAngle);
  distanceCm = getDistance();
  sendToMQTT(trackingAngle, distanceCm); 
  if (distanceCm >= TRACKING_DISTANCE_CM) {
    Serial.println("Objet perdu. Reprise du BALAYAGE.");
    currentState = STATE_SCANNING; 
  }
  delay(100); 
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

// =================================
// FONCTIONS MQTT
// =================================
void connectToMQTT() {
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);
  mqtt.onMessage(messageHandler); 

  Serial.print("ESP32 - Connexion au broker MQTT");

  while (!mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  if (!mqtt.connected()) {
    Serial.println("ESP32 - Timeout broker MQTT !");
    return;
  }

  if (mqtt.subscribe(SUBSCRIBE_TOPIC))
    Serial.print("ESP32 - Souscrit au topic: ");
  else
    Serial.print("ESP32 - Echec souscription au topic: ");
  Serial.println(SUBSCRIBE_TOPIC); 

  // Souscription au topic de découverte
  if (mqtt.subscribe(TOPIC_DISCOVER))
    Serial.print("ESP32 - Souscrit au topic: ");
  else
    Serial.print("ESP32 - Echec souscription au topic: ");
  Serial.println(TOPIC_DISCOVER); 

  Serial.println("ESP32 - Broker MQTT Connecte !");
}

void sendToMQTT(int angle, float distance) { 
  StaticJsonDocument<200> message;
  message["timestamp"] = millis();
  message["angle"] = angle;        
  message["distance"] = distance;  
  // <-- MODIFIÉ : Ajout du mode actuel dans le JSON
  message["mode"] = (currentState == STATE_SCANNING) ? "scanning" : "tracking"; 
  
  char messageBuffer[512];
  serializeJson(message, messageBuffer);

  mqtt.publish(PUBLISH_TOPIC, messageBuffer);
}

// =================================
// HANDLER DE MESSAGE (Le Cerveau)
// =================================
void messageHandler(String &topic, String &payload) {
  Serial.println("ESP32 - recu de MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload: " + payload);

  // --- Votre logique de découverte (conservée) ---
  if (topic == TOPIC_DISCOVER) {
    Serial.println("🔍 Reponse a la demande de decouverte");
    
    // Utiliser ArduinoJson est plus propre pour créer le JSON
    StaticJsonDocument<200> doc;
    doc["deviceId"] = MQTT_CLIENT_ID;
    doc["type"] = "ESP32_Radar_Servo_Ultrason";
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["timestamp"] = millis();
    
    char response[256];
    serializeJson(doc, response);
    
    mqtt.publish(TOPIC_REGISTER, response);
    Serial.println("✅ Reponse decouverte envoyee sur gay/1/register");
    Serial.println(response);
    
  }
  
  // --- Votre logique de commande (modifiée) ---
  else if (topic == SUBSCRIBE_TOPIC) {
    int separatorIndex = payload.indexOf('-'); 

    if (separatorIndex == -1) {
      Serial.println("Erreur: Format de message inconnu (attendu: 'start-end')");
      return;
    }
    String startStr = payload.substring(0, separatorIndex);
    String endStr = payload.substring(separatorIndex + 1);
    int newStart = startStr.toInt();
    int newEnd = endStr.toInt();

    if (newStart < 0 || newEnd > 180 || newStart >= newEnd) {
      Serial.println("Erreur: Angles invalides. Doivent être 0-180 et start < end.");
      return; 
    }

    scanStart = newStart;
    scanEnd = newEnd;

    // --- MODIFIÉ : Forcer le retour au balayage ---
    currentState = STATE_SCANNING; 

    Serial.print("Scan range mis a jour: ");
    Serial.print(scanStart);
    Serial.print(" a ");
    Serial.println(scanEnd);
    Serial.println("Forcage du mode BALAYAGE.");
  }
}