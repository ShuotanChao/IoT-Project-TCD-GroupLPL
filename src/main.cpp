#include <NimBLEDevice.h>

static NimBLERemoteCharacteristic *pRemoteCharacteristic;
static bool doConnect = false;
static bool connected = false;
static bool doScan = false;
static NimBLEAdvertisedDevice *myDevice;

static const char *SERVICE_UUID = "3a4e2ff2-c9fb-11ed-afa1-0242ac120002";
static const char *CHARACTERISTIC_UUID = "7214fb32-c9fb-11ed-afa1-0242ac120002";

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
  void onResult(NimBLEAdvertisedDevice *advertisedDevice)
  {
    if (advertisedDevice->getName() == "Receiver")
    {
      NimBLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      doScan = true;
    }
  }
};

void onScanEnd(NimBLEScanResults results)
{
  if (!doConnect)
  {
    doScan = true;
  }
}

bool connectToServer()
{
  NimBLEClient *pClient = NimBLEDevice::createClient();
  pClient->connect(myDevice);

  NimBLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr)
  {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr)
  {
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canWrite())
  {
    connected = true;
  }
  else
  {
    pClient->disconnect();
    return false;
  }

  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  NimBLEDevice::init("");

  NimBLEScan *pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pScan->setInterval(45);
  pScan->setWindow(15);
  pScan->setActiveScan(true);
  pScan->start(5, onScanEnd);
}

void loop()
{
  if (doConnect)
  {
    doConnect = false;
    connectToServer();
  }
  if (connected)
  {
    pRemoteCharacteristic->writeValue("1");
    delay(1000);
    pRemoteCharacteristic->writeValue("0");
    delay(1000);
  }
  if (doScan)
  {
    NimBLEDevice::getScan()->start(0);
  }
}
