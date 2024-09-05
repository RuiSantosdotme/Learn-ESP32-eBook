/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/learn-esp32-with-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Access Point credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

Adafruit_BME280 bme; // I2C

String temperature;
String humidity;
String pressure;

// Create a web server object
WebServer server(80);

String readTemp() {
  return String(bme.readTemperature());
  //return String(1.8 * bme.readTemperature() + 32);
}

String readHumi() {
  return String(bme.readHumidity());
}

String readPres() {
  return String(bme.readPressure() / 100.0F);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Initialize BME280
  if (!bme.begin(0x76)) {  // Change 0x76 to 0x77 if necessary
    Serial.println("Could not find BME280 sensor, check wiring!");
    while (1);
  }

  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Set up the web server to respond with sensor data
  server.on("/temperature", HTTP_GET, []() {
    temperature = readTemp();
    String response = temperature;
    server.send(200, "text/plain", response);
  });

  server.on("/humidity", HTTP_GET, []() {
    humidity = readHumi();
    String response = humidity;
    server.send(200, "text/plain", response);
  });

  server.on("/pressure", HTTP_GET, []() {
    pressure = readPres();
    String response = pressure;
    server.send(200, "text/plain", response);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}
