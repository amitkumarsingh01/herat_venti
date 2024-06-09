#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Define the I2C address for the LCD
#define I2C_ADDR 0x27

// Define the pins for the potentiometer and switch
const int agePotPin = 34;   // Age group potentiometer
const int switchPin = 2;    // Switch pin

// Define DHT sensor
#define DHTPIN 18     // DHT sensor pin
#define DHTTYPE DHT11 // DHT type (DHT11 or DHT22)

// Define servo pin
const int servoPin = 14;

// Define LCD dimensions
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// WiFi credentials
const char* ssid = "Project";
const char* password = "12345678";

// ThingSpeak API key
const char* apiKey = "XM0TUF2DL1KVCBNG";

// Initialize the LCD
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

// Initialize the Servo
Servo myservo;

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize I2C communication for the LCD
  Wire.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize servo
  myservo.attach(servoPin);

  // Initialize DHT sensor
  dht.begin();

  // Set switch pin as input
  pinMode(switchPin, INPUT_PULLDOWN);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Print initial messages
  lcd.setCursor(0, 0);
  lcd.print("Age:          ");
}

void loop() {
  // Read value from potentiometer
  int agePotValue = analogRead(agePotPin);

  // Read state of the switch
  int switchState = digitalRead(switchPin);

  // Print analog value and switch state to serial monitor
  Serial.print("Age Pot Value: ");
  Serial.print(agePotValue);
  Serial.print("\tSwitch State: ");
  Serial.println(switchState);

  // Map agePotValue to age group
  int cat;
  if (agePotValue < 1000) {
    lcd.setCursor(5, 0);
    lcd.print("Infant       ");
    cat = 1;
  } else if (agePotValue > 3500 and agePotValue < 5000) {
    lcd.setCursor(5, 0);
    lcd.print("Adult        ");
    cat = 2;
  } else {
    lcd.setCursor(5, 0);
    lcd.print("Child        ");
    cat = 3;
  }

  // Display switch state on LCD and serial monitor
  lcd.setCursor(0, 1);
  if (switchState == HIGH) {
    lcd.print("Status: On   ");
    Serial.println("Switch Status: On");
    // Start servo based on the age category
    switch (cat) {
      case 1: // Infant
        myservo.write(75);
        delay(1000);
        myservo.write(0);
        delay(1000);
        break;
      case 3: // Child
        myservo.write(90);
        delay(750);
        myservo.write(0);
        delay(750);
        break;
      case 2: // Adult
        myservo.write(120);
        delay(500);
        myservo.write(0);
        delay(500);
        break;
    }

    // Read temperature and humidity from DHT sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Print the results to the serial monitor
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");

    // Send data to ThingSpeak
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = "http://api.thingspeak.com/update?api_key=" + String(apiKey) + "&field1=" + String(t) + "&field2=" + String(h);
      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("ThingSpeak response code: %d\n", httpCode);
      } else {
        Serial.printf("Error in sending data: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Error in WiFi connection");
    }

  } else {
    lcd.print("Status: Off  ");
    Serial.println("Switch Status: Off");
  }

  delay(500); // Adjust delay as needed
}