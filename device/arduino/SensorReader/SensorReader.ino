/*
 * Arduino firmware for a basic weather station for an astronomical observatory. 
 * Sends sensor readings from a MLX90614 IR sensor and a rain sensor.
 * Sends firmata messages as json strings once per second.
 * 
 * Uses Adafruit's Adafruit_MLX90614 library and wiring, see https://learn.adafruit.com/using-melexis-mlx90614-non-contact-sensors/wiring-and-test
 *
 * PARTS:
 * 1 x Arduino nano (http://www.dx.com/p/arduino-nano-v3-0-81877#.ViKE2RCrRE4)
 * 1 x RG-11 Rain sensor
 * 1 x MLX90614 IR temp sensor 
 * 
 * Wiring:
 * Rain sensor: 
 *      Relay is attached to pin 8 and acts as a simple on off switch. Ensure you wire with a pull-down resistor. If you dont know what that means goodle 'arduino button' to see how a basic switch/button is wired.
 * MLX90614
 *      Wired to A4 and A5 with 2 pull up resistors (see https://learn.adafruit.com/using-melexis-mlx90614-non-contact-sensors/wiring-and-test)
 *      
 * Sample json message
 *      {"rain":false,"sky":20.93,"ambient":21.93}
 *
 */
#include <Firmata.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <stdarg.h>

#define RAIN_SENSOR_PIN 8

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

/**
 * Startup firmata and the mlx
 */
void setup() {
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.begin(57600);
  pinMode(8,INPUT);
  mlx.begin();  
}

/**
 * Once per second take readings from sensors, build a json string and send over firmata.
 */
void loop()
{
  char charBuf[50];
  String message = "{\"rain\":";
  if(digitalRead(RAIN_SENSOR_PIN)==HIGH) {
    message += "true,";
  } else {
    message += "false,";
  }
  message += "\"sky\":";
  message += mlx.readObjectTempC();
  message += ",\"ambient\":";
  message += mlx.readAmbientTempC();
  message += "}";  
  message.toCharArray(charBuf, 50);
  Firmata.sendString(charBuf);  
  delay(1000);
}



