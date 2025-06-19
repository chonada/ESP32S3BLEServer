
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

BLESecurity *pSecurity = new BLESecurity();

uint width = 0;
uint height = 0;

struct Point {
  short x;
  short y;
};

class Lcd {
private:
  TFT_eSPI *tft;

public:
  Lcd(TFT_eSPI *tftp) {
    tft = tftp;
    tft->init();
    tft->setRotation(1);
    tft->fillScreen(TFT_DARKGREY);
    tft->setTextFont(1);
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
  }

  void status(char *status) {
    Point statusLocation = {
      0, tft->height() / 2
    };
    tft->drawString(status, statusLocation.x, statusLocation.y);
  }
};

Lcd *lcd;

class MySecurity : public BLESecurityCallbacks {
private:
  Lcd *lcd;

public:
  MySecurity(Lcd *lcdp) {
    lcd = lcdp;
  }

  uint32_t onPassKeyRequest() override {
    Serial.println("Passkey requested");
    lcd->status("Passkey requested");
    return 123456;  // Set your passkey here
  }

  void onPassKeyNotify(uint32_t passkey) override {
    Serial.print("Passkey: ");
    Serial.println(passkey);
    lcd->status("onPassKeyNotify");
  }

  bool onConfirmPIN(uint32_t passkey) override {
    Serial.print("Confirm Passkey: ");
    Serial.println(passkey);
    lcd->status("onConfirmPIN");
    return true;  // Return true to confirm the PIN
  }

  bool onSecurityRequest() override {
    lcd->status("onSecurityRequest");
    return true;  // Accept security requests from the peer device
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) override {
    if (cmpl.success) {
      lcd->status("Authentication Success");
      Serial.println("Authentication Success");
    } else {
      lcd->status("Authentication Failed");
      Serial.println("Authentication Failed");
    }
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32 Passkey");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps with iPhone discovery
  pAdvertising->setMinPreferred(0x12);
  pSecurity->setCapability(ESP_IO_CAP_IO);
  pSecurity->setKeySize(16);
  pSecurity->setStaticPIN(123456);  // Set a 6-digit passkey here
  lcd = new Lcd(&tft);
  BLEDevice::setSecurityCallbacks(new MySecurity(lcd));
}

void loop() {
  delay(1000);
}