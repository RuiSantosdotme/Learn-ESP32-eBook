/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/learn-esp32-with-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

// Load libraries
#include <WiFi.h>
#include <Preferences.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// DHT Sensor
const int DHTPin = 27;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables for temperature and humidity
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

// Preferences object
Preferences preferences;

// Set GPIOs for: output variable, RGB LED, PIR Motion Sensor, and LDR
const int output = 2;
const int redRGB = 14;
const int greenRGB = 12;
const int blueRGB = 13;
const int motionSensor = 25;
const int ldr = 33;
// Store the current output state
String outputState = "off";

// Timers - Auxiliary variables
long now = millis();
long lastMeasure = 0;
boolean startTimer = false;

// Auxiliary variables to store selected mode and settings 
int selectedMode = 0;
int timer = 0;
int ldrThreshold = 0;
int armMotion = 0;
int armLdr = 0;
String modes[4] = { "Manual", "Auto PIR", "Auto LDR", "Auto PIR and LDR" };

// Decode HTTP GET value
String valueString = "0";
int pos1 = 0;
int pos2 = 0;
// Variable to store the HTTP request
String header;
// Set web server port number to 80
WiFiServer server(80);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  // initialize the DHT sensor
  dht.begin();

  // Serial port for debugging purposes
  Serial.begin(115200);

  // PIR Motion Sensor mode, then set interrupt function and RISING mode
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  
  Serial.println("start...");

  // Initialize Preferences
  preferences.begin("settings", false);
  
  // Initialize the output variable and RGB pins as OUTPUTs
  pinMode(output, OUTPUT);
  pinMode(redRGB, OUTPUT);
  pinMode(greenRGB, OUTPUT);
  pinMode(blueRGB, OUTPUT);

  // Read from flash memory on start and store the values in auxiliary variables
  // Set output to last state (saved in the flash memory)
  if(preferences.getInt("outputState", 0) == 0) {
    outputState = "off";
    digitalWrite(output, HIGH);
  }
  else {
    outputState = "on";
    digitalWrite(output, LOW);
  }

  selectedMode = preferences.getInt("selectedMode", 0);
  timer = preferences.getInt("timer", 0);
  ldrThreshold = preferences.getInt("ldrThreshold", 0);
  configureMode();
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();                     
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Request example: GET /?mode=0& HTTP/1.1 - sets mode to Manual (0)
            if(header.indexOf("GET /?mode=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              selectedMode = valueString.toInt();
              preferences.putInt("selectedMode", selectedMode);
              configureMode();
            }
            // Change the output state - turn GPIOs on and off
            else if(header.indexOf("GET /?state=on") >= 0) {
              outputOn();
            } 
            else if(header.indexOf("GET /?state=off") >= 0) {
              outputOff();
            }
            // Set timer value
            else if(header.indexOf("GET /?timer=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              timer = valueString.toInt();
              preferences.putInt("timer", timer);
              Serial.println(valueString);
            }
            // Set LDR Threshold value
            else if(header.indexOf("GET /?ldrthreshold=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              ldrThreshold = valueString.toInt();
              preferences.putInt("ldrThreshold", ldrThreshold);
              Serial.println(valueString);
            }
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            // Drop down menu to select mode
            client.println("<p><strong>Mode selected:</strong> " + modes[selectedMode] + "</p>");
            client.println("<select id=\"mySelect\" onchange=\"setMode(this.value)\">");
            client.println("<option>Change mode");
            client.println("<option value=\"0\">Manual");
            client.println("<option value=\"1\">Auto PIR");
            client.println("<option value=\"2\">Auto LDR");
            client.println("<option value=\"3\">Auto PIR and LDR</select>");
          
            // Display current state, and ON/OFF buttons for output 
            client.println("<p>GPIO - State " + outputState + "</p>");
            if(selectedMode == 0) {
              if(outputState == "off") {
                client.println("<p><button class=\"button\" onclick=\"outputOn()\">ON</button></p>");
              } 
              else {
                client.println("<p><button class=\"button button2\" onclick=\"outputOff()\">OFF</button></p>");
              }
            }
            else if(selectedMode == 1) {
              client.println("<p>Timer (0 and 255 in seconds): <input type=\"number\" name=\"txt\" value=\"" + 
                              String(preferences.getInt("timer", 0)) + "\" onchange=\"setTimer(this.value)\" min=\"0\" max=\"255\"></p>");
            }
            else if(selectedMode == 2) {
              client.println("<p>LDR Threshold (0 and 100%): <input type=\"number\" name=\"txt\" value=\"" + 
                              String(preferences.getInt("ldrThreshold", 0)) + "\" onchange=\"setThreshold(this.value)\" min=\"0\" max=\"100\"></p>");
            }
            else if(selectedMode == 3) {
              client.println("<p>Timer (0 and 255 in seconds): <input type=\"number\" name=\"txt\" value=\"" + 
                               String(preferences.getInt("timer", 0)) + "\" onchange=\"setTimer(this.value)\" min=\"0\" max=\"255\"></p>");
              client.println("<p>LDR Threshold (0 and 100%): <input type=\"number\" name=\"txt\" value=\"" + 
                               String(preferences.getInt("ldrThreshold", 0)) + "\" onchange=\"setThreshold(this.value)\" min=\"0\" max=\"100\"></p>");            
            }
            // Get and display DHT sensor readings
            if(header.indexOf("GET /?sensor") >= 0) {
              // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
              float h = dht.readHumidity();
              // Read temperature as Celsius (the default)
              float t = dht.readTemperature();
              // Read temperature as Fahrenheit (isFahrenheit = true)
              float f = dht.readTemperature(true);
              // Check if any reads failed and exit early (to try again).
              if (isnan(h) || isnan(t) || isnan(f)) {
                Serial.println("Failed to read from DHT sensor!");
                strcpy(celsiusTemp,"Failed");
                strcpy(fahrenheitTemp, "Failed");
                strcpy(humidityTemp, "Failed");         
              }
              else {
                dtostrf(t, 6, 2, celsiusTemp);             
                dtostrf(f, 6, 2, fahrenheitTemp);         
                dtostrf(h, 6, 2, humidityTemp);
              }
              client.println("<p>");
              client.println(celsiusTemp);
              client.println("*C</p><p>");
              client.println(fahrenheitTemp);
              client.println("*F</p></div><p>");
              client.println(humidityTemp);
              client.println("%</p></div>");
              client.println("<p><a href=\"/\"><button>Remove Sensor Readings</button></a></p>");
            }
            else {
              client.println("<p><a href=\"?sensor\"><button>View Sensor Readings</button></a></p>");
            }
            client.println("<script> function setMode(value) { var xhr = new XMLHttpRequest();"); 
            client.println("xhr.open('GET', \"/?mode=\" + value + \"&\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); } ");
            client.println("function setTimer(value) { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?timer=\" + value + \"&\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); } ");
            client.println("function setThreshold(value) { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?ldrthreshold=\" + value + \"&\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); } ");
            client.println("function outputOn() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?state=on\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); } ");
            client.println("function outputOff() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?state=off\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); } ");
            client.println("function updateSensorReadings() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?sensor\", true);"); 
            client.println("xhr.send(); setInterval(function(){ location.reload(true); }, 1500); }</script></body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
  }
  
  // Starts a timer to turn on/off the output according to the time value or LDR reading
  now = millis();
  
  // Mode selected (1): Auto PIR
  if(startTimer && armMotion && !armLdr) {
    if(outputState == "off") {
      outputOn();
    }
    else if((now - lastMeasure > (timer * 1000))) {
      outputOff();
      startTimer = false;     
    }
  }  
  
  // Mode selected (2): Auto LDR
  // Read current LDR value and turn the output accordingly
  if(armLdr && !armMotion) {
    int ldrValue = map(analogRead(ldr), 0, 4095, 0, 100); 
    //Serial.println(ldrValue);
    if(ldrValue > ldrThreshold && outputState == "on") {
      outputOff();
    }
    else if(ldrValue < ldrThreshold && outputState == "off") {
      outputOn();
    }
    delay(100);
  }
  
  // Mode selected (3): Auto PIR and LDR
  if(startTimer && armMotion && armLdr) {
    int ldrValue = map(analogRead(ldr), 0, 4095, 0, 100);
    //Serial.println(ldrValue);
    if(ldrValue > ldrThreshold) {
      outputOff();
      startTimer = false;
    }
    else if(ldrValue < ldrThreshold && outputState == "off") {
      outputOn();
    }
    else if(now - lastMeasure > (timer * 1000)) {
      outputOff();
      startTimer = false;
    }
  }
}

// Checks if motion was detected and the sensors are armed. Then, starts a timer.
void detectsMovement() {
  if(armMotion || (armMotion && armLdr)) {
    Serial.println("MOTION DETECTED!!!");
    startTimer = true;
    lastMeasure = millis();
  }  
}
void configureMode() {
  // Mode: Manual
  if(selectedMode == 0) {
    armMotion = 0;
    armLdr = 0;
    // RGB LED color: red
    digitalWrite(redRGB, LOW);
    digitalWrite(greenRGB, HIGH);
    digitalWrite(blueRGB, HIGH);
  }
  // Mode: Auto PIR
  else if(selectedMode == 1) {
    outputOff();
    armMotion = 1;
    armLdr = 0;
    // RGB LED color: green
    digitalWrite(redRGB, HIGH);
    digitalWrite(greenRGB, LOW);
    digitalWrite(blueRGB, HIGH);
  }
  // Mode: Auto LDR
  else if(selectedMode == 2) {
    armMotion = 0;
    armLdr = 1;
    // RGB LED color: blue    
    digitalWrite(redRGB, HIGH);
    digitalWrite(greenRGB, HIGH);
    digitalWrite(blueRGB, LOW);
  }
  // Mode: Auto PIR and LDR
  else if(selectedMode == 3) {
    outputOff();
    armMotion = 1;
    armLdr = 1;
    // RGB LED color: purple    
    digitalWrite(redRGB, LOW);
    digitalWrite(greenRGB, HIGH);
    digitalWrite(blueRGB, LOW);
  }
}

// Change output pin to on or off
void outputOn() {
  Serial.println("GPIO on");
  outputState = "on";
  digitalWrite(output, LOW);
  preferences.putInt("outputState", 1);
}
void outputOff() { 
  Serial.println("GPIO off");
  outputState = "off";
  digitalWrite(output, HIGH);
  preferences.putInt("outputState", 0);
}