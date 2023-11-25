#define WIFI_MSG_BUFFER_SIZE (50)
#define PRIORITY_CORE 0
#define SERIAL_BAUD_RATE 115200

#define NO_DATA_INPUT_INT -1

#define TICKS_TO_WAIT_FARM 500
#define TICKS_TO_WAIT_SENSOR 500
#define TICKS_TO_WAIT_WATER 500

struct farmDataStruct {
  char id;
  int light;
  int soil_humidity;
  int air_humidity;
  int temperature;
};

// Controls access to the MagicSensor data variable
SemaphoreHandle_t xMagicSensorData;
// TODO: REVIEW DATATYPE
int data = NO_DATA_INPUT_INT; // Initial value when no data is feeded

SemaphoreHandle_t xFarmData;
struct farmDataStruct farmData = {"", -1, -1, -1, -1};

// Controls access to the Water pump
SemaphoreHandle_t xWaterPump;

// Controls access to the Serial Port to display data
SemaphoreHandle_t xSerialPort

void initializeGlobalVariables()
{
  xMagicSensorData = xSemaphoreCreateBinary();
  xFarmData = xSemaphoreCreateBinary();
  xWaterPump = xSemaphoreCreateBinary();
  xSerialPort = xSemaphoreCreateBinary();
  // free the "mutex" to allow tasks to start using it
  xSemaphoreGive(xMagicSensorData);
  xSemaphoreGive(xFarmData);
  xSemaphoreGive(xWaterPump);
  xSemaphoreGive(xSerialPort);
}

void setupTasks(){
  // MARGARIDA && TULIPA: Wifi setup, read data, publish data
  xTaskCreatePinnedToCore(
    ComsProcess,
    "WIFI + Send and recieve/send data from/to the MQTT server",
    6144,
    NULL,
    1,
    NULL,
    1       // We execute this task in a separate core
  );

  // TULIPA: Read data from sensor
  xTaskCreatePinnedToCore(
    ReadSensorData,
    "Send plant data to the MQTT server",
    4096,
    NULL,
    1,
    NULL,
    0
  );

  // ALVOCAT: Be water my friend
  xTaskCreatePinnedToCore(
    WaterPump,
    "Send plant data to the MQTT server",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  // Here we define the tasks/routines that will be implemented
  initializeGlobalVariables();
  setupTasks();
}

// Ignore main loop as we are using FreeRTOS
void loop() {}
