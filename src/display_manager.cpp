#include <string.h>
#include <display_manager.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Arduino.h>

//DISPLAY
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST); // Use dedicated hardware SPI pins

//struct for a display item on the screen
typedef struct display_item{
  char* tag;
  char* text;
  uint16_t color;
} DisplayItem;

DisplayItem display_arr[max_display_items];

//to be called on setup
void setupDisplay(){
    // turn on backlite
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH);

    // turn on the TFT / I2C power supply
    pinMode(TFT_I2C_POWER, OUTPUT);
    digitalWrite(TFT_I2C_POWER, HIGH);
    delay(10);

    // initialize TFT
    tft.init(135, 240); // Init ST7789 240x135
    tft.setRotation(3);
    tft.fillScreen(BLACK);
    tft.cp437(true);
}

//to be called whenever display needs updated
void manageDisplay(){
  tft.fillScreen(BLACK); //clear screen

  for (int i=0; i<5; i++){
    if(display_arr[i].tag != NULL){ //if exists, display this entry
      tft.setCursor(0, i*line_spacing);
      tft.setTextColor(display_arr[i].color);
      //char *chars = display_arr[i].text;
      tft.print(String(display_arr[i].tag) + " " + String(display_arr[i].text));
      continue;
    }
  } 
}

//add a message to the screen
void addText(char *tag, char *text, uint16_t color) {
  for (int i=0; i<5; i++){
    if(display_arr[i].tag == NULL || display_arr[i].tag == tag){ //if exists or is empty, update this entry
      DisplayItem new_item = {tag, text, color};  //create new struct
      display_arr[i] = new_item; //update value
      return;
    }
  }


}

//clear a topic from the message array
void clearText(char *tag){
  for (int i=0; i<5; i++){
    if(display_arr[i].tag == tag){ //if exists or is empty, delete this entry
      display_arr[i] = {NULL,NULL,BLACK}; //set to empty
      return;
    }
  }
}