#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<ESPAsyncUDP.h>
#include<ESP8266mDNS.h>
#include<Streaming.h>
#include<OneWire.h>
#include<DallasTemperature.h>

const char* ssid = "747_2.4GHz";
const char* password = ""; // provide

ESP8266WebServer server(80);
String serviceName = "Sensor-Boiler-Temp";

AsyncUDP udp;

long previousUdpBrodcast = 0;
long brodcastInterval = 5000; // 5 seconds

OneWire sensorBus(D4);
DallasTemperature tempSensor(&sensorBus);

float temp;

void setup() {
  Serial.begin(9600);

  Serial << "Iniciando" << endl;
  Serial << "Conectando na rede: " << ssid << endl;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial << "WiFi Status: " << wifiStatusToString(WiFi.status()) << endl;
  }

  WiFi.setAutoReconnect(true);
  WiFi.hostname(serviceName);

  Serial << "Conectado" << endl;
  Serial << "Endereco local: " << WiFi.localIP() << endl;

  if (!MDNS.begin(serviceName)) {
    Serial << "Erro ao iniciar mDNS" << endl;
  }

  MDNS.addService("http", "tcp", 80);

  server.on("/", HTTPMethod::HTTP_GET, indexHandler);
   server.on("/json", HTTPMethod::HTTP_GET, jsonHandler);
  server.on("/metrics", HTTPMethod::HTTP_GET, metricsHandler);
  server.onNotFound(notFoundHandler);
  server.begin();
}

void indexHandler() {
  server.send(200, "text/html; charset=utf-8", "<html><head><meta charset=\"UTF-8\"><title>Temperatura Boiler</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>Temperatura: " + String(temp) + "°C</body>");
}

void jsonHandler() {
  server.send(200, "application/json", "{\"temperatura\": " + String(temp) + "}");
}

void notFoundHandler() {
  server.send(404, "text/plain; charset=utf-8", "Não encontrado");
}

void metricsHandler() {
  String output = String();
  output += "# HELP sensor_boiler_temp Temperatura do boiler\n";
  output += "# TYPE sensor_boiler_temp gauge\n";
  output += "sensor_boiler_temp{value=\"temperatura\"} " + String(temp) + "\n";
  server.send(200, "text/plain; charset=utf-8", output);
}

void loop() {
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempCByIndex(0);
  Serial << "Temperatura sensor: " << temp << endl;
  MDNS.update();

  unsigned long currentMillis = millis();

  if ((currentMillis - previousUdpBrodcast) > brodcastInterval && WiFi.isConnected()) {
    udp.broadcastTo(serviceName.c_str(), 8804);
    previousUdpBrodcast = currentMillis;
  }

  server.handleClient();
}

String wifiStatusToString(int status) {
  switch (status) {
    case 255: return "WL_NO_SHIELD";
    case 0: return "WL_IDLE_STATUS";
    case 1: return "WL_NO_SSID_AVAIL";
    case 2: return "WL_SCAN_COMPLETED";
    case 3: return "WL_CONNECTED";
    case 4: return "WL_CONNECT_FAILED";
    case 5: return "WL_CONNECTION_LOST";
    case 6: return "WL_WRONG_PASSWORD";
    case 7: return "WL_DISCONNECTED";
  }

  return String("UNKNOWN") + "(" + status + ")";
}