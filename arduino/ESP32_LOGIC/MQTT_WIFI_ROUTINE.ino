#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HttpClient.h>

/*
 * Calls wifi_setup and mqtt_setup
 */
void coms_start();

/*
 * Sets up the WiFI connection
 */
void wifi_setup();

/*
 * Sets up the base MQTT setup
 */
void mqtt_setup();

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
 * Uses HTTP to store data to the backend
 */
void store_data();

/*
 * Merges farm data with plant data, then, publishes the new data
 */
void merge_send_data();

// Serial port displays to show status of the connection process
void displayConnectingWifiMessage();
void displayStartConnectingWiFiMessage();
void displayConnectedWiFiMessage();


/**
 * This should not be hardcoded here, but it's a hackathon so...
 */
// WIFI INFORMATION
const char* ssid = "JMGS20FE";
const char* password = "pwsf9034";

// MQTT INFORMATION
const char* MQTT_SERVER_IP = "192.168.38.84";
const int MQTT_SERVER_PORT = 1883;

const char* clientID ="grupo_2";
const char* MQTT_CLIENT_NAME="grupo_2";
const char* MQTT_CLIENT_PSWD="grupo_2";

const char* MQTT_RECIEVE_TOPIC="hackeps/eurecat";
const char* MQTT_SEND_TOPIC="hackeps/G2";

// HTTP
const char* HTTP_SERVER_URL = "http://X.X.X.X/savedata";

// OTHER
WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http;

unsigned long lastMsg = 0;
char msg[WIFI_MSG_BUFFER_SIZE];
int value = 0;

/**
 * Defined in ESP32_LOGIC
 * Starts WiFI connection
 * Recieves data from hackeps/eurecat topic
 * Publishes data to hackeps/G2
 */
void ComsProcess(void *pvParameters)
{
  // Initialize the local variables here
  (void) pvParameters;
  coms_start();
  // Main routine loop
  for(;;)
  {
    handleMqtt();
    vTaskDelay(10);
  }
}

void coms_start()
{
  wifi_setup();
  mqtt_setup();
}

void wifi_setup()
{
  vTaskDelay(10);
  // We start by connecting to a WiFi network
  displayStartConnectingWiFiMessage();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    displayConnectingWifiMessage();
    vTaskDelay(500);
  }
  displayConnectedWiFiMessage();
}

void mqtt_setup()
{
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(handle_received_data);
}

void handle_mqtt()
{
  if (!client.connected())
  {
    connectMqtt();
  }
  client.loop();
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
      client.subscribe("hello/world");
  }
  else
  {
    // Print failure & delay
    xSemaphoreTake(xSerialPort, TICKS_TO_WAIT_SERIAL);
    Serial.print("Failed MQTT connection, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    xSemaphoreGive(xSerialPort);
    vTaskDelay(5000);
  }
}

void handle_received_data(char* topic, byte* payload, unsigned int length)
{
  parse_data(topic, payload, length);
  store_data();
  merge_send_data();
}

void parse_data(char* topic, byte* payload, unsigned int length)
{
  //{"id": "P_03_HOME", "light": 99, "soil_humidity": 39, "air_humidity": 55, "temperature": 99}
  xSemaphoreTake(xSerialPort, TICKS_TO_WAIT_SERIAL);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  xSemaphoreGive(xSerialPort);

  String content = "";
  for(size_t i = 0; i < len; i++)
  {
    content.concat(data[i]);
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, content);
  if(error) return;

  xSemaphoreTake(xFarmData, TICKS_TO_WAIT_FARM);

  farmData.id = doc["id"];
  farmData.light = doc["light"];
  farmData.soil_humidity = doc["soil_humidity"];
  farmData.air_humidity = doc["air_humidity"];
  farmData.temperature = doc["temperature"];

  xSemaphoreGive(xFarmData);
}

void store_data()
{
  const int capacity = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<capacity> doc;

  xSemaphoreTake(xFarmData, TICKS_TO_WAIT_FARM);

  doc["id"] = farmData.id;
  doc["light"] =farmData.light;
  doc["soil_humidity"] = farmData.soil_humidity;
  doc["air_humidity"] = farmData.air_humidity;
  doc["temperature"] = farmData.temperature;

  xSemaphoreGive(xFarmData);
  http.begin(HTTP_SERVER_URL);

  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(doc);
}


void merge_send_data(byte* payload, unsigned int length)
{
  // Merge sensor data and send it to the topic
  const int capacity = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<capacity> doc;

  xSemaphoreTake(xFarmData, TICKS_TO_WAIT_FARM);
  doc["id"] = farmData.id;
  doc["light"] =farmData.light;
  doc["soil_humidity"] = farmData.soil_humidity;
  doc["air_humidity"] = farmData.air_humidity;
  doc["temperature"] = farmData.temperature;
  xSemaphoreGive(xFarmData);

  // Send the data;
  client.publish(doc, MQTT_SEND_TOPIC);
}

void displayConnectingWifiMessage()
{
  xSemaphoreTake(xSerialPort, TICKS_TO_WAIT_SERIAL);
  Serial.print(".");
  xSemaphoreGive(xSerialPort);
}

void displayStartConnectingWiFiMessage()
{
  xSemaphoreTake(xSerialPort, TICKS_TO_WAIT_SERIAL);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  xSemaphoreGive(xSerialPort);
}

void displayConnectedWiFiMessage()
{
  xSemaphoreTake(xSerialPort, TICKS_TO_WAIT_SERIAL);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  xSemaphoreGive(xSerialPort);
}
