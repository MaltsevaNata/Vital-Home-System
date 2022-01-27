#include <ArduinoJson.h>
#include <HX711.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

#include "filter.h"

#define DT  5                     // DT output of sensor HX711 (GPIO5)
#define SCK 4                    // SCK output of sensor HX711 (GPIO4)
#define CONTROL 2                // servo control pin (GPIO2)
 
HX711 scale;
WiFiClient WIFI_CLIENT;
PubSubClient MQTT_CLIENT;   
StaticJsonDocument<200> jsondoc;
Servo servo;

float units = 0.035274;          // koef for unit conversion from ounce to gramms
float calibration_factor = 15.45;    // tenzo sensor calibration factor
float weight_units;
float weight_gr;
float filtered_weight;
int time_read = 0;
bool started_feeding = false;

void setup()
{
  Serial.begin(9600);           

  // Attempt to connect to a specific access point
  WiFi.begin("wifi_name", "wifi_pass");

  // Keep checking the connection status until it is connected
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  // Print the IP address of your module
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  scale.begin(DT, SCK);          // initialize HX711
  scale.set_scale();             
  scale.tare();   // reset the weight
  
  /* КАЛИБРОВКА
  scale.set_scale();             // Не калибруем полученные значения 
  scale.tare();                  // Обнуляем вес на весах (тарируем)
  Serial.println("You have 10 seconds to set your known load");    // Выводим в монитор порта текст 
  delay(10000);                                                    // Ждём 10 секунд
  Serial.print("calibration factor: ");                            // Выводим текст в монитор последовательного порта
  calibration_factor = scale.get_units(10) / (weight_of_standard / units);      // Считываем значение с тензодатчика
  Serial.println(calibration_factor);                                           // Выводим в монитор порта значение корректирующего коэффициента
  */
  calibration_factor = 15.45; // got by experiment calibration

  servo.attach(CONTROL, 640, 2600); //servo pin and min/max values
  servo.write(0); // default poistion

}

// This function handles received messages
void myMessageArrived(char* topic, byte* payload, unsigned int length) {
  // Convert the message payload from bytes to a string
  String message = "";
  for (unsigned int i=0; i< length; i++) {
    message = message + (char)payload[i];
  }
   
  // Print the message to the serial port
  Serial.println(message);

  if (message == "cat") {
    started_feeding = true;
  }
}


// This function connects to the MQTT broker
void reconnect() {
  // Set our MQTT broker address and port
  MQTT_CLIENT.setServer("192.168.0.115", 1883);
  MQTT_CLIENT.setClient(WIFI_CLIENT);

  // Loop until we're reconnected
  while (!MQTT_CLIENT.connected()) {
    Serial.println("Attempt to connect to MQTT broker");
    MQTT_CLIENT.connect("esp8266_tenzo");

    // Wait some time to space out connection requests
    delay(3000);
  }
  Serial.println("MQTT connected");
  // Subscribe to the topic where our web page is publishing messages
  MQTT_CLIENT.subscribe("control/feed");

  // Set the message received callback
  MQTT_CLIENT.setCallback(myMessageArrived);
}

void open_slider() {
  servo.write(45); //ставим вал под 0
}

void close_slider() {
  servo.write(0); //ставим вал под 0
}


void loop() {
  // Check if we're connected to the MQTT broker
  if (!MQTT_CLIENT.connected()) {
    // If we're not, attempt to reconnect
    reconnect();
  }

  if (started_feeding ) {
    if (filtered_weight < 20) { // if not enough food 
      open_slider();
    }
    else {
      close_slider();
      started_feeding = false;
    }
  }
  
  scale.set_scale(calibration_factor);
  // get weight in ounces
  weight_units = scale.get_units(), 10;

  if (weight_units < 0) {
    weight_units = 0.00;
  }

  // ounces to gramms
  weight_gr = weight_units * units;

  // perform filtration
  filtered_weight = findMedianN(weight_gr);
  
  time_read = time_read + 1;

  if (time_read >= 50) { // once in 500 ms send weight data
    Serial.print("weight_gr =");
    Serial.print(filtered_weight);
    Serial.println(" gr");
 
    jsondoc["weight"] = filtered_weight;
    char out[128];
    serializeJson(jsondoc, out);

    // Publish a message to a topic
    MQTT_CLIENT.publish("feeder/info",  out);

    time_read = 0;
  }
  MQTT_CLIENT.loop();
  delay(10);
}

