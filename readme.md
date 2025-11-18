# 📡 Projet Radar ESP32 avec Contrôle MQTT

Ce projet transforme un ESP32, un servomoteur et un capteur à ultrasons en un radar Wi-Fi. Il scanne son environnement et publie les données (angle et distance) en temps réel sur un broker MQTT.

Il peut également être contrôlé à distance via MQTT pour changer sa plage de balayage (par exemple, scanner uniquement entre 30 et 90 degrés).

## ✨ Fonctionnalités

- **Balayage Radar** : Un servomoteur effectue un balayage et un capteur à ultrasons mesure la distance.

- **Connectivité Wi-Fi** : Utilise WiFiManager pour une configuration facile du réseau Wi-Fi via un portail captif.

- **Publication MQTT** : Envoie les données (angle, distance) au format JSON vers un topic MQTT.

- **Contrôle à Distance** : Écoute un topic MQTT pour recevoir des commandes et ajuster la plage de balayage.

- **Reset Facile** : Permet d'effacer les identifiants Wi-Fi enregistrés en touchant simplement la broche T0 (GPIO 4) de l'ESP32.

## 🛠️ Matériel Requis

- Un ESP32 (Modèle WROOM-32 ou similaire).

- Un servomoteur (ex: SG90).

- Un capteur à ultrasons (ex: HC-SR04).

- Quelques fils de prototypage.

## 🔌 Câblage

Ce code utilise les broches suivantes (vous pouvez les modifier en haut du fichier .ino) :

|Composant | Broche du composant | Broche de l' ESP32 |
|:---------|:-------------------:|-------------------:|
|Servomoteur|Signal (Orange)|GPIO13|
||VCC (Rouge)|5V|
||GND(Marron)|GND|
|Capteur Ultrason|Trig|GPIO5|
||Echo|GPIO18|
||VCC|5V|
||GND|GND|

## 📦 Logiciel & Installation
### 1. Bibliothèques
Assurez-vous d'avoir installé les bibliothèques suivantes via le Gestionnaire de bibliothèques de l'Arduino IDE :

-    ESP32Servo

 -   WiFiManager

  -  MQTTClient

   - ArduinoJson
 ### 2. Configuration du Broker

Ce projet est pré-configuré pour utiliser le broker de test public :

- Adresse : broker.emqx.io

- Port : 1883

Vous pouvez changer ces valeurs dans le code si vous avez votre propre broker.

### 3. Téléversement

Téléversez le code sur votre ESP32. Ouvrez le Moniteur Série (vitesse 115200) pour voir les messages de statut.

## 🚀 Mode d'Emploi

1. Première Connexion (Configuration Wi-Fi)

Au premier démarrage (ou après un reset), l'ESP32 ne connaît pas votre réseau Wi-Fi.

   - L'ESP32 va créer un Point d'Accès Wi-Fi (AP).

   - Prenez votre téléphone ou ordinateur et cherchez les réseaux Wi-Fi.

   - Connectez-vous au réseau nommé :

       - SSID : espcestgay

        - Mot de passe : gay12345678

   - Une fois connecté, une page (portail captif) devrait s'ouvrir automatiquement.

   - Cliquez sur "Configure WiFi".

   - Sélectionnez votre réseau Wi-Fi domestique, entrez son mot de passe, et cliquez sur "Save".

   - L'ESP32 va redémarrer et se connecter à votre Wi-Fi et au broker MQTT. Vous verrez son adresse IP s'afficher dans le Moniteur Série.
  
  2. Réinitialiser le Wi-Fi

Si vous changez de réseau Wi-Fi ou de mot de passe, vous devez réinitialiser la configuration :

   - Pendant que l'ESP32 est allumé, touchez et maintenez la broche T0 (GPIO 4) pendant une seconde.

   - Le moniteur série affichera "Suppression des reglages...".

   - L'ESP32 redémarrera et le point d'accès espcestgay sera à nouveau visible. Reprenez à l'étape 1.
  
  ## 📡 Contrôle via MQTT (API)

  Utilisez un client MQTT (comme MQTTX, MQTT Explorer, etc.) pour interagir avec le radar.

1. Voir les Données du Radar (Publication)

L'ESP32 publie ses données sur ce topic. Abonnez-vous-y pour voir le flux de données.

   - Topic : gay/1

   - Payload (JSON) : Les messages ressembleront à ceci :
        ```json
        {
        "timestamp": 123456,
        "angle": 90,
        "distance": 34.12
        }
        ```

2. Contrôler la Plage de Balayage (Commande)

Vous pouvez envoyer un message sur ce topic pour dire au radar de scanner uniquement une zone précise.

 -   Topic : gay/1/setScan

  -  Payload (Texte Brut) : Le message doit être au format angle_debut-angle_fin.

       - Exemple : 30-90

       - Exemple : 0-45

        - Exemple : 120-180

    ⚠️ ATTENTION : Lorsque vous publiez ce message, assurez-vous que votre client MQTT est réglé sur Plaintext (Texte brut) et PAS sur JSON, sinon l'ESP32 ne comprendra pas la commande.