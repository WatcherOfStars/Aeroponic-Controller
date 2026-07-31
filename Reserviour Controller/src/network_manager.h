//wifi connection data
#define WLAN_SSID         "plantphone"//wifi network ssid
#define WLAN_PASS         "growing23"//wifi password

//buffers
#define MQTT_TX_BUFSIZE   1024
#define MQTT_RX_BUFSIZE   1024

// ====================================================== //"10.1.99.175"
#define HOST              "192.168.137.70"
#define PORT              1883
#define CLIENT_ID       "AutoponicsV2"
// ======================================================

#define TOPIC            "AUTOPONICS_DEVICES"

//DEVICE DATA
#define DEVICE_USER       "Autoponics Prototype V3"
#define DEVICE_NAME       "Temp_Humid_Test"

void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void update_wifi();
void reconnect();
void publishToMQTT(char* topic, char* message);