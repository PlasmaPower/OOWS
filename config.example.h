#define ARDUINO_NAME String("Name")
#define CC3000_SHIELD
#define NETWORK "Network_Name"
#define APIKEY String("0123456789ABCDEF")
#define CUSTOM_DATA_SERVER_IP cc3000.IP2U32(12, 34, 12, 34)
#define CUSTOM_DATA_SERVER_PASSWORD String("1234")
#define SENSORS {new DHT22Sensor(2)}
#define OUTPUTS {new SerialOutput()}
