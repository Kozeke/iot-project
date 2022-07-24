//WIFI and ADC2 Channels are not working together hence using ADC1 channels

#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include <Stepper.h>

#define ThermistorPin 35
#define LDR_PIN       33
#define SOIL_MOISTURE_PIN 34


#define R1  10000
#define C1  (float)1.009249522e-03
#define C2  (float)2.378405444e-04
#define C3  (float)2.019202697e-07

// Update these with values suitable for your network.
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "industrial.api.ubidots.com";
const int stepsPerRevolution = 200;
String prev_temp = "";
String prev_ldr = "";
String prev_soil = "";
#define mqtt_port 1883
#define MQTT_USER "BBFF-8jMRx8MB8JNxxZuMGLJ8fSocMFRHGe"
#define MQTT_PASSWORD ""
#define MQTT_SERIAL_PUBLISH_LIGHT "/v1.6/devices/garden/light"
#define MQTT_SERIAL_PUBLISH_TEMPERATUE "/v1.6/devices/garden/temp"
#define MQTT_SERIAL_PUBLISH_SOIL "/v1.6/devices/garden/soil"
#define MQTT_SERIAL_RECEIVER_MOTOR "/v1.6/devices/garden/motor"
#define MQTT_SERIAL_RECEIVER_MOTOR_STATUS "/v1.6/devices/garden/mstatus"

bool mstatus = 0;

Stepper myStepper(stepsPerRevolution, 23, 22, 21, 19);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

float avg[3]={0,0,0};


void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("/icircuit/presence/ESP32/", "hello world");
      // ... and resubscribe
      //client.subscribe(MQTT_SERIAL_RECEIVER_MOTOR);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);

    JsonVariant answer = doc["value"];
    auto value = answer.as<int>();      // value = 1 or 0
    Serial.println("-------new message from broker-----");
    Serial.print("topic:");
    Serial.println(topic);
    Serial.print("Motor direction:");  
    Serial.println(value);
    Serial.println();
    if(value == 0)
    {
      myStepper.step(-stepsPerRevolution);
      delay(500);
      mstatus = 1;
    }
    else if(value == 1)
    {
      myStepper.step(stepsPerRevolution);
      delay(500);
      mstatus = 0;
    }
}

void publishSerialData(const char *pub_str,const char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(pub_str, serialData);
}


void setup() {
  Serial.begin(115200);

  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  myStepper.setSpeed(60);
  client.subscribe(MQTT_SERIAL_RECEIVER_MOTOR);
}

//NTC Temparature Reading and Processing
float getTemperature(void)
{
  static int avgArrayIndex=0;
  int Vo;
  float logR2,R2, T, Tc;
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (4096.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (C1 + C2*logR2 + C3*logR2*logR2*logR2));
  Tc = T - 273.15;
  avg[avgArrayIndex++] = Tc;
  if(avgArrayIndex > 2)
  avgArrayIndex = 0;
  Tc = (avg[0] + avg[1] + avg[2])/3;
  return Tc;
}


float getMoisturePercentage(void)
{
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(SOIL_MOISTURE_PIN);
  moisture_percentage = ( 100 - ( (sensor_analog/4096.00) * 100 ) );
  moisture_percentage = (float)moisture_percentage;
  return moisture_percentage;
}

float getLightPercentage(void)
{
  int ldrRawVal;
  float percentage;
  ldrRawVal = analogRead(LDR_PIN);    
  percentage = ((float)((ldrRawVal*100)/4096));
//  percentage = 100 - percentage;
  return percentage;
}

void loop() {
  float lightpercentage = getLightPercentage();
  float temp = getTemperature();
  float soilMoisturePer = getMoisturePercentage();
  String temp_s(temp);
  String soilPer_s(soilMoisturePer);
  String lightPer_s(lightpercentage);

  String mstatus_s(mstatus);

  if(soilMoisturePer > 80.00)
  {
    myStepper.step(stepsPerRevolution);
    delay(500);
  }
 

  if (temp_s != prev_temp || lightPer_s != prev_ldr || soilPer_s != prev_soil) {

    DynamicJsonDocument jBuffer(1024);
    jBuffer["temperature"] = temp_s.c_str();
    jBuffer["LDR"] = lightPer_s.c_str();
    jBuffer["Moisture"] = soilPer_s.c_str();
    jBuffer["Motor"] = mstatus;

    publishSerialData(MQTT_SERIAL_RECEIVER_MOTOR_STATUS,mstatus_s.c_str());  
    delay(2500);
    publishSerialData(MQTT_SERIAL_PUBLISH_SOIL,soilPer_s.c_str());
    delay(250);
    publishSerialData(MQTT_SERIAL_PUBLISH_TEMPERATUE,temp_s.c_str());
    delay(250);
    publishSerialData(MQTT_SERIAL_PUBLISH_LIGHT,lightPer_s.c_str());
    delay(250);
    client.loop();
    Serial.print("Temperature: "); 
    Serial.print(temp_s.c_str());
    Serial.print(" C, LDR: "); 
    Serial.print(lightPer_s.c_str()); 
    Serial.print("%");
    Serial.print(" Moisture Percentage = ");
    Serial.print(soilPer_s.c_str());
    Serial.println("%"); 
    prev_temp = temp_s;
    prev_ldr = lightPer_s;
    prev_soil = soilPer_s;

  } else {
    Serial.println("No change");
    delay(500);
  }

}