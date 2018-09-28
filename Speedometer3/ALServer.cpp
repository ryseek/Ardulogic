#include "ALServer.h"

const char *ssid     = "ARDULOGIC";  // Название сети WiFi
const char *password = "1234567890"; // Пароль для подключения

ESP8266WebServer server(80);         // Создаем веб сервер на 80 порту

ALServer::ALServer()
{
}

void ALServer::serverHandle()
{
  server.handleClient(); // Ждёи подлючения
}

void ALServer::setData(Metering* meterings, size_t n)
{
  _meterings = meterings;
  _n = n;
}

void ALServer::createAP()
{
  WiFi.softAP(ssid, password); // Создаём точку WiFi
  delay(500);

  // Указываем по каким роутам какие методы запускать
  server.on("/", [this]() {
    handleRoot();
  });

  server.on("/style.css", [this]() {
    handleStyle();
  });

  server.on("/results", [this]() {
    handleResults();
  });

  server.begin();
}

// Метод формирует стартовую страницу http://192.168.4.1
void ALServer::handleRoot()
{
  server.send(200, "text/html", data_indexHTML);
}

void ALServer::handleStyle() {
  server.send(200, "text/css", data_styleCSS);
}

void ALServer::handleResults()
{
  StaticJsonBuffer<2000> jsonBuffer;
  JsonArray& arrayJ = jsonBuffer.createArray();

  for (int i = 0; i < _n; i++) {
    if (0.0 != _meterings[i].accel30) {
      JsonObject& object = jsonBuffer.createObject();
      dtostrf(_meterings[i].accel30, 3, 1, buf30);
      object["a30"] = (String)buf30;

      dtostrf(_meterings[i].accel60, 3, 1, buf60);
      object["a60"] = (String)buf60;

      dtostrf(_meterings[i].accel100, 3, 1, buf100);
      object["a100"] = (String)buf100;
      arrayJ.add(object);
    }
  }

  char buffer[512];
  arrayJ.printTo(buffer, sizeof(buffer));

  server.send(200, "text/html", buffer);
}

