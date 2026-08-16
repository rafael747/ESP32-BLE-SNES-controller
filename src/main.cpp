#include <Arduino.h>
#include <GameControllers.h>
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

const int LATCH_PIN = 25;
const int CLOCK_PIN = 32;
const int DATA_PIN_0 = 27; 

const int BATT_PIN = 35;

// Battery thresholds in Volts
const float fullBattery = 4.1;  // e.g., fully charged Li-ion
const float emptyBattery = 3.3; // e.g., depleted battery

int delayToSleep = 0;

GameControllers controllers;
BleGamepad bleGamepad("ESP32 SNES Gamepad", "Espressif", 100);

// Custom map function that bounds percentages between 0 and 100
float mapConstrain(float x, float in_min, float in_max) {
  if (x < in_min) return 0;
  if (x > in_max) return 100;
  return (x - in_min) * 100.0 / (in_max - in_min);
}

void setup() {
  Serial.begin(115200);

  analogSetAttenuation(ADC_11db);
  float voltage = analogReadMilliVolts(BATT_PIN) /1000 * 2 ;
  float batteryPercentage = mapConstrain(voltage, emptyBattery, fullBattery);
  Serial.print("battery percentage: ");
  Serial.println(batteryPercentage);
  Serial.print("battery voltage: ");
  Serial.println(voltage);

  Serial.println("Starting BLE gamepad");

  controllers.init(LATCH_PIN, CLOCK_PIN); 
  controllers.setController(0, GameControllers::SNES, DATA_PIN_0);

  bleGamepad.begin();
  bleGamepad.setBatteryLevel(batteryPercentage);
}

void handleButtonPress(int controllerIndex, GameControllers::Button button, int bleButton) {
  if (controllers.down(controllerIndex, button)) {
    bleGamepad.press(bleButton);
  } else {
    bleGamepad.release(bleButton);
  }
  if (button == GameControllers::START){
    if(controllers.down(controllerIndex, button)){
      if(delayToSleep > 2000){
        //sleep
        Serial.printf("Should sleep now");
        esp_deep_sleep_start(); 
      }
      delayToSleep += 1;
    } else {
        delayToSleep = 0;
    }
  }
}

void handleDPadPress() {
  if (controllers.down(0, GameControllers::UP)) {
    bleGamepad.setHats(DPAD_UP);
  } else if (controllers.down(0, GameControllers::DOWN)) {
    bleGamepad.setHats(DPAD_DOWN);
  } else if (controllers.down(0, GameControllers::LEFT)) {
    bleGamepad.setHats(DPAD_LEFT);
  } else if (controllers.down(0, GameControllers::RIGHT)) {
    bleGamepad.setHats(DPAD_RIGHT);
  } else {
    bleGamepad.setHats(DPAD_CENTERED);
  }
}

void loop() {
  controllers.poll(); //read all controllers at once


  if (!bleGamepad.isConnected())
    {
      Serial.println("BLE gamepad not connected");
      delay(500);
      return;
    }

  // A/B/X/Y buttons
  handleButtonPress(0, GameControllers::A, BUTTON_2);
  handleButtonPress(0, GameControllers::B, BUTTON_1);
  handleButtonPress(0, GameControllers::X, BUTTON_4);
  handleButtonPress(0, GameControllers::Y, BUTTON_3);

  // L1/R1 buttons
  handleButtonPress(0, GameControllers::L, BUTTON_7);
  handleButtonPress(0, GameControllers::R, BUTTON_8);

  // Start/Select buttons
  handleButtonPress(0, GameControllers::START, BUTTON_10);
  handleButtonPress(0, GameControllers::SELECT, BUTTON_9);

  // DPAD
  handleDPadPress();
}