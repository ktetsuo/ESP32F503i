#ifndef __F503I_H__
#define __F503I_H__

#include <BLEAddress.h>
#include <BLEClient.h>
#include <cstdint>
#include <semphr.h>

class BLEClient;
class BLERemoteService;
class BLERemoteCharacteristic;

class F503i
{
public:
  F503i();
  ~F503i();

  bool begin(const BLEAddress &bleAddress);
  void end();
  bool isConnect() const;

  // キー関係
  uint16_t getKeyValue() const;
  bool isKeyOn(unsigned int keyIndex) const;
  static bool isKeyOn(uint16_t keyValue, unsigned int keyIndex);
  static char keyIndexToChar(unsigned int keyIndex);
  enum KeyIndex
  {
    KEY_0 = 0,
    KEY_1 = 1,
    KEY_2 = 2,
    KEY_3 = 3,
    KEY_4 = 4,
    KEY_5 = 5,
    KEY_6 = 6,
    KEY_7 = 7,
    KEY_8 = 8,
    KEY_9 = 9,
    KEY_ASTER = 10, // *
    KEY_SHARP = 11, // #
    //
    KEY_COUNT = 12,
  };
  enum KeyMask : uint16_t
  {
    KEY_MASK_0 = 0x0001,
    KEY_MASK_1 = 0x0002,
    KEY_MASK_2 = 0x0004,
    KEY_MASK_3 = 0x0008,
    KEY_MASK_4 = 0x0010,
    KEY_MASK_5 = 0x0020,
    KEY_MASK_6 = 0x0040,
    KEY_MASK_7 = 0x0080,
    KEY_MASK_8 = 0x0100,
    KEY_MASK_9 = 0x0200,
    KEY_MASK_ASTER = 0x0400,
    KEY_MASK_SHARP = 0x0800,
  };
  static const uint16_t KEY_MASK_TABLE[KEY_COUNT];
  static const char KEY_CHAR_TABLE[KEY_COUNT];

  // LED関係
  void turnOnLED(unsigned int ledIndex);
  void turnOffLED(unsigned int ledIndex);
  void setLEDBrightness(unsigned int ledIndex, uint8_t brightness);
  enum LEDIndex
  {
    LED_LEFT = 0,
    LED_CENTER = 1,
    LED_RIGHT = 2,
    //
    LED_COUNT = 3,
  };

  // 光センサー関係
  uint16_t getLightSensorValue() const;

  // ブザー関係
  void turnOnBuzzer(uint8_t value);
  void turnOffBuzzer();
  enum Note
  {
    NOTE_OFF = 0,
    NOTE_A0 = 1,
    NOTE_AS0 = 2,
    NOTE_B0 = 3,
    NOTE_C1 = 4,
    NOTE_CS1 = 5,
    NOTE_D1 = 6,
    NOTE_DS1 = 7,
    NOTE_E1 = 8,
    NOTE_F1 = 9,
    NOTE_FS1 = 10,
    NOTE_G1 = 11,
    NOTE_GS1 = 12,
    NOTE_A1 = 13,
    NOTE_AS1 = 14,
    NOTE_B1 = 15,
    NOTE_C2 = 16,
    NOTE_CS2 = 17,
    NOTE_D2 = 18,
    NOTE_DS2 = 19,
    NOTE_E2 = 20,
    NOTE_F2 = 21,
    NOTE_FS2 = 22,
    NOTE_G2 = 23,
    NOTE_GS2 = 24,
    NOTE_A2 = 25,
    NOTE_AS2 = 26,
    NOTE_B2 = 27,
    NOTE_C3 = 28,
    NOTE_CS3 = 29,
    NOTE_D3 = 30,
    NOTE_DS3 = 31,
    NOTE_E3 = 32,
    NOTE_F3 = 33,
    NOTE_FS3 = 34,
    NOTE_G3 = 35,
    NOTE_GS3 = 36,
    NOTE_A3 = 37,
    NOTE_AS3 = 38,
    NOTE_B3 = 39,
    NOTE_C4 = 40,
    NOTE_CS4 = 41,
    NOTE_D4 = 42,
    NOTE_DS4 = 43,
    NOTE_E4 = 44,
    NOTE_F4 = 45,
    NOTE_FS4 = 46,
    NOTE_G4 = 47,
    NOTE_GS4 = 48,
    NOTE_A4 = 49,
    NOTE_AS4 = 50,
    NOTE_B4 = 51,
    NOTE_C5 = 52,
    NOTE_CS5 = 53,
    NOTE_D5 = 54,
    NOTE_DS5 = 55,
    NOTE_E5 = 56,
    NOTE_F5 = 57,
    NOTE_FS5 = 58,
    NOTE_G5 = 59,
    NOTE_GS5 = 60,
    NOTE_A5 = 61,
    NOTE_AS5 = 62,
    NOTE_B5 = 63,
    NOTE_C6 = 64,
    NOTE_CS6 = 65,
    NOTE_D6 = 66,
    NOTE_DS6 = 67,
    NOTE_E6 = 68,
    NOTE_F6 = 69,
    NOTE_FS6 = 70,
    NOTE_G6 = 71,
    NOTE_GS6 = 72,
    NOTE_A6 = 73,
    NOTE_AS6 = 74,
    NOTE_B6 = 75,
    NOTE_C7 = 76,
    NOTE_CS7 = 77,
    NOTE_D7 = 78,
    NOTE_DS7 = 79,
    NOTE_E7 = 80,
    NOTE_F7 = 81,
    NOTE_FS7 = 82,
    NOTE_G7 = 83,
    NOTE_GS7 = 84,
    NOTE_A7 = 85,
    NOTE_AS7 = 86,
    NOTE_B7 = 87,
    NOTE_C8 = 88,
    NOTE_CS8 = 89,
    NOTE_D8 = 90,
    NOTE_DS8 = 91,
    NOTE_E8 = 92,
    NOTE_F8 = 93,
    NOTE_FS8 = 94,
    NOTE_G8 = 95,
    NOTE_GS8 = 96,
    NOTE_A8 = 97,
    NOTE_AS8 = 98,
    NOTE_B8 = 99,
    NOTE_C9 = 100,
    NOTE_CS9 = 101,
    NOTE_D9 = 102,
    NOTE_DS9 = 103,
    NOTE_E9 = 104,
    NOTE_F9 = 105,
    NOTE_FS9 = 106,
    NOTE_G9 = 107,
    NOTE_GS9 = 108,
  };

private:
  class ClientCallback : public BLEClientCallbacks
  {
    bool _bConnect = false;

  public:
    bool isConnect() const;

  private:
    virtual void onConnect(BLEClient *pClient) override;
    virtual void onDisconnect(BLEClient *pClient) override;
  };

  uint16_t _keyValue = 0;
  uint16_t _lightSensorValue = 0;

  BLEAddress _bleAddress = BLEAddress("");
  BLEClient *_pClient = nullptr;
  BLERemoteService *_pService = nullptr;
  BLERemoteCharacteristic *_pKeyChara = nullptr;
  BLERemoteCharacteristic *_pLEDChara[LED_COUNT] = {};
  BLERemoteCharacteristic *_pLightSensorChara = nullptr;
  BLERemoteCharacteristic *_pLightSensorConfigChara = nullptr;
  BLERemoteCharacteristic *_pBuzzerChara = nullptr;

  ClientCallback _clientCallback;
  SemaphoreHandle_t _mutex;

  static void connectTask(void *param);
  void connetTask();
};

#endif // __F503I_H__
