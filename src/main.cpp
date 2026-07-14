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
#include <pressure_transducer.h>

//==============MESSAGING VARS==============
const int message_time = 5; //time in seconds between messages
float last_message_time = 0;


//==============AEROPONICS VARS==============
const int FLOW_PIN = 5; //for solenoid control
const int PUMP_CTL_PIN = 10; //to control inflow pumps

float flow_on_time = 15; //seconds
float flow_off_time = 300; //seconds should be 300

float shutoff_pressure = 82; //psi, pressure at which to shut off pumps
float pressure_hysteresis = 5; //psi, pressure below shutoff_pressure at which to turn pumps back on


enum FlowState {ON, OFF};
FlowState flow_state;
float last_change_time = 0;

bool pumps_on = false;

CQRobotTDS tds(17, 5.0); //TDS sensor on pin 18, 5V reference
PressureTransducer pressure_sensor(16); //pressure sensor on pin 19
WaterTempSensor water_temp_sensor(6); //water temp sensor on pin 6

//==============SENSOR PINS==============
Adafruit_AHTX0 aht; //humidity and temp


void setup() 
{
    Serial.begin(9600);

    setupDisplay();

    //aeroponics setup
    pinMode(FLOW_PIN, OUTPUT);
    pinMode(PUMP_CTL_PIN, OUTPUT);
    
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

  //manage inflow from reservoir to pressure tank
  if(pressure_sensor.readPressure() >= shutoff_pressure){ //if pressure high, turn off pumps
    pumps_on = false;
    Serial.println("Pumps Off");
    addText("PUMP_STATE", "PUMPS_OFF", RED);
  }
  else if(pressure_sensor.readPressure() <= shutoff_pressure-pressure_hysteresis){ //if pressure low and buffer time past, turn on pumps
    pumps_on = true;
    Serial.println("Pumps On");
    addText("PUMP_STATE", "PUMPS_ON", GREEN);
  }

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

  //read temp
  double temperature = water_temp_sensor.readTemperature();
  char wtr_temp[32];
  dtostrf(temperature, 7, 2, wtr_temp);
  addText("WATER_TEMP", wtr_temp, BLUE);

  //read TDS
  double TDS_val = tds.update(25); //25 degree placeholder
  char tds_str[32];
  dtostrf(TDS_val, 7, 2, tds_str);
  addText("TDS", tds_str, BLUE);

  //read pressure
  double pressure = pressure_sensor.readPressure();
  char pressure_str[32];
  dtostrf(pressure, 7, 2, pressure_str);
  addText("PRESSURE", pressure_str, BLUE);

  //read humidity and air temperature
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  char humidity_str[32];
  dtostrf(humidity.relative_humidity, 7, 2, humidity_str);
  addText("HUMIDITY", humidity_str, BLUE);
  char air_temp_str[32];
  dtostrf(temp.temperature, 7, 2, air_temp_str);
  addText("AIR_TEMP", air_temp_str, BLUE);

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


