/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/learn-esp32-with-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// The URL of the webpage/API/server
const char* url = "http://www.google.com";

void setup() {
  // Start the serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Create an HTTPClient object
  HTTPClient http;

  // Begin the HTTP request to the specified URL
  http.begin(url);

  // Send the HTTP GET request
  int httpResponseCode = http.GET();

  // Check the response code to see if the request was successful
  if (httpResponseCode > 0) {
    // If successful, print the HTTP response code and the response payload (the content of the page)
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
  } else {
    // If there was an error, print the error code
    Serial.println("Error in HTTP request, code: " + String(httpResponseCode));
  }

  // End the HTTP request
  http.end();
}

void loop() {
}