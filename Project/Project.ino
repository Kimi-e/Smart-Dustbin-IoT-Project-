#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6JxuOrM9B"
#define BLYNK_TEMPLATE_NAME "iot"
#define BLYNK_AUTH_TOKEN "wl5HMPueNJ-TSo7FMhGIPJU67L6nando"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>

// Components
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer myTimer;
DHT myDHT(15, DHT22);  // DHT connected to D8 (GPIO15)

// WiFi & Blynk Credentials
char auth[] = "wl5HMPueNJ-TSo7FMhGIPJU67L6nando";
char ssid[] = "*******";
char pass[] = "********";

// Pin Definitions
#define trigPin 14          // D5 - Ultrasonic Trig
#define echoPin 13          // D7 - Ultrasonic Echo
#define LED_OPERATION 0     // D3 - Movement indicator LED
#define LED_STATUS 2        // D4 - Dustbin status LED

unsigned long duration;
int distance;
bool isOpen = false;
String lastStatus = ""; // LCD status tracker to prevent flicker

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED_OPERATION, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);

  digitalWrite(LED_OPERATION, LOW);
  digitalWrite(LED_STATUS, LOW);

  myServo.attach(12);  // D6 (GPIO12) - Servo control pin
  myServo.write(0);    // Start closed

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("SMART DUSTBIN");

  myDHT.begin();
  Blynk.begin(auth, ssid, pass);

  myTimer.setInterval(1000L, dustbinControl);
}

void loop() {
  Blynk.run();
  myTimer.run();
}

void dustbinControl() {
  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 20000); // Timeout protection
  distance = duration * 0.0343 / 2;

  // Read temperature
  float temp = myDHT.readTemperature();
  if (isnan(temp)) temp = 0;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Temperature: ");
  Serial.print(temp, 1);
  Serial.println(" C");

  digitalWrite(LED_OPERATION, HIGH); // Show movement in progress

  if (distance < 15 && !isOpen) {
    // Open Dustbin
    myServo.write(90);
    digitalWrite(LED_STATUS, HIGH); // Dustbin is open
    isOpen = true;
    displayStatus("OPENING", temp);
    Blynk.virtualWrite(V1, "Dustbin Opened");
    Blynk.virtualWrite(V2, temp);
  } 
  else if (distance >= 15 && isOpen) {
    // Close Dustbin
    myServo.write(0);
    digitalWrite(LED_STATUS, LOW); // Dustbin is closed
    isOpen = false;
    displayStatus("CLOSING", temp);
    Blynk.virtualWrite(V1, "Dustbin Closed");
    Blynk.virtualWrite(V2, temp);
  } 
  else if (distance >= 15 && !isOpen) {
    // Idle State
    displayStatus("SMART DUSTBIN", temp);
    Blynk.virtualWrite(V2, temp);
  }

  delay(500);
  digitalWrite(LED_OPERATION, LOW); // Movement complete
}

void displayStatus(String status, float temp) {
  if (lastStatus != status) { // Update LCD only if status changes
    lcd.clear();
    lcd.setCursor((16 - status.length()) / 2, 0);
    lcd.print(status);
    lastStatus = status;
  }
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print("C   ");
}
