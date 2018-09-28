/*
  Connections:
  GPS TX -> Wemos RX (disconnect to upload this sketch) or Soft port D7
  GPS RX -> Wemos TX  or Soft port D8
  Screen SDA -> Wemos SDA (D2)
  Screen SCL -> Wemos SCL (D1)
*/
#include <Arduino.h>
#include "LCD.h"

#include "UbloxGPS.h"
#include "ALServer.h"

ALServer srv;
LCD lcd;

Metering metering;
Metering meterings[10];

FullData fullData;

void setup() {
  Serial.begin(115200);
  serial.begin(115200);
  delay(500);

  metering = {0.0, 0.0, 0.0, 0.0};

  gpsSetup();
  srv.createAP();
  lcd.init();
}

unsigned long lastScreenUpdate = 0; // Частота обновления дисплея

float latitude  = 0.0;           // Ширина
float longitude = 0.0;           // Долгота

float meteringTime  = 0.0;       // Время замера
bool onlySpeedMeter = false;     // Режим спидометра

unsigned long startMillis   = 0; // Начало отсчета
unsigned long startITOW     = 0; // Начало отсчета из GPS
unsigned long currentMillis = 0; // Текущее время

bool start        = false;       // Начало замера
bool meteringSave = false;       // Сохранён ли последний замер

double startLat  = 0.0;
double startLon  = 0.0;
double finishLat = 0.0;
double finishLon = 0.0;

void loop() {
  currentMillis = millis(); // текущее время в миллисекундах
  int msgType = processGPS();
  if ( msgType == MT_NAV_PVT ) {
    fullData.numSV = ubxMessage.navPvt.numSV;
    fullData.gSpeedKm = ubxMessage.navPvt.gSpeed * 0.0036;
    sprintf(fullData.gpsTime, "%02d:%02d:%02d", ubxMessage.navPvt.hour, ubxMessage.navPvt.minute, ubxMessage.navPvt.second);
    latitude = (float)(ubxMessage.navPvt.lat / 10000000.0f);
    longitude = (float)(ubxMessage.navPvt.lon / 10000000.0f);
    dtostrf(latitude, 8, 6, fullData.bufLatitude);
    dtostrf(longitude, 8, 6, fullData.bufLongitude);
    fullData.hAcc = ubxMessage.navPvt.hAcc / 1000.0f;
  }

  // ---------------------- Замер ----------------------
  // Если движемся
  if (fullData.gSpeedKm > 0) {

    // Время замера
    meteringTime = (float)(ubxMessage.navPvt.iTOW - startITOW) / 1000;   //TODO: в начало


    // Если это был старт
    if (!start) {
      start = true;
      meteringSave = false;
      startMillis = millis();
      startITOW = ubxMessage.navPvt.iTOW;
      metering = {0.0, 0.0, 0.0, 0.0};
      startLat = latitude;
      startLon = longitude;
      finishLat = 0.0;
      finishLon = 0.0;
      onlySpeedMeter = false;
    } else if (meteringTime > 99) {
      // Если замер длится дольше 99 секунд TODO
      onlySpeedMeter = true;
    }

    // Замеры разгона
    if (!onlySpeedMeter) {
      if (0.0 == metering.accel30 && fullData.gSpeedKm >= 30) {
        // Разгон до 30км/ч
        metering.accel30 = meteringTime;
      }
      else if (0.0 == metering.accel60 && fullData.gSpeedKm >= 60) {
        // Разгон до 60км/ч
        metering.accel60 = meteringTime;
      }
      else if (0.0 == metering.accel100 && fullData.gSpeedKm >= 100) {
        // Разгон до 100км/ч
        metering.accel100 = meteringTime;
      }
    }

  }
  // Если была ложная тревога
  else if (start && 0 == fullData.gSpeedKm) {
    start = false;
    meteringSave = false;
    startMillis = 0;
    meteringTime = 0;
    startLat = 0.0;
    startLon = 0.0;
    finishLat = 0.0;
    finishLon = 0.0;
  }
  // -------------------- END Замер --------------------

  // Если замер завершен
  if (!meteringSave && 0.0 != metering.accel100) {

    //------------- последний результат всегда сверху
    Metering *meteringsNew = new Metering[10];
    memcpy(meteringsNew, meterings, sizeof(meterings[0]) * 10);
    meterings[0] = metering;
    for (int i = 0; i < 9; i++)
    {
      meterings[i + 1] = meteringsNew[i];
    }
    delete[] meteringsNew;
    //-------------
    srv.setData(meterings, (int)(sizeof(meterings) / sizeof(meterings[0])));
    meteringSave = true;
  }

  // ---------------------- Экран ----------------------
  unsigned long now = millis();
  if (now - lastScreenUpdate > 100) {
    if (0 == fullData.numSV) {
      lcd.gpsScreen();
    } else {
      lcd.updateScreen(&fullData, &metering);
    }
    lastScreenUpdate = now;
  }
  // -------------------- END Экран --------------------

  // Ждём подключения только если не делаем замер
  if (!start || onlySpeedMeter) {
    srv.serverHandle();
  }
}

