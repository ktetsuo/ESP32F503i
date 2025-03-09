#include "F503i.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include "esp32-hal-log.h"

static constexpr const char *SERVICE_UUID = "f7fce510-7a0b-4b89-a675-a79137223e2c"; // サービスのUUID

static constexpr const char *KEY_CHARA_UUID = "f7fce531-7a0b-4b89-a675-a79137223e2c"; // キーのCharacteristicのUUID

static constexpr const char *LED_LEFT_CHARA_UUID = "f7fce517-7a0b-4b89-a675-a79137223e2c";   // LED(左)のUUID
static constexpr const char *LED_CENTER_CHARA_UUID = "f7fce518-7a0b-4b89-a675-a79137223e2c"; // LED(真ん中)のUUID
static constexpr const char *LED_RIGHT_CHARA_UUID = "f7fce51b-7a0b-4b89-a675-a79137223e2c";  // LED(右)のUUID

static constexpr const char *BUZZER_CHARA_UUID = "f7fce521-7a0b-4b89-a675-a79137223e2c"; // ブザーのUUID

static constexpr const char *LIGHT_SENSOR_CHARA_UUID = "f7fce532-7a0b-4b89-a675-a79137223e2c";        // 光センサーのUUID
static constexpr const char *LIGHT_SENSOR_CONFIG_CHARA_UUID = "f7fce533-7a0b-4b89-a675-a79137223e2c"; // 光センサー設定のUUID

const uint16_t F503i::KEY_MASK_TABLE[KEY_COUNT] = {
    KEY_MASK_0,
    KEY_MASK_1,
    KEY_MASK_2,
    KEY_MASK_3,
    KEY_MASK_4,
    KEY_MASK_5,
    KEY_MASK_6,
    KEY_MASK_7,
    KEY_MASK_8,
    KEY_MASK_9,
    KEY_MASK_ASTER,
    KEY_MASK_SHARP,
};
const char F503i::KEY_CHAR_TABLE[KEY_COUNT] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '*',
    '#',
};

////////////////////////////////////////////////////////////////////////////////
bool F503i::ClientCallback::isConnect() const
{
  return _bConnect;
}

void F503i::ClientCallback::onConnect(BLEClient *pClient)
{
  _bConnect = true;
}

void F503i::ClientCallback::onDisconnect(BLEClient *pClient)
{
  _bConnect = false;
}

////////////////////////////////////////////////////////////////////////////////
F503i::F503i()
{
  _mutex = xSemaphoreCreateMutex();
}

F503i::~F503i()
{
  if (_mutex != nullptr)
  {
    vSemaphoreDelete(_mutex);
  }
}

bool F503i::begin(const BLEAddress &bleAddress)
{
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE)
  {
    // 事前にBLEDevice::init()が必要です
    _bleAddress = bleAddress;
    _pClient = BLEDevice::createClient();
    if (_pClient == nullptr)
    {
      log_e("Failed to create BLEClient");
      xSemaphoreGive(_mutex);
      return false;
    }
    _pClient->setClientCallbacks(&_clientCallback);

    if (xTaskCreate(connectTask, "connectTask", 4096, this, 1, nullptr) != pdPASS)
    {
      log_e("Failed to create connectTask");
      xSemaphoreGive(_mutex);
      return false;
    }
    xSemaphoreGive(_mutex);
    return true;
  }
  return false;
}

void F503i::connectTask(void *param)
{
  F503i *pThis = static_cast<F503i *>(param);
  pThis->connetTask();
  vTaskDelete(nullptr);
}

void F503i::connetTask()
{
  while (_pClient != nullptr)
  {
    if (_pClient->isConnected())
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    if (!_pClient->connect(_bleAddress))
    {
      BLEAddress a = _bleAddress; // BLEAddress.toString()がconstでないので仕方なくコピー
      log_e("Failed to connect: %s", a.toString().c_str());
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
    {
      log_e("mutex error");
      break;
    }
    // サービス取得
    _pService = _pClient->getService(SERVICE_UUID);
    if (_pService == nullptr)
    {
      log_e("Failed to find service UUID: %s", SERVICE_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    // キーのキャラクタリスティック取得
    _pKeyChara = _pService->getCharacteristic(KEY_CHARA_UUID);
    if (_pKeyChara == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", KEY_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    // キーの通知ハンドラを設定
    if (_pKeyChara->canNotify())
    {
      _pKeyChara->registerForNotify([this](BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                    {
                                      // キーの状態が変わったときの処理
                                      if (length == 2)
                                      {
                                        _keyValue = ~((pData[1] << 8) | pData[0]) & 0x0fff;
                                      } });
    }
    // LEDのキャラクタリスティック取得
    _pLEDChara[LED_LEFT] = _pService->getCharacteristic(LED_LEFT_CHARA_UUID);
    if (_pLEDChara[LED_LEFT] == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", LED_LEFT_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    _pLEDChara[LED_CENTER] = _pService->getCharacteristic(LED_CENTER_CHARA_UUID);
    if (_pLEDChara[LED_CENTER] == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", LED_CENTER_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    _pLEDChara[LED_RIGHT] = _pService->getCharacteristic(LED_RIGHT_CHARA_UUID);
    if (_pLEDChara[LED_RIGHT] == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", LED_RIGHT_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }

    // 光センサー設定のキャラクタリスティック取得
    _pLightSensorConfigChara = _pService->getCharacteristic(LIGHT_SENSOR_CONFIG_CHARA_UUID);
    if (_pLightSensorConfigChara == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", LIGHT_SENSOR_CONFIG_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    // 通知を有効にする
    _pLightSensorConfigChara->writeValue(static_cast<uint8_t>(1), true);

    // 光センサーのキャラクタリスティック取得
    _pLightSensorChara = _pService->getCharacteristic(LIGHT_SENSOR_CHARA_UUID);
    if (_pLightSensorChara == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", LIGHT_SENSOR_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    // 光センサーの通知ハンドラを設定
    if (_pLightSensorChara->canNotify())
    {
      _pLightSensorChara->registerForNotify([this](BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
                                            {
                                              // 光センサーの状態が変わったときの処理
                                              if (length == 2)
                                              {
                                                _lightSensorValue = (pData[1] << 8) | pData[0];
                                              } });
    }
    // ブザーのキャラクタリスティック取得
    _pBuzzerChara = _pService->getCharacteristic(BUZZER_CHARA_UUID);
    if (_pBuzzerChara == nullptr)
    {
      log_e("Failed to find characteristic UUID: %s", BUZZER_CHARA_UUID);
      _pClient->disconnect();
      xSemaphoreGive(_mutex);
      break;
    }
    xSemaphoreGive(_mutex);
  }
}

void F503i::end()
{
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
  {
    log_e("mutex error");
    return;
  }
  if (_pClient)
  {
    _pClient->disconnect();
    delete _pClient;
    _pClient = nullptr;
  }
  _pService = nullptr;
  _pKeyChara = nullptr;
  _pLEDChara[LED_LEFT] = nullptr;
  _pLEDChara[LED_CENTER] = nullptr;
  _pLEDChara[LED_RIGHT] = nullptr;
  _pLightSensorChara = nullptr;
  _pLightSensorConfigChara = nullptr;
  _pBuzzerChara = nullptr;
  xSemaphoreGive(_mutex);
}

bool F503i::isConnect() const
{
  return _clientCallback.isConnect();
}

uint16_t F503i::getKeyValue() const
{
  return _keyValue;
}

bool F503i::isKeyOn(unsigned int keyIndex) const
{
  return isKeyOn(_keyValue, keyIndex);
}

bool F503i::isKeyOn(uint16_t keyValue, unsigned int keyIndex)
{
  if (KEY_COUNT <= keyIndex)
  {
    return false;
  }
  return keyValue & KEY_MASK_TABLE[keyIndex];
}

char F503i::keyIndexToChar(unsigned int keyIndex)
{
  if (KEY_COUNT <= keyIndex)
  {
    return '\0';
  }
  return KEY_CHAR_TABLE[keyIndex];
}

void F503i::turnOnLED(unsigned int ledIndex)
{
  setLEDBrightness(ledIndex, 255);
}

void F503i::turnOffLED(unsigned int ledIndex)
{
  setLEDBrightness(ledIndex, 0);
}

void F503i::setLEDBrightness(unsigned int ledIndex, uint8_t brightness)
{
  if (LED_COUNT <= ledIndex)
  {
    return;
  }
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
  {
    log_e("mutex error");
    return;
  }
  BLERemoteCharacteristic *pLedChara = _pLEDChara[ledIndex];
  if (pLedChara == nullptr)
  {
    xSemaphoreGive(_mutex);
    return;
  }
  pLedChara->writeValue(&brightness, 1);
  xSemaphoreGive(_mutex);
}

uint16_t F503i::getLightSensorValue() const
{
  return _lightSensorValue;
}

void F503i::turnOnBuzzer(uint8_t value)
{
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
  {
    log_e("mutex error");
    return;
  }
  if (!_pBuzzerChara)
  {
    xSemaphoreGive(_mutex);
    return;
  }
  _pBuzzerChara->writeValue(value);
  xSemaphoreGive(_mutex);
}

void F503i::turnOffBuzzer()
{
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
  {
    log_e("mutex error");
    return;
  }
  if (!_pBuzzerChara)
  {
    xSemaphoreGive(_mutex);
    return;
  }
  _pBuzzerChara->writeValue(0);
  xSemaphoreGive(_mutex);
}
