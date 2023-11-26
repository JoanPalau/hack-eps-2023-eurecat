#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// #include <DHT.h>

#define WIFI_MSG_BUFFER_SIZE (50)
#define SERIAL_BAUD_RATE 9600
#define NO_DATA_INPUT_INT -1

// #define DHTPIN D4     // Digital pin connected to the DHT sensor
// #define DHTTYPE DHT11 // DHT 11
// DHT dht(DHTPIN, DHTTYPE);

/*
 * This should not be hardcoded here, but it's a hackathon so...
 */
// WIFI INFORMATION
const char* ssid = "JMGS20FE";
const char* password = "pwsf9034";

// MQTT INFORMATION
const char* MQTT_SERVER_IP = "84.88.76.18";
const int MQTT_SERVER_PORT = 1883;

const char* clientID ="grupo_2";
const char* MQTT_CLIENT_NAME="grupo_2";
const char* MQTT_CLIENT_PSWD="grupo_2";

const char* MQTT_RECIEVE_TOPIC="hackeps/eurecat";
const char* MQTT_SEND_TOPIC="hackeps/G2";

// HTTP
const char* HTTP_SERVER_URL = "http://pondere.es:8004/save_data";

// OTHER
WiFiClient wifiClient;
PubSubClient client(wifiClient);
HTTPClient http;

struct farmDataStruct {
  String id;
  int light;
  int soil_humidity;
  int air_humidity;
  int temperature;
};

struct farmActionsStruct {
  String dhtmode;
  bool pump;
};

unsigned long lastMsg = 0;
char msg[WIFI_MSG_BUFFER_SIZE];
int value = 0;

// data
struct farmDataStruct farmData = {"Patata", -1, -1, -1, -1};

/*
 * Sets up the WiFI connection
 */
void wifi_setup();

/*
 * Sets up the base MQTT setup
 */
void mqtt_setup();

/*
 * Sets up the base HTTP setup
 */
void http_setup();

/*
 * Connects to the MQTT SERVER or loops though the main MQTT client loop
 */
void handle_mqtt();

/*
 * Subscribes to the MQTT TOPIC
 */
void connect_mqtt();

/*
 * Callback function that triggers further actions, when data is received
 */
void handle_received_data(char* topic, byte* payload, unsigned int length);

/*
 * Parses the payload and stores the info localy
 */
void parse_data(char* topic, byte* payload, unsigned int length);

/*
 * Merges farm data with plant data, then, publishes the new data
 */
void parse_actions_data(char* topic, byte* payload, unsigned int length);

/*
 * Uses HTTP to store data to the backend
 */
void store_data();

/*
 * Reads the information from the DHT sensor and stores it
 */
void read_dht();

/*
 * Reads the information from the Ground Humidity sensor and stores it
 */
void read_gh();

/*
 * Reads the information from the Light sensor and stores it
 */
void read_light();

/*
 * Activates / deactivates the water pump
 */
void control_water_pump(bool active);

// Serial port displays to show status of the connection process
void displayConnectingWifiMessage();
void displayStartConnectingWiFiMessage();
void displayConnectedWiFiMessage();

// START CODE
void setup() {
  // COMS
  Serial.begin(SERIAL_BAUD_RATE);
  wifi_setup();
  mqtt_setup();
  http_setup();

  // HARDWARE
  // dht.begin();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second

  handle_mqtt();
}

// COMS FUNCTIONS

void handle_mqtt()
{
  if (!client.connected())
  {
    connect_mqtt();
  } else {
    client.loop();
  }
}

// conecta o reconecta al MQTT
// OK -> suscribe a topic y publica un mensaje
// NO -> delay 5s
void connect_mqtt()
{
  // Serial.print("Starting MQTT connection...");
  if (client.connect(clientID, MQTT_CLIENT_NAME, MQTT_CLIENT_PSWD))
  {
      // Read data 
      client.subscribe(MQTT_RECIEVE_TOPIC);
  }
  else
  {
    // Print failure & delay
    Serial.print("Failed MQTT connection, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
  }
}



void handle_received_data(char* topic, byte* payload, unsigned int length)
{
  if(true) {
    parse_data(topic, payload, length);
    store_data();
  } else {

  }
}

void parse_data(char* topic, byte* payload, unsigned int length)
{
  //{"id": "P_03_HOME", "light": 99, "soil_humidity": 39, "air_humidity": 55, "temperature": 99}
  payload[length] = '\0';

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String content = String((const char *)payload);

  Serial.println(content);


  StaticJsonDocument<192> doc;
  DeserializationError error = deserializeJson(doc, content);
  if(error) {
     Serial.println(error.f_str());
     return;
  }

  farmData.id = String(doc["id"]);
  farmData.light = doc["light"];
  farmData.soil_humidity = doc["soil_humidity"];
  farmData.air_humidity = doc["air_humidity"];
  farmData.temperature = doc["temperature"];
}

void parse_actions_data(char* topic, byte* payload, unsigned int length) {

}

void store_data()
{
  StaticJsonDocument<192> doc;

  doc["id"] = farmData.id;
  doc["light"] =farmData.light;
  doc["soil_humidity"] = farmData.soil_humidity;
  doc["air_humidity"] = farmData.air_humidity;
  doc["temperature"] = farmData.temperature;

  http.begin(wifiClient, HTTP_SERVER_URL);

  http.addHeader("Content-Type", "application/json");

  String postMessage;
  serializeJson(doc, postMessage);
  int httpResponseCode = http.POST(postMessage);
}

// CONTROLER FUNCTIONS
void read_dht(){

}

void read_gh()
{

}

void read_light()
{

}

void control_water_pump(bool active) {

}

// SETUP FUNCTIONS

void wifi_setup()
{
  // We start by connecting to a WiFi network
  displayStartConnectingWiFiMessage();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    displayConnectingWifiMessage();
    delay(500);
  }
  displayConnectedWiFiMessage();
}

void mqtt_setup()
{
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(handle_received_data);
}

void http_setup()
{
  http.begin(wifiClient, HTTP_SERVER_URL);
}

// HELPER FUNCTIONS

void displayConnectingWifiMessage()
{
  Serial.print(".");
}

void displayStartConnectingWiFiMessage()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
}

void displayConnectedWiFiMessage()
{
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
