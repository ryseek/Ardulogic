#include "LCD.h"

LCD::LCD()
{
  Wire.begin(4, 5);
  Wire.beginTransmission(0x3c);
  uint8_t error = Wire.endTransmission();
  if (error == 0) {
    isset = true;
    display = new SSD1306(0x3c, 4, 5); // https://github.com/esp8266/Arduino/blob/master/variants/d1_mini/pins_arduino.h
  } else {
    isset = false;
  }
}

void LCD::init()
{
  if (!isset) {
    return;
  }
  display->init();
  display->flipScreenVertically();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
}

void LCD::gpsScreen()
{
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Orbitron_Light_30);
  display->drawString(64, 6, "GPS");
  display->drawString(64, 34, "search");
  display->display();
}

void LCD::updateScreen(FullData* fullData, Metering* metering)
{
  display->clear();

  //Desenho linhas sinal
  display->drawVerticalLine(7, 2, 8);
  display->drawVerticalLine(6, 2, 8);
  display->drawVerticalLine(4, 5, 5);
  display->drawVerticalLine(3, 5, 5);
  display->drawVerticalLine(1, 8, 2);
  display->drawVerticalLine(0, 8, 2);

  display->drawHorizontalLine(0, 11, 128);
  display->drawHorizontalLine(0, 54, 128);

  //sprintf(gpsNumSatellites, "%d", ubxMessage.navPvt.numSV);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(11, 0, (String)fullData->numSV);

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(42, 0, (String)fullData->hAcc);

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, fullData->gpsTime);

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);
  dtostrf(metering->accel30, 3, 1, buf30);
  display->drawString(122, 10, (String)buf30);

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);
  dtostrf(metering->accel60, 3, 1, buf60);
  display->drawString(122, 24, (String)buf60);

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);
  dtostrf(metering->accel100, 3, 1, buf100);
  display->drawString(122, 38, (String)buf100);

  //dtostrf(latitude, 8, 6, bufLatitude);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0, 54, fullData->bufLatitude);

  //dtostrf(longitude, 8, 6, bufLongitude);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 54, fullData->bufLongitude);

  char gpsSpeed[3];
  sprintf(gpsSpeed, "%03d", fullData->gSpeedKm);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Orbitron_Light_30);
  display->drawString(45, 20, gpsSpeed);

  display->display();
}

