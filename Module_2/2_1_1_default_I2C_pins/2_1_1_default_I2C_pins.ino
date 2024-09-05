/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/learn-esp32-with-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

void setup() {
  Serial.begin(115200);

  // Print out the default I2C pins
  Serial.print("Default I2C SCL pin: ");
  Serial.println(SDA);
  
  Serial.print("Default I2C SDA pin: ");
  Serial.println(SCL);
}

void loop() {
  
}