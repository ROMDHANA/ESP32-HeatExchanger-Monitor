#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MAX31865.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Constantes de configuration
#define FORMAT_LITTLEFS_IF_FAILED true
#define FILE_EXTENSION_CSV ".csv"
#define CONFIG_FILE "/conf.json"
#define pt100_NUM 2 // Nombre de capteurs PT100

// Objets globaux
DNSServer dnsServer;
AsyncWebServer server(80);
AsyncEventSource events("/events"); 

// Initialisation des capteurs PT100
Adafruit_MAX31865 pt100_1 = Adafruit_MAX31865(4);  // Capteur PT100 connecté au pin 4
Adafruit_MAX31865 pt100_2 = Adafruit_MAX31865(5);  // Capteur PT100 connecté au pin 5

// Paramètres pour les capteurs PT100
const int RREF = 430;     // Valeur de la résistance de référence
const int RNOMINAL = 100; // Résistance nominale du PT100

// Documents JSON dynamiques pour la configuration et les données
DynamicJsonDocument CONFIG(1000);
DynamicJsonDocument DATA(1000);

unsigned long previousMillis = 0; // Pour le stockage du temps précédent

// Fonction pour lire la température d'un capteur PT100
float readTemperature(Adafruit_MAX31865 &sensor) {
    uint16_t rtd = sensor.readRTD();
    float ratio = rtd / 32768.0;
    float resistance = ratio * RREF;
    return sensor.temperature(RNOMINAL, RREF);
}

// Fonction pour lire la configuration à partir du système de fichiers
void readConfig() {
    File file = LittleFS.open(CONFIG_FILE);
    if (file) {
        deserializeJson(CONFIG, file);
        file.close();
    }
}

// Fonction pour sauvegarder la configuration dans le système de fichiers
void saveConfig() {
    File file = LittleFS.open(CONFIG_FILE, FILE_WRITE);
    serializeJson(CONFIG, file);
    file.close();
}

// Classe pour gérer les requêtes de type captive portal
class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {
        server.addHandler(&events);
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        return true;  // Peut gérer toutes les requêtes
    }

    void handleRequest(AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html; charset=UTF-8");
    }
};

// Fonction d'initialisation
void setup() {
    Serial.begin(115200);

    // Montage du système de fichiers LittleFS
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS Mount Failed, formatting...");
        if (!LittleFS.begin()) {
            Serial.println("LittleFS Mount Failed after formatting");
            return;
        }
    }

    // Lecture de la configuration depuis le système de fichiers
    readConfig();
    serializeJsonPretty(CONFIG, Serial);

    // Initialisation des capteurs PT100
    pt100_1.begin(MAX31865_2WIRE); // Mode 2 fils
    pt100_2.begin(MAX31865_2WIRE); // Mode 2 fils

    // Configuration du point d'accès Wi-Fi
    WiFi.softAP("Echangeur Halle");
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Ajout du gestionnaire de requêtes pour le serveur web
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // Filtre pour AP seulement

    // Démarrage du serveur web
    server.begin();
}

// Boucle principale
void loop() {
    dnsServer.processNextRequest();

    // Vérifie si le logging est activé dans la configuration
    if (CONFIG["log"].as<bool>()) {
        unsigned long currentMillis = millis();
        
        // Vérifie si l'intervalle de temps pour la prochaine lecture est écoulé
        if (currentMillis - previousMillis >= CONFIG["dt"].as<unsigned long>()) {
            previousMillis = currentMillis;  // Mise à jour de l'horodatage précédent

            // Lecture des températures et mise à jour des données
            DATA["T"][0] = readTemperature(pt100_1);
            DATA["T"][1] = readTemperature(pt100_2);
            DATA["fs"][0] = LittleFS.totalBytes();
            DATA["fs"][1] = LittleFS.usedBytes();
            DATA["t"] = currentMillis / 1000; // Horodatage en secondes

            // Sérialisation des données pour affichage dans la console
            serializeJsonPretty(DATA, Serial);

            // Sérialisation des données en JSON pour les envoyer via AsyncEventSource
            String json;
            serializeJson(DATA, json);
            events.send(json.c_str(), "new_readings", millis());

            // Gestion des fichiers de log
            String filename = "/logs";
            createDirectoryIfNotExists(filename);

            // Création du chemin de fichier basé sur millis() pour organiser les logs
            filename += "/" + String(currentMillis / (1000 * 60 * 60 * 24 * 365) + 1970); // Année
            createDirectoryIfNotExists(filename);
            
            filename += "/" + String((currentMillis / (1000 * 60 * 60 * 24) % 12) + 1); // Mois
            createDirectoryIfNotExists(filename);
            
            filename += "/" + String((currentMillis / (1000 * 60 * 60 * 24) % 31) + 1) + FILE_EXTENSION_CSV; // Jour
            createFileIfNotExists(filename, "Temps,Température");

            // Écriture des données dans le fichier de log
            File dataFile = LittleFS.open(filename, FILE_APPEND);
            if (dataFile) {
                dataFile.printf("%lu", currentMillis / 1000);
                for (size_t i = 0; i < pt100_NUM; i++) {
                    dataFile.printf(",%.1f", DATA["T"][i].as<float>());
                }
                dataFile.println();
                dataFile.close();
            }
        }
    }

    // Gestion des commandes reçues via le port série
    if (Serial.available()) {
        DynamicJsonDocument doc(300);
        String token = Serial.readStringUntil('\n');

        DeserializationError err = deserializeJson(doc, token);
        if (!err) {
            serializeJsonPretty(doc, Serial);
            JsonObject root = doc.as<JsonObject>();
            bool save_config = false;
            for (JsonPair kv : root) {
                if (kv.key() == "dir") {
                    DynamicJsonDocument dir_(1000);
                    JsonObject rootObject = dir_.to<JsonObject>();

                    listDir(LittleFS, kv.value().as<const char *>(), rootObject);
                    serializeJsonPretty(dir_, Serial);
                }
                if (CONFIG.containsKey(kv.key().c_str())) {
                    save_config = true;
                    CONFIG[kv.key().c_str()] = kv.value();
                }
            }
            if (save_config) {
                saveConfig();
            }
        }
    }
}

// Fonction pour créer un répertoire s'il n'existe pas
void createDirectoryIfNotExists(const String &path) {
    if (!LittleFS.exists(path)) {
        LittleFS.mkdir(path);
        Serial.println("Created directory: " + path);
    }
}

// Fonction pour créer un fichier avec un en-tête s'il n'existe pas
void createFileIfNotExists(const String &path, const String &header) {
    if (!LittleFS.exists(path)) {
        File file = LittleFS.open(path, FILE_WRITE);
        if (file) {
            file.println(header);
            file.close();
            Serial.println("Created file: " + path);
        }
    }
}

// Fonction pour lister les fichiers et répertoires dans un format JSON
void listDir(fs::FS &fs, const char *dirname, JsonObject &jsonObject) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        String fileName = String(file.name());
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(fileName);
            JsonObject subdirContents = jsonObject.createNestedObject(fileName);
            listDir(fs, file.path(), subdirContents);
        } else {
            Serial.print("  FILE: ");
            Serial.print(fileName);
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
            jsonObject[fileName] = file.size();
        }
        file = root.openNextFile();
    }
}
