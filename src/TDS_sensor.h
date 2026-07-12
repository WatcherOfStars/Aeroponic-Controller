#define TdsSensorPin A1
#define VREF 3.3      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point

void setupTDS();
float getTDS();
int getMedianNum(int bArray[], int iFilterLen);