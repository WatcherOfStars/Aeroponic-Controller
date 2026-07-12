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

void setupDisplay();
void manageDisplay();
void addText(char *tag, char *text, uint16_t color);
void clearText(char *tag);