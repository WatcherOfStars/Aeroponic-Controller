#include <Arduino.h>

static unsigned int line_spacing = 10;
static const unsigned int max_display_items = 10;

#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00

// External SPI TFT configuration.
// Update these values to match your display wiring.
#ifndef DISP_WIDTH
#define DISP_WIDTH 320
#endif

#ifndef DISP_HEIGHT
#define DISP_HEIGHT 240
#endif

#ifndef DISP_ROTATION
#define DISP_ROTATION 3
#endif

#ifndef DISP_SCK
#define DISP_SCK 21
#endif

#ifndef DISP_MOSI
#define DISP_MOSI 12
#endif

#ifndef DISP_MISO
#define DISP_MISO -1
#endif

#ifndef DISP_CS
#define DISP_CS 18
#endif

#ifndef DISP_DC
#define DISP_DC 6
#endif

#ifndef DISP_RST
#define DISP_RST 7
#endif

// Set to -1 if your display backlight is always-on or not MCU controlled.
#ifndef DISP_BL
#define DISP_BL -1
#endif

void setupDisplay();
void manageDisplay();
void addText(char *tag, char *text, uint16_t color);
void clearText(char *tag);