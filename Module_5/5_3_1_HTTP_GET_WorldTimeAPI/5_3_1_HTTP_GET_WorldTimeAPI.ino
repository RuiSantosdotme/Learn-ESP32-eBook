/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/learn-esp32-with-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Specify the timezone you want to get the time for: https://worldtimeapi.org/api/timezone/
const char* timezone = "Europe/Lisbon";
// API endpoint
String url = String("http://worldtimeapi.org/api/timezone/") + timezone;

// Store date and time
String currentDate;
String currentTime;

// Store hour, minute, second
int hour;
int minute;
int second;

// Store the result from the API request
String requestData = "failed";

String formatTime(int time) {
  return (time < 10) ? "0" + String(time) : String(time);
}

String getDateTime() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the request
    http.begin(url);
    // Make the GET request
    int httpCode = http.GET();
    if (httpCode > 0) {
      // Check for the response
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Time information:");
        Serial.println(payload);
        // Parse the JSON to extract the time
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          const char* datetime = doc["datetime"];          
          // Split the datetime into date and time
          return String(datetime);
        } else {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return "failed";
        }
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
      return "failed";
    }
    http.end(); // Close connection
  } else {
    Serial.println("Not connected to Wi-Fi");
    return "failed";
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  while(requestData == "failed") {
    requestData = getDateTime();
  }

  int splitIndex = requestData.indexOf('T');
  // Extract date
  currentDate = requestData.substring(0, splitIndex);
  Serial.println("Current Date: " + currentDate);

  // Extract time
  currentTime = requestData.substring(splitIndex + 1, splitIndex + 9); 
  Serial.println("Current Time: " + currentTime);
  
  // Extract hour, minute, second
  hour = currentTime.substring(0, 2).toInt();
  minute = currentTime.substring(3, 5).toInt();
  second = currentTime.substring(6, 8).toInt();
  Serial.println("Hour: " + String(formatTime(hour)));
  Serial.println("Minute: " + String(formatTime(minute)));
  Serial.println("Second: " + String(formatTime(second)));
}

void loop() {

}