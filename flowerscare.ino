/*
  Flowerscare - based on BLE_server.ino from Arduino-ESP32 examples

    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLE2904.h>

#define SERVICE_UUID    "fbf6184a-1c89-11ea-afc9-4b26e3dea594"
#define CHAR_SOIL1_UUID "281a29ac-1c8a-11ea-b76e-4fb27025b3da"
#define CHAR_SOIL2_UUID "2890fa50-1c8a-11ea-a92d-d305201750e1"
#define CHAR_LIGHT_UUID "28e815f6-1c8a-11ea-8251-43e51596d8ab"
#define CHAR_VOLTS_UUID "293fa622-1c8a-11ea-a43a-57b4c400c11d"

BLEServer *pServer;

BLEService *pBattService;
BLECharacteristic *pCharBatteryLevel;

BLEService *pService;
BLECharacteristic *pCharDeviceName;
BLECharacteristic *pCharSoil1;
BLECharacteristic *pCharLight;
BLECharacteristic *pCharSoil2;
BLECharacteristic *pCharVolts;

#define PIN_SOIL1 33
#define PIN_SOIL2 35
#define PIN_LIGHT 36
#define PIN_BATTERY 34

bool deviceConnected = false;
bool oldDeviceConnected = false;
String name = "Flowerscare";

uint32_t last_notify = 0;
uint16_t notify_interval = 15*1000;

uint32_t last_print = 0;
uint16_t print_interval = 5*1000;

struct BLE2906_Range {
  uint16_t min;
  uint16_t max;
} __attribute__((packed));


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Flowerscare setup");

  // 
  // Create server
  // 
  BLEDevice::init(name.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 
  // Create service: Battery Level
  // 
  Serial.println("Create battery service");
  pBattService = pServer->createService(BLEUUID((uint16_t) 0x180F),15,0);
  Serial.println("Create battery level characteristic");
  pCharBatteryLevel = pBattService->createCharacteristic(
    BLEUUID((uint16_t) 0x2a19),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE);

  // 
  // Create service: Custom (flowerscare)
  // 
  Serial.println("Create flowerscare service");
  pService = pServer->createService(BLEUUID(SERVICE_UUID), 64,1);
  Serial.println("Create characteristics");

  // 
  // Create characteristic: name
  // 
  pCharDeviceName = pService->createCharacteristic(
    BLEUUID((uint16_t) 0x2a00),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE);
  pCharDeviceName->setValue(name.c_str());
  BLE2904 *pDescFormat = new BLE2904();
  pDescFormat->setFormat(BLE2904::FORMAT_UTF8);
  pCharDeviceName->addDescriptor(pDescFormat);

  // 
  // Create characteristic: soil moisture level (capacitive)
  // 
  pCharSoil1 = pService->createCharacteristic(
    CHAR_SOIL1_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE);

  BLEDescriptor *pDescSoil1 = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  pDescSoil1->setValue("Soil moisture level");
  pCharSoil1->addDescriptor(pDescSoil1);
  pCharSoil1->addDescriptor(new BLE2902());
  pCharSoil1->addDescriptor(new BLEDescriptor(BLEUUID((uint16_t) 0x290C)));
  pDescFormat = new BLE2904();
  pDescFormat->setFormat(BLE2904::FORMAT_UINT16);
  pCharSoil1->addDescriptor(pDescFormat);

  BLEDescriptor *pDescRange = new BLEDescriptor(BLEUUID((uint16_t) 0x2906));
  struct BLE2906_Range range = {0,4095};
  pDescRange->setValue((uint8_t *)&range, sizeof(range));
  pCharSoil1->addDescriptor(pDescRange);


  // 
  // Create characteristic: soil moisture level (resistive)
  // 
  pCharSoil2 = pService->createCharacteristic(
    CHAR_SOIL2_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE);

  BLEDescriptor *pDescSoil2 = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  pDescSoil2->setValue("Soil moisture level (resistive)");
  pCharSoil2->addDescriptor(pDescSoil2);
  pCharSoil2->addDescriptor(new BLE2902());
  pCharSoil2->addDescriptor(new BLEDescriptor(BLEUUID((uint16_t) 0x290C)));
  pDescFormat = new BLE2904();
  pDescFormat->setFormat(BLE2904::FORMAT_UINT16);
  pCharSoil2->addDescriptor(pDescFormat);

  pDescRange = new BLEDescriptor(BLEUUID((uint16_t) 0x2906));
  pDescRange->setValue((uint8_t *)&range, sizeof(range));
  pCharSoil2->addDescriptor(pDescRange);

  // 
  // Create characteristic: light level
  // 
  pCharLight = pService->createCharacteristic(
    CHAR_LIGHT_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE);

  BLEDescriptor *pDescLight = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  pDescLight->setValue("Light level");
  pCharLight->addDescriptor(pDescLight);
  pCharLight->addDescriptor(new BLE2902());
  pCharLight->addDescriptor(new BLEDescriptor(BLEUUID((uint16_t) 0x290C)));
  pDescFormat = new BLE2904();
  pDescFormat->setFormat(BLE2904::FORMAT_UINT16);
  pCharLight->addDescriptor(pDescFormat);

  pDescRange = new BLEDescriptor(BLEUUID((uint16_t) 0x2906));
  pDescRange->setValue((uint8_t *)&range, sizeof(range));
  pCharLight->addDescriptor(pDescRange);

  // 
  // Create characteristic: battery voltage
  // 
  pCharVolts = pService->createCharacteristic(
    CHAR_VOLTS_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE);

  BLEDescriptor *pDescVolts = new BLEDescriptor(BLEUUID((uint16_t) 0x2901));
  pDescVolts->setValue("Battery voltage");
  pCharVolts->addDescriptor(pDescVolts);
  pCharVolts->addDescriptor(new BLE2902());
  pDescFormat = new BLE2904();
  pDescFormat->setFormat(BLE2904::FORMAT_UINT16);
  pDescFormat->setExponent(-3);
  pCharVolts->addDescriptor(pDescFormat);

  pDescRange = new BLEDescriptor(BLEUUID((uint16_t) 0x2906));
  range.min = 2500;
  range.max = 4200;
  pDescRange->setValue((uint8_t *)&range, sizeof(range));
  pCharVolts->addDescriptor(pDescRange);

  // 
  // Set initial values for characteristics
  // 
  Serial.println("Set initial values");
  uint16_t zero = 0;
  pCharSoil1->setValue(zero);
  pCharSoil2->setValue(zero);
  pCharLight->setValue(zero);
  pCharVolts->setValue(zero);
  
  // 
  // Start services
  // 
  pBattService->start();
  pService->start();

  // 
  // Initiate BLE advertising
  // 
  Serial.println("Begin Advertising");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->addServiceUUID(BLEUUID((uint16_t) 0x180F));
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  // 
  // Set up IO pins
  // 
  Serial.println("Set pin modes");
  
  pinMode(PIN_SOIL1, INPUT);
  pinMode(PIN_SOIL2, INPUT);
  pinMode(PIN_LIGHT, INPUT);
  pinMode(PIN_BATTERY, INPUT);
  
  Serial.println("Setup complete!");
}

void loop() {
  // 
  // Read input pins
  // 
  uint16_t soil1 = 4095-analogRead(PIN_SOIL1);
  uint16_t soil2 = analogRead(PIN_SOIL2);
  uint16_t light = analogRead(PIN_LIGHT);
  int  batt = analogRead(PIN_BATTERY);
  uint8_t battLevel = map(batt, 0, 4095, 0, 100);
  uint16_t millivolts = map(batt, 0, 4095, 0, 6600);
  uint32_t now = millis();

  // 
  // Update characterirstics
  // 
  pCharBatteryLevel->setValue(&battLevel,1);
  pCharSoil1->setValue(soil1);
  pCharSoil2->setValue(soil2);
  pCharLight->setValue(light);
  pCharVolts->setValue(millivolts);

  if (( now >= (last_print+print_interval) ) 
    ) {
    Serial.printf("%ssoil1=%d soil2=%d light=%d batt=%d millivolts=%d level=%d\n",
		  deviceConnected?"C ":"  ",
		  (int)soil1, (int)soil2, (int)light, batt, (int)millivolts, (int)battlLevel);
    last_print = now;
  }

  // 
  // Handle connection state changes
  // 
  if (!deviceConnected && oldDeviceConnected) {
    // we have just disconnected since the previous loop
    Serial.println("Device disconnected.  Restart advertising");Serial.flush();
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }
  else if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    Serial.println("Device connected"); Serial.flush();
    oldDeviceConnected = deviceConnected;
  }

  // 
  // Send notifications
  //
  if ( deviceConnected &&
       ( now >= (last_notify+notify_interval) ) 
    ) {
    // We are currently connected
    // fixme: only do this if notifications are turned on 
    Serial.println("Send notifications");Serial.flush();
    pCharBattery->notify();
    pCharSoil1->notify();
    pCharSoil2->notify();
    pCharLight->notify();
    pCharVolts->notify();
    last_notify = now;
  }

  
  delay(100);
}

