#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// This function connects to the MQTT broker
void reconnect(PubSubClient mqtt_client, WiFiClient wifi_client) {
  // Set our MQTT broker address and port
  mqtt_client.setServer("192.168.0.115", 1883);
  mqtt_client.setClient(wifi_client);

  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.println("Attempt to connect to MQTT broker");
    mqtt_client.connect("esp8266_tenzo");

    // Wait some time to space out connection requests
    delay(3000);
  }
  Serial.println("MQTT connected");
}