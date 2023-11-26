#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <DHT.h>

#define WIFI_MSG_BUFFER_SIZE (50)
#define SERIAL_BAUD_RATE 9600
#define NO_DATA_INPUT_INT -1

#define DHTTYPE DHT11 // DHT 11

// ANALOG INPUT PIN
#define ANALOG_IN A0

// LIGHT SYSTEM SWITCH PIN
#define LED_1 D5

// WATER PUMP SWITCH PIN
#define PUMP_PIN D1

// GH / LIGHT SWITCH PINS
#define GH_PIN D3
#define LIGHT_PIN D2

// TEMP / HUM INPUT PIN
#define TEMP_HUM_IN D6

DHT dht(TEMP_HUM_IN, DHTTYPE);

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

const char* MQTT_DATA_TOPIC="hackeps/eurecat";
const char* MQTT_ACTIONS_TOPIC="hackeps/G2";

// HTTP
const char* HTTP_SERVER_URL = "http://pondere.es:8004/save_data";

const int MIN_LIGHT = 640;
const int MAX_LIGHT = 910;

const int MAX_HUM = 550;
const int MIN_HUM = 500;

const int OUTPUT_MIN = 0;
const int OUTPUT_MAX = 100;

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
  bool day;
};

unsigned long lastMsg = 0;
char msg[WIFI_MSG_BUFFER_SIZE];
int value = 0;

// data
struct farmDataStruct farmData = {"GLOBAL", -1, -1, -1, -1};
struct farmActionsStruct farmActions = {"all", false, true};

// state
int lightState = 0;
int pumpState = 0;
struct farmDataStruct fieldData = {"NUTS", -1, -1, -1, -1};

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
void parse_data(byte* payload, unsigned int length);

/*
 * Merges farm data with plant data, then, publishes the new data
 */
void parse_actions_data(byte* payload, unsigned int length);

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
 * Select operative mode for hardware switch between GH and LIGHT sensor
 *  "all" -> read both
 *  "air" -> light only
 *  "soil" -> ground only
 */
void control_light_gh_readings(String mode);

/*
 * Activates / deactivates the light system
 */
void control_light_system(bool active);

/*
 * Activates / deactivates the water pump
 */
void control_water_pump(bool active);

// HELPERS
/*
 * Translate range value from A to B
 */
int map(int value, int start1, int end1, int start2, int end2);

/*
 * handles the serialization of the data to store FARM + FIELD 
 */
String serialize_data_to_store();

// Serial port displays to show status of the connection process
void displayConnectingWifiMessage();
void displayStartConnectingWiFiMessage();
void displayConnectedWiFiMessage();

// START CODE
void setup() {
  // DEBUG
  Serial.begin(SERIAL_BAUD_RATE);

  // HARDWARE
  dht.begin();
  pinMode(LED_1, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(GH_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

// COMS
  wifi_setup();
  mqtt_setup();
  http_setup();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second

  // READ DHT
  read_dht();

  // UPDATE HARDWARE STATUS
  control_light_system(!farmActions.day);
  control_light_gh_readings(farmActions.dhtmode);
  control_water_pump(farmActions.pump);

  // COMS
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
      client.subscribe(MQTT_DATA_TOPIC);
      client.subscribe(MQTT_ACTIONS_TOPIC);
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
    payload[length] = '\0';

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");

  if(strcmp(topic, MQTT_DATA_TOPIC) == 0) {
    parse_data(payload, length);
    store_data();
  } else if(strcmp(topic, MQTT_ACTIONS_TOPIC) == 0){
    parse_actions_data(payload, length);
  }
}

void parse_data(byte* payload, unsigned int length)
{
  //{"id": "P_03_HOME", "light": 99, "soil_humidity": 39, "air_humidity": 55, "temperature": 99}
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

void parse_actions_data(byte* payload, unsigned int length) {
//{"id": "P_03_HOME", "light": 99, "soil_humidity": 39, "air_humidity": 55, "temperature": 99}
  String content = String((const char *)payload);
  Serial.println(content);

  StaticJsonDocument<192> doc;
  DeserializationError error = deserializeJson(doc, content);
  if(error) {
     Serial.println(error.f_str());
     return;
  }

  String dhtmode_candidate = doc["dhtmode"];
  if(strcmp(dhtmode_candidate.c_str(), "-1") != 0) {
    Serial.println("Valid DHT_MODE candidate!");
    farmActions.dhtmode = dhtmode_candidate;
  }
  farmActions.day = doc["day"];
  farmActions.pump = doc["pump"];
}

void store_data()
{
  http.begin(wifiClient, HTTP_SERVER_URL);

  http.addHeader("Content-Type", "application/json");

  String postMessage = serialize_data_to_store();
  int httpResponseCode = http.POST(postMessage);
}

// CONTROLER FUNCTIONS
void read_dht()
{
  int h = (int)dht.readHumidity();
  Serial.println("AIR_HUM");
  
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    fieldData.air_humidity = h;
    Serial.println(fieldData.air_humidity);
  }

  int t = (int)dht.readTemperature();
  Serial.println("TEMP");
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    fieldData.temperature = t;
    Serial.println(fieldData.temperature);
  }
}

void read_gh()
{
  fieldData.soil_humidity = mapInversed(analogRead(ANALOG_IN), MIN_HUM, MAX_HUM, OUTPUT_MIN, OUTPUT_MAX);
  Serial.println("SOIL HUMIDITY");
  Serial.println(fieldData.soil_humidity);
}

void read_light()
{
  fieldData.light = map(analogRead(ANALOG_IN), MIN_LIGHT, MAX_LIGHT, OUTPUT_MIN, OUTPUT_MAX);
  Serial.println("SOIL LIGHT");
  Serial.println(fieldData.light);
}

void control_light_gh_readings(String mode)
{

  // IMPORTANT!: dalays are used to let the signal stabilize
  const char* parsed_mode = mode.c_str();
  if(strcmp(parsed_mode, "all") == 0) {
    Serial.println("I enter ALL");
    // setup correct ground read
    digitalWrite(GH_PIN, HIGH);
    digitalWrite(LIGHT_PIN, LOW);
    
    delay(100);
    read_gh();

    // setup correct light read
    digitalWrite(GH_PIN, LOW);
    digitalWrite(LIGHT_PIN, HIGH);
    delay(100);
    read_light();

  } else if(strcmp(parsed_mode, "soil") == 0) {
    Serial.println("I enter SOIL");
    digitalWrite(GH_PIN, HIGH);
    digitalWrite(LIGHT_PIN, LOW);
    delay(100);
    read_gh();

  } else if(strcmp(parsed_mode, "air") == 0) {
    Serial.println("I enter AIR");
    digitalWrite(GH_PIN, LOW);
    digitalWrite(LIGHT_PIN, HIGH);
    delay(100);
    read_light();
  }
}

void control_light_system(bool active)
{
  if (lightState == LOW && active) {
    // turn LED on
    lightState = HIGH;
    digitalWrite(LED_1, HIGH);
  } else if(lightState == HIGH && !active) {
    // turn LED off
    lightState = LOW;
    digitalWrite(LED_1, LOW);
  }
}

void control_water_pump(bool active) {
  if(active && pumpState == LOW) {
    pumpState = HIGH;
    digitalWrite(PUMP_PIN, HIGH);
  } else if(!active && pumpState == HIGH) {
    pumpState = LOW;
    digitalWrite(PUMP_PIN, LOW);
  }
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

// UTILS

int map(int value, int start1, int end1, int start2, int end2) {
  return start2 + (int)(((long long)(end2 - start2) * (value - start1)) / (end1 - start1));
}

int mapInversed(int value, int start1, int end1, int start2, int end2) {
  return start2 + (end2 - start2) * (1 - ((value - start1)) / (end1 - start1));
}

String serialize_data_to_store() 
{
  const size_t capacity = JSON_OBJECT_SIZE(7) + 1 * JSON_OBJECT_SIZE(6) + 300; // Adjust the capacity based on your data size
  StaticJsonDocument<capacity> doc;

  // Create a JsonObject to hold the storePayload data
  JsonObject payloadObj = doc.to<JsonObject>();

  // Create JsonObject for farm and fields
  JsonObject farmObj = payloadObj.createNestedObject("farm");
  farmObj["id"] = farmData.id;
  farmObj["light"] =farmData.light;
  farmObj["soil_humidity"] = farmData.soil_humidity;
  farmObj["air_humidity"] = farmData.air_humidity;
  farmObj["temperature"] = farmData.temperature;

  JsonArray fieldsArray = payloadObj.createNestedArray("fields");
  for (size_t i = 0; i < 1; ++i) {
      JsonObject fieldObj = fieldsArray.createNestedObject();
      fieldObj["id"] = fieldData.id;
      fieldObj["light"] = fieldData.light;
      fieldObj["soil_humidity"] = fieldData.soil_humidity;
      fieldObj["air_humidity"] = fieldData.air_humidity;
      Serial.println("SEND AIR H VALUE");
      Serial.println(fieldData.air_humidity);
      Serial.println("SEND AIR T VALUE");
      Serial.println(fieldData.temperature);
      fieldObj["temperature"] = fieldData.temperature;
  }

  // Serialize the JSON document
  String serializedPayload;
  serializeJson(doc, serializedPayload);

  return serializedPayload;
};

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
