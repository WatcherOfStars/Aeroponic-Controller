#define SensorPin A3            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 20
#define ArrayLenth  40    //times of collection

void setupPH();
void updatePH();
float getPH();
double avergearray(int* arr, int number);