#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int pump1 = 27;
int pump2 = 21;

int _moisture,sensor_analog;
const int sensor_pin = 32;  /* Soil moisture sensor O/P pin */


// define OLED
#define SDA_PIN 19
#define SCL_PIN 22
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Voltage sensor
#define ANALOG_IN_PIN 34
#define ADC_SAMPLES 20

float adc_voltage = 0.0;
float in_voltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float calibration_factor = 1.21; // adjust once using multimeter
int adc_value = 0;

float batteryVoltage = 0.0;
int batteryPercent = 0;

// Ultrasonic sensor
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
const int trigPin = 5;
const int echoPin = 18;
long duration;
float distanceCm;
float distanceInch;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Starts the serial communication

  // Ultrasonic sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Pump
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);

  // Voltage sensor
  analogReadResolution(12);
  analogSetPinAttenuation(ANALOG_IN_PIN, ADC_11db);
}

float readBatteryVoltage() {
  long adc_sum = 0;

  for (int i = 0; i < ADC_SAMPLES; i++) {
    adc_sum += analogRead(ANALOG_IN_PIN);
    delay(2);
  }

  float adc_avg = adc_sum / (float)ADC_SAMPLES;

  float adc_voltage = (adc_avg * 3.3) / 4095.0;
  float voltage = adc_voltage * (R1 + R2) / R2;
  voltage *= calibration_factor;

  return voltage;
}

int batteryPercentage(float voltage) {
  if (voltage >= 8.4) return 100;
  if (voltage <= 6.6) return 0;

  // Piecewise approximation (much better than linear)
  if (voltage > 8.0)
    return map(voltage * 100, 800, 840, 75, 100);
  else if (voltage > 7.6)
    return map(voltage * 100, 760, 800, 40, 75);
  else if (voltage > 7.2)
    return map(voltage * 100, 720, 760, 15, 40);
  else
    return map(voltage * 100, 660, 720, 0, 15);
}

void loop() {
  // put your main code here, to run repeatedly:
  /**
  This is the code for pump
  
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, HIGH);
  delay(3000);

  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);
  delay(3000);
  */

  /**
  This is the code for ultrasonic sensor
  */
    digitalWrite(trigPin, LOW);// Clears the trigPin
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);// Sets the trigPin on HIGH state for 10 micro seconds
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);// Clears the trigPin
    duration = pulseIn(echoPin, HIGH);// Reads the echoPin, returns the sound wave travel time in microseconds
    distanceCm = duration * SOUND_SPEED/2; // Calculate the distance
    distanceInch = distanceCm * CM_TO_INCH; // Convert to inches
    Serial.print("Distance (cm): ");// Prints the distance in the Serial Monitor
    Serial.println(distanceCm);
    Serial.print("Distance (inch): ");
    Serial.println(distanceInch);

    
    /**
    This is the code for moisture sensor
    */
    sensor_analog = analogRead(sensor_pin);
    _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
    Serial.print("Moisture = ");
    Serial.print(_moisture);  /* Print Temperature on the serial window */
    Serial.println("%");


    /**
    This is the code for voltage sensor
    */
    batteryVoltage = readBatteryVoltage();
    batteryPercent = batteryPercentage(batteryVoltage);
    
    Serial.print("Battery Voltage = ");
    Serial.print(batteryVoltage, 2);
    Serial.print(" V | Battery = ");
    Serial.print(batteryPercent);
    Serial.println(" %");

    /**
    Display sensor readings on OLED
    */
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    // Display voltage sensor reading
    display.print("Voltage: ");
    display.print(batteryVoltage, 2);
    display.println(" V");
    
    // Display battery percentage
    display.print("Battery: ");
    display.print(batteryPercent);
    display.println(" %");
    
    // Display moisture sensor reading
    display.print("Moisture: ");
    display.print(_moisture, 1);
    display.println(" %");
    
    // Display ultrasonic sensor reading
    display.print("Distance: ");
    display.print(distanceCm, 1);
    display.println(" cm");
    
    display.display();
    
    delay(1000);
}
