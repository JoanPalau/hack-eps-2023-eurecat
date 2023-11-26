# hack-eps-2023-eurecat

# PURPOSE AND MOTIVATION

The main purpose was to have fun and end a journey which for some, started 7 years ago. With this project we wanted to feel challanged and try our bests in order to produce a unique system  that allowed to monitor and help plants grow.
An additional motivation was to choose this topic was to further explore the Arduino world which for some of our members was quite a challanging topic.
Furthermore, we think this systems can be implemented and further used on the real world in order to increase farmers profitability or help countries optimize their food production.

# SYSTEM REQUISITES
- ESP8266
- Awesome Sensor Toolkit
- A plant :)
- WiFi
- PC with basic dev tools such as Python and Arduino IDE

# Backend 
Application made with flask, htmx and postgresql that allows users to:
1. Interact with the hardware system
2. Visualize streaming data
2. Visualize real time graphs on multiple dashboards
3. Explore the raw data 
4. Compare averages and real time data

# mqtt
An script that suscribes to eurecat mqtt server and topic and saves into the
database the received data.

## topics
- **hackeps/eurecat:** streaming data provided by Eurecat
- **hackeps/G2:** topic used to comunicate from the APP to the Hardware asyncronously

# arduino
ESP8266 based hardware system that is capable of:

1. Subscribe to the hackeps/eurecat topic
2. Subscribe to the hackeps/G2 topic
3. Send aggregated data to the backend through http
4. Interact with a Water Pump, simulating a spray motion
5. Interact with the light sensor and the ground humidity sensors at the same time
6. Open a light when its nightime
7. Obtain environment temperature and humidity

# dataset
Contains the following files:

1. Original dataset
2. Notebook with the data science corrections and predictions 
3. Clean Dataset correcting the data
4. "Weather forecast" for the next 14 days 

# alexa

**"Alexa, abre el p\**\* Arduino"**

Alexa skill that allows you to:

1. Know the latest data available for your field
    
    "Alexa, ¿cual es la última lectura?"
2. Water an specific field

    "Alexa, riega los **tomates**"
