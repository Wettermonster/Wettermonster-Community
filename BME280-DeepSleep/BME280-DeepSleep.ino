/*Wettermonster-Community BME280-DeepSleep
  von Christian H.

   Dieser Sketch ist Bestandteil des Projektes „Wettermonster“.
   Eine Anleitung und alle weiteren Informationen finden Sie unter https://wettermonster.de.

   Dieses Material steht unter der Creative-Commons-Lizenz Namensnennung-Nicht kommerziell 4.0 International.
   Um eine Kopie dieser Lizenz zu sehen, besuchen Sie http://creativecommons.org/licenses/by-nc/4.0/.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

Adafruit_BME280 bme;


const char* ssid = "NAME DEINES WLAN";
const char* password = "PASSWORD VOM WLAN";
const char* id = "STATIONSNUMMER VON WETTERMONSTER";
const char* key = "KEY DER STATION VON WETTERMONSTER";
const int interval = 2;

float temperature;
float humidity;
float pressure;
float Percipitation;
float numClicksRain;
float windSpeed;
float numRevsAnemometer;
char* windDirection;
float luminosity;
volatile unsigned long previousTimeRain = 0, previousTimeSpeed = 0, delayTime = 20;
unsigned long lastMillis;

WiFiClient client;

void sendToWettermonster() {

  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i <= 5)
  {
    i++;
    Serial.println("WiFi nicht verbunden. Versuche neu zu verbinden...");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(1000);
  }

  if (i > 5)
  {
    Serial.println("Verbindnung zu " + String(ssid) + " fehlgeschlagen. Neustart.");
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED && client.connect("upload.wettermonster.de", 80)) // Verbindung zum Server aufbauen
  {

    Serial.println ("Verbunden mit upload.wettermonster.de");
    client.print("GET /speichern.php");
    client.print("?id=");
    client.print(id);
    client.print("&schluessel=");
    client.print(key);
    client.print("&temperatur=");
    client.print(temperature);
    client.print("&luftfeuchtigkeit=");
    client.print(humidity);
    client.print("&luftdruck=");
    client.print(pressure);
    client.print("&niederschlag=");
    client.print(Percipitation);
    client.print("&windgeschwindigkeit=");
    client.print(windSpeed);
    client.print("&windrichtung=");
    client.print(windDirection);
    client.print("&helligkeit=");
    client.print(luminosity);
    client.println(" HTTP/1.1");
    client.println("Host: upload.wettermonster.de");
    client.println("User-Agent: Wettermonster");
    client.println("Accept: text/html");
    client.println();

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Timeout !");
        client.stop();
      }
    }

    Serial.println("Daten an Wettermonster gesendet");
  }

  else
  {
    Serial.println("Verbindung fehlgeschlagen");
  }

  client.stop();


}

void readBME() {

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;

}
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Verbinde mit " + String(ssid));

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Wifi Aktiviert");
  Serial.println("");
  Serial.print("Verbunden mit: ");
  Serial.println(ssid);
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());

  if (!bme.begin(0x76)) {
    Serial.println("BME280 konnte nicht gefunden werde, checke bitte die Verbindungen!");
    return;

    lastMillis = millis();
  }

  if (millis() - lastMillis > (interval * 60000)) {
  }
  if (ESP.getFreeHeap() <= 20000) {
    Serial.println("Der freie Heap beträgt nur noch: " + String(ESP.getFreeHeap()) + " Der ESP wird deshalb neu gestartet.");
    ESP.restart();
  }

  delay(100);


  readBME();
  bme.begin(0x76);
  sendToWettermonster();
  lastMillis = millis();
  Serial.println("Going into deep sleep for 1500 seconds");
  ESP.deepSleep(1500e6); // 20e6 is 20 microseconds
  WiFi.begin();
}

void loop() {
}

