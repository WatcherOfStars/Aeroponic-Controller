#include <Adafruit_AHTX0.h>

#include <OneWireNg.h>

//#include <Time.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include <SPI.h>

#include <display_manager.h>
#include <network_manager.h>

//==============MESSAGING VARS==============
const int message_time = 5; //time in seconds between messages
float last_message_time = 0;


//==============AEROPONICS VARS==============
const int FLOW_PIN = 5; //for solenoid control

float flow_on_time = 15; //seconds
float flow_off_time = 300; //seconds should be 300

enum FlowState {ON, OFF};
FlowState flow_state;
float last_change_time = 0;

//==============SENSOR PINS==============
Adafruit_AHTX0 aht; //humidity and temp


void setup() 
{
    Serial.begin(9600);

    setupDisplay();

    //aeroponics setup
    pinMode(FLOW_PIN, OUTPUT);
    
    if (! aht.begin()) {
        Serial.println("Could not find AHT? Check wiring");
        while (1) delay(10);
    }
    Serial.println("AHT10 or AHT20 found");

    flow_state = OFF;
    last_change_time = millis()/1000.0;
    last_message_time = millis()/1000.0;

    //wifi setup
    //setup_wifi();
    

}

void manageSystem(){
  float current_time = millis()/1000.0;

  //manage flow from pressure tank to grow tower
  switch(flow_state){
    case ON: //in open mode
      digitalWrite(FLOW_PIN, HIGH); //open valve
      if(current_time-last_change_time >= flow_on_time){ //switch states if needed
        Serial.println("VALVE_CLOSED");
        addText("VALVE_STATE", "VALVE_CLOSED", RED);
        last_change_time = current_time;
        flow_state = OFF;
      }
      break;
    case OFF: //in closed mode
      digitalWrite(FLOW_PIN, LOW); //close valve
      if(current_time-last_change_time >= flow_off_time){ //switch states if needed
        Serial.println("VALVE_OPEN");
        addText("VALVE_STATE", "VALVE_OPEN", GREEN);
        last_change_time = current_time;
        flow_state = ON;
      }
      break;
  }

  //read humidity and air temperature
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  char humidity_str[32];
  dtostrf(humidity.relative_humidity, 7, 2, humidity_str);
  addText("HUMIDITY", humidity_str, BLUE);
  Serial.println("Humidity: " + String(humidity.relative_humidity));
  char air_temp_str[32];
  dtostrf(temp.temperature, 7, 2, air_temp_str);
  addText("AIR_TEMP", air_temp_str, BLUE);
  Serial.println("Air Temp: " + String(temp.temperature));

  manageDisplay();
}

void publishTemperatureData()
{
  //set up json and add timestamp
  char buffer[10];
  char message[256];
  JsonDocument doc;

  doc["time"] = millis()/60000.0;

  //add device data
  doc["user"] = DEVICE_USER;
  doc["device_name"] = DEVICE_NAME;

    //get sensor vals
    sensors_event_t humidity, temp;

    //refesh humidity (rH) and temperature (celcius)
    aht.getEvent(&humidity, &temp);

    //add sensor data
    doc["type"] = "'temperature'";
    doc["value"] = temp.temperature;

    //send the data
    serializeJson(doc,message);
    Serial.println(message);
    publishToMQTT(TOPIC, message);
}


void publishHumidityData()
{
    //set up json and add timestamp
    char buffer[10];
    char message[256];
    JsonDocument doc;

    doc["time"] = millis()/60000.0;

    //add device data
    doc["user"] = DEVICE_USER;
    doc["device_name"] = DEVICE_NAME;

    //get sensor vals
    sensors_event_t humidity, temp;

    //refesh humidity (rH) and temperature (celcius)
    aht.getEvent(&humidity, &temp);

    //add sensor data
    doc["type"] = "'humidity'";
    doc["value"] = humidity.relative_humidity;

    //send the data
    serializeJson(doc,message);
    Serial.println(message);
    publishToMQTT(TOPIC, message);
}

void publishValveData()
{
  //set up json and add timestamp
  char buffer[10];
  char message[256];
  JsonDocument doc;

  doc["time"] = millis()/60000.0;

  //add device data
  doc["user"] = DEVICE_USER;
  doc["device_name"] = DEVICE_NAME;

    //add sensor data
    doc["type"] = "'valve_state'";
    doc["value"] = 1;
    if(flow_state == OFF) doc["value"] = 0;

    //send the data
    serializeJson(doc,message);
    Serial.println(message);
    publishToMQTT(TOPIC, message);
}

void loop() 
{
    //update_wifi();

    float current_time = millis()/1000.0;

    //publish data if it is time
    if(current_time-last_message_time >= message_time){
      last_message_time = current_time;
      Serial.println("PUBLISHING");
      Serial.println("Time: " + String(current_time));
      Serial.println("State: " + flow_state);
      //publishWaterTemperatureData();
      //publishTemperatureData();
      //publishHumidityData();
      //publishValveData();
    }

    manageSystem();

    delay(200);
}


