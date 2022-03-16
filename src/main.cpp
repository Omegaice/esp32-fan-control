#include <sstream>

// Framework
#include <Arduino.h>

// Fans
#include "NoctuaFan.h"

NoctuaFan fan(0, GPIO_NUM_26, GPIO_NUM_5);

// Sensors
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire onewire(GPIO_NUM_15);
DallasTemperature temperature(&onewire);

// Wifi
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// Sensor Data
float CurrentTemperatureC = 0.0;

void prometheus_metrics(AsyncWebServerRequest *request) {
    std::stringstream sMetrics;
    sMetrics << "# HELP sensor_temperature ambient temperature" << std::endl;
    sMetrics << "# TYPE sensor_temperature gauge" << std::endl;
    sMetrics << "sensor_temperature " << CurrentTemperatureC << std::endl;
    sMetrics << "# HELP fan_speed fan speed setting" << std::endl;
    sMetrics << "# TYPE fan_speed gauge" << std::endl;
    sMetrics << "fan_speed " << fan.get_speed() * 100.0f << std::endl;
    sMetrics << "# HELP fan_rpm fan rotations per minute" << std::endl;
    sMetrics << "# TYPE fan_rpm gauge" << std::endl;
    sMetrics << "fan_rpm " << fan.RotationsPerMinute();
    request->send(200, "text/plain", sMetrics.str().c_str());
}

void http_fan(AsyncWebServerRequest *request) {
    if (request->hasParam("speed")) {
        std::stringstream sSpeed(request->getParam("speed")->value().c_str());

        float percentage = 0.0f;
        sSpeed >> percentage;
        Serial.printf("Setting fan speed to %3.2f\n", percentage);

        fan.set_speed(percentage);
    }

    std::stringstream sResult;
    sResult << "fan_speed " << fan.get_speed() * 100.0f;
    request->send(200, "text/plain", sResult.str().c_str());
}

void IRAM_ATTR fan_interrupt() {
    fan.increment();
}

void setup() {
    Serial.begin(115200);

    temperature.begin();
    fan.begin(fan_interrupt);

    const char* ssid = "OurSweetIOT";
    const char* password = "AComplexPassword";

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/metrics", HTTP_GET, prometheus_metrics);
    server.on("/fan", HTTP_GET, http_fan);
    server.begin();
}

unsigned long lastTempUpdate = 0;
void loop() {
    fan.update();

    unsigned long currentMillis = millis();
    if ((currentMillis - lastTempUpdate) >= 1000) {
        lastTempUpdate = currentMillis;

        temperature.requestTemperatures();
        CurrentTemperatureC = temperature.getTempCByIndex(0);
    }
}