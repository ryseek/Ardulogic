#ifndef LCD_h
#define LCD_h

#include <SSD1306.h>
#include <Wire.h>
#include "font.h"

#include "ALServer.h"

struct FullData
{
  unsigned int numSV;  // Спутники
  float hAcc;          // Точность
  char gpsTime[9];     // Время
  char bufLatitude[10];
  char bufLongitude[10];
  unsigned int gSpeedKm = 0; // Скорость
};

class LCD
{
  private:
    SSD1306 *display;
    char buf30[5], buf60[5], buf100[5];

  public:
    bool isset;
    LCD();
    void init();
    void gpsScreen();
    void updateScreen(FullData* fullData, Metering* metering);
};

#endif
