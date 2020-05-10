//DHT 11

#include <DHT.h> //DHT Library for DHT11
#include <InfluxDb.h>
#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Connect and publish to the MQTT broker

int DHTpin = 2; //DHTPin
int analogSensorPin = A0; //AnalogPin
float light_level = 0; //int for light
float moisture_value= 0; //int for moisture

#define DHTTYPE DHT11 //Type of DHT Sensor
#define INFLUXDB_HOST "10.0.0.16:8086"
Influxdb influx(INFLUXDB_HOST);
DHT dht(DHTpin, DHTTYPE); //Refer to DHT as dht here on out with DHTpin and Type noted

// MQTT
const char* mqtt_server = "10.0.0.16";  // IP of the MQTT broker
const char* humidity_topic = "grow/bedroom/all/humidity";
const char* temperature_topic = "grow/bedroom/all/temperature";
const char* lightlevel_topic = "grow/bedroom/all/lightLevel";
const char* plsoilmoisture_topic = "grow/bedroom/peacelilly/moisture";
const char* mqtt_username = "echo"; // MQTT username
const char* mqtt_password = "echo"; // MQTT password
const char* clientID = "growmon"; // MQTT client ID

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient); 

// WiFi
char* ssid = "House Stark";                 // Your personal network SSID
const char* wifi_password = "Th1515H0u535t@rk"; // Your personal network password

// Custom function to connet to the MQTT broker via WiFi
void connect_MQTT(){
  Serial.print("Linking connection to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("Link established");
  Serial.print("Link ID: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Broker Session Established");
  }
  else {
    Serial.println("Broker Failed");
  }
}

void setup(){
  dht.begin(); //start DHT
  Serial.begin(9600); //serial comms open on Comm Port 5, 9600 baud rate
  pinMode(D5, OUTPUT); //sets pin to output mode (Digital pin)for photoresistor
  pinMode(D6, OUTPUT); //sets pin to output mode (Digital pin)for moisture sensor
  digitalWrite(D5, LOW); //sets pin to Low voltage (off) for photoresistor
  digitalWrite(D6, LOW); //sets pin to Low voltage (off) for moisture sensor
}

void loop(){
    connect_MQTT();
    Serial.setTimeout(5000);
    delay(5000);
    float h = dht.readHumidity(); //read the humidity and save it
    float t = (dht.readTemperature() * 1.8) + 32;  //read the temp and save it       
    Serial.print("Current humidity = "); //print humidity
    Serial.print(h); //print humidity
    Serial.print("%  "); //print humidity
    Serial.print("temperature = "); //print temp
    Serial.print(t); //convert temp to farenheit
    Serial.println("F  "); //print temp
    // MQTT can only transmit strings
    String hs="Hum: "+String((float)h)+" % ";
    String ts="Temp: "+String((float)t)+" C ";
    digitalWrite(D5, HIGH); //sets pin to High voltage (on) for photoresistor
    delay(10000); //wait 10 milliseconds to give sensor time to read
    Serial.print("LIGHT LEVEL : "); //print light level
    light_level= analogRead(analogSensorPin); //read light level
    Serial.println(light_level); //print light level
    // MQTT can only transmit strings
    String ll="Hum: "+String((float)light_level);
    digitalWrite(D5, LOW); //sets pin to Low voltage (off) for photoresistor
    digitalWrite(D6, HIGH); //sets pin to High voltage (on) for moisture sensor
    delay(10000); //wait 10 milliseconds to give sensor time to read
    Serial.print("MOISTURE LEVEL : "); //print moisture level
    moisture_value= analogRead(analogSensorPin); //read moisture level
    Serial.println(moisture_value); //print moisture level
    String ms="Hum: "+String((float)moisture_value);
    digitalWrite(D6, LOW); //sets pin to Low voltage (off) for moisture sensor

     // PUBLISH to the MQTT Broker (topic = Temperature, defined at the beginning)
  if (client.publish(temperature_topic, String(t).c_str())) {
    Serial.println("Temperature sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(temperature_topic, String(t).c_str());
  }

  // PUBLISH to the MQTT Broker (topic = Humidity, defined at the beginning)
  if (client.publish(humidity_topic, String(h).c_str())) {
    Serial.println("Humidity sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Humidity failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(humidity_topic, String(h).c_str());
  }

  // PUBLISH to the MQTT Broker (topic = Light, defined at the beginning)
  if (client.publish(lightlevel_topic, String(light_level).c_str())) {
    Serial.println("Light Level sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Light Level failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(lightlevel_topic, String(light_level).c_str());
  }

  // PUBLISH to the MQTT Broker (topic = PLMoisture, defined at the beginning)
  if (client.publish(plsoilmoisture_topic, String(moisture_value).c_str())) {
    Serial.println("Peace Lilly Moisture sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Peace Lilly Moisture value failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(plsoilmoisture_topic, String(moisture_value).c_str());
  }

  delay(5000);       // print new values every 1 Minute
}
