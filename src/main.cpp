#include <Adafruit_AHTX0.h>

#include <OneWire.h>

//#include <Time.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include <SPI.h>

#include <display_manager.h>
#include <water_temp_sensor.h>
#include <network_manager.h>
#include <TDS_sensor.h>
#include <EC_sensor.h>
#include <pH_sensor.h>

//==============MESSAGING VARS==============
const int message_time = 5; //time in seconds between messages
float last_message_time = 0;


//==============AEROPONICS VARS==============
const int FLOW_PIN = 6; //for solenoid control
const int PRESSURE_SW_PIN = 13; //for inflow pressure switch
const int PUMP_CTL_PIN = 10; //to control inflow pumps

float flow_on_time = 15; //seconds
float flow_off_time = 30; //seconds should be 300

float pressure_buffer_time = 5; //pump for 5 extra seconds to ensure above 60 psi

enum FlowState {ON, OFF};
FlowState flow_state;
float last_change_time = 0;

float last_pressure_trigger = 0;
bool pumps_on = false;

//==============SENSOR PINS==============
Adafruit_AHTX0 aht; //humidity and temp
const int TDS_PIN = A5;
const int EC_PIN = A4;
const int pH_PIN = A3;


void setup() 
{
    Serial.begin(9600);

    setupDisplay();

    //aeroponics setup
    pinMode(FLOW_PIN, OUTPUT);
    pinMode(PRESSURE_SW_PIN, INPUT);
    pinMode(PUMP_CTL_PIN, OUTPUT);


    //sensor setup
    pinMode(TDS_PIN, INPUT);
    pinMode(EC_PIN, INPUT);
    pinMode(pH_PIN, INPUT);

    setupTempSensor();
    setupPH();
    setupTDS();
    
    // if (! aht.begin()) {
    //     Serial.println("Could not find AHT? Check wiring");
    //     while (1) delay(10);
    // }
    // Serial.println("AHT10 or AHT20 found");

    flow_state = OFF;
    last_change_time = millis()/1000.0;
    last_message_time = millis()/1000.0;
    last_pressure_trigger = millis()/1000.0;

    //wifi setup
    setup_wifi();
    

}

void loop() 
{
    update_wifi();

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
      publishValveData();
    }

    manageSystem();

    delay(200);
}

void manageSystem(){
  float current_time = millis()/1000.0;
  updatePH();

  // Serial.println("Pressure Switch: " + str(digitalRead(PRESSURE_SW_PIN)));

  // //manage pressure pumps
  // if(digitalRead(PRESSURE_SW_PIN)){ //if pressure low
  //   pumps_on = true;
  //   last_pressure_trigger = current_time;
  //   Serial.println("Pumps Off");
  // }
  // else if(current_time - last_pressure_trigger >= pressure_buffer_time){ //if pressure high and buffer time past
  //   pumps_on = false;
  //   Serial.println("Pumps On");
  // }
  
  // digitalWrite(PUMP_CTL_PIN, pumps_on);

  //manage flow
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

  //read temp
  double temperature = getTemp();
  char wtr_temp[32];
  dtostrf(temperature, 7, 2, wtr_temp);
  addText("WATER_TEMP", wtr_temp, BLUE);

  //read TDS
  double TDS = getTDS();
  char tds[32];
  dtostrf(TDS, 7, 2, tds);
  addText("TDS", tds, BLUE);

  //read pH
  double pH = getPH();
  char pH_c[32];
  dtostrf(pH, 7, 2, pH_c);
  addText("pH", pH_c, BLUE);

  manageDisplay();
}





void publishTemperatureData()
{
  //set up json and add timestamp
  char buffer[10];
  char message[256];
  StaticJsonDocument<400> doc;

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
    StaticJsonDocument<400> doc;

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
  StaticJsonDocument<400> doc;

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


