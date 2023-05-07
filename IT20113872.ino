#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Servo.h>


#define WIFI_SSID "EDIZON"
#define WIFI_PASSWORD "qwer1234"

#define MQTT_HOST "139.162.50.185"
#define MQTT_PORT 1883


#define MQTT_PUB_TEMP "esp/ds18b20/temperature"
#define MQTT_PUB_HUMIDITY "esp/ds18b20/humidity"
#define MQTT_PUB_HI "esp/ds18b20/hi"

Servo servo1;  // create servo object

int pos = 0;   // variable to store servo position




// Data wire is plugged into pin D4 on the ESP8266
#define ONE_WIRE_BUS D3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


#define DHTPIN 2     // DHT11 pin
#define DHTTYPE DHT11  // DHT11 sensor type

DHT dht(DHTPIN, DHTTYPE);

// Variables to hold sensor readings
float temp;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings


void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}



void setup() {
  servo1.attach(D4);  // attach servo to pin 9
  Serial.begin(115200);
  Serial.println();  
  sensors.begin();
  dht.begin();
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials("addservername", "passwordserver");
  
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  // if (currentMillis - previousMillis >= interval) {
    // Save the last


  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New DHT sensor readings
    sensors.requestTemperatures(); 
    temp = sensors.getTempCByIndex(0);

    float temperatureCx = dht.readTemperature();
    float temperatureC = temp;
    float humidityx = dht.readHumidity();
    float humidity = 50;

    float HI = -42.379 + 2.04901523 * temperatureC + 10.14333127 * humidity - 0.22475541 * temperatureC * humidity - 0.00683783 * temperatureC * temperatureC - 0.05481717 * humidity * humidity + 0.00122874 * temperatureC * temperatureC * humidity + 0.00085282 * temperatureC * humidity * humidity - 0.00000199 * temperatureC * temperatureC * humidity * humidity;

    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print(" °C");
    Serial.print("\tHumidity: ");
    Serial.print(humidity);
    Serial.print(" %");
    Serial.print("\tHeat Index: ");
    Serial.print(HI);
    Serial.println(" F");    

    

    // Temperature in Celsius degrees
    
    // Temperature in Fahrenheit degrees
    //temp = sensors.getTempFByIndex(0);
    
    // Publish an MQTT message on topic esp32/ds18b20/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());   
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_TEMP);
    
    uint16_t packetIdPub12 = mqttClient.publish(MQTT_PUB_HI, 1, true, String(HI).c_str());     
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_HI);       
    
    uint16_t packetIdPub123 = mqttClient.publish(MQTT_PUB_HUMIDITY, 1, true, String(humidity).c_str());                                    
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_HUMIDITY);


    Serial.println(packetIdPub1);
    Serial.println(packetIdPub12);
    Serial.println(packetIdPub123);
    Serial.printf("Message: %.2f /n", sensors.getTempCByIndex(0)); 


    //Servo if  
    if (HI > 79 && HI < 91) {                   // increment pos variable
      servo1.write(0);        // set servo position
      delay(5);
                  // wait for 50ms before next step
    }    

    if (HI > 89 && HI < 104) {                   // increment pos variable
      servo1.write(70);        // set servo position
      delay(10);    
      servo1.write(0);            // wait for 50ms before next step
    }    

    if (HI > 104) {                   // increment pos variable
      servo1.write(140);        // set servo position
      delay(10);  
      servo1.write(0);              // wait for 50ms before next step
    }    
    
    if (HI > 80 & HI < 90) {                   // increment pos variable
      servo1.write(180);        // set servo position
      delay(10);      
      servo1.write(0);          // wait for 50ms before next step
    }        
    
  }  
}