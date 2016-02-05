/**
 * Arduino firmware for a basic weather station for an astronomical observatory. 
 * Monitors temperature & humidity using a DHT11 sensor, rain using an RG-11 rain sensor and clouds using an MLX90614 IR temperature sensor to measure sky temperature.
 * 
 * Provides an HTTP interface.
 *
 * Uses the ethercard library by Jean-Claude Wippler (https://github.com/jcw/ethercard) You will need to install this in your arduino IDE to flash this firmware. See instrunctions in the ethercard github project page.
 * NOTE you must enable format $T by uncommenting '#define FLOATEMIT' in the library. See http://jeelabs.net/pub/docs/ethercard/classBufferFiller.html
 * 
 * Uses the DHT library https://github.com/adafruit/DHT-sensor-library
 * 
 * Uses Adafruit's Adafruit_MLX90614 library and wiring, see https://learn.adafruit.com/using-melexis-mlx90614-non-contact-sensors/wiring-and-test
 *
 * PARTS:
 * 1 x Arduino nano (http://www.dx.com/p/arduino-nano-v3-0-81877#.ViKE2RCrRE4)
 * 1 x RG-11 Rain sensor
 * 1 x DHT11 temp&humidity sensor
 * 1 x MLX90614 IR temp sensor 
 * 
 * Wiring:
 *     Arduino -- Ethernet module:
 *      Pin 8 -- CS
 *      Pin 11 -- ST
 *      Pin 12 -- SO
 *      Pin 13 -- SCK
 *      5v -- 5v
 *      GND -- GND
 *     Arduino -- DHT 11
 *      Pin 2 -- DHT
 *      GND -- GND
 *      5v -- vvv
 *     Rain sensor
 *      Pin 4 -- trigger
 *      GND -- GND
 *      5v -- vcc
 *     MLX90614
 *      Wired to A4 and A5 with 2 pull up resistors (see https://learn.adafruit.com/using-melexis-mlx90614-non-contact-sensors/wiring-and-test)
 *
 *  Example GET request:
 *    Request: GET http://192.168.1.204/weather
 *
 **/
#include <EtherCard.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#define BUFFERSIZE 350

//HTTP responses
const char WEATHER_RESPONSE[] PROGMEM = "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\n"
                                          "Pragma: no-cache\r\n\r\n" 
                                          "{\n"
                                          "\t\"uptime\":\"$D$D:$D$D:$D$D\",\n"
                                          "\t\"insideTemp\":$T,\n"
                                          "\t\"insideHumidity\":$T,\n"
                                          "\t\"dewPoint\":$T,\n"
                                          "\t\"skyTemp\":$T,\n"
                                          "\t\"outsideTemp\":$T,\n"
                                          "\t\"rain\":$S\n"
                                          "}";                             
const char BADREQUEST_RESPONSE[] PROGMEM = "HTTP/1.0 400 Bad Request";
const char NOTFOUND_RESPONSE[] PROGMEM = "HTTP/1.0 404 Not Found";

#define DHTPIN 4     // DHT11 sensor
#define DHTTYPE DHT11   // DHT11 sensor type
#define RAINPIN 2 //rain sensor
#define SAMPLE_INTERVAL 2000 //how often to take a reading.
static long lastSampleTime = 0;

static double latestTempReading;
static double latestHumidityReading;
static double latestDewPoint;
static double latestSkyTemp;
static double previousSkyTemp;
static double latestOutsideTemp;
static double previousOutsideTemp;
static boolean latestRainReading;
static boolean previousRainReading;
static boolean changeInConditions;

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2E, 0x30, 0x31 };
static byte myip[] = { 192, 168, 1, 204 }; //STATIC IP address. Change this for your network setup. 
static byte gwip[] = { 192,168,1,1 }; //Change this for your network setup. 
static byte dns[] = {192,168,1,1};

//ethernet buffer config
byte Ethernet::buffer[BUFFERSIZE];
BufferFiller bfill;
Stash stash;

//IR temp sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
  }
  ether.staticSetup(myip,gwip,dns);
//  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
//    Serial.println(F("Failed to access Ethernet controller"));
//  if (!ether.dhcpSetup())
//    Serial.println(F("DHCP failed")); 
  mlx.begin();  
}

/**
 * Main loop. Monitor for HTTP requests and sample sensor data every SAMPLE_INTERVAL. 
 */
void loop() {
  if (millis()-lastSampleTime > SAMPLE_INTERVAL) {
    sample();
    lastSampleTime = millis();
  }
  if (changeInConditions) {
    //TODO: Fire an event / message to a message bus
    changeInConditions = false;
  }
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (pos)  {
    bfill = ether.tcpOffset();
    char* data = (char *) Ethernet::buffer + pos;
    Serial.println(data);
    String stringData = String(data);
    if (!stringData.startsWith("GET /weather")) {
      ether.httpServerReply(notFound());
    } else {
      ether.httpServerReply(weatherResponse());
    }
  }
}

/**
 * Build the HTTP response for the weather json object.
 */
static word weatherResponse() {
  long t = millis() / 1000;
  word hh = t / 3600;
  byte mm = (t / 60) % 60;
  byte ss = t % 60;
  char *raining = "false";
  if(latestRainReading) {
     raining = "true";
  }
  bfill = ether.tcpOffset();
  bfill.emit_p(WEATHER_RESPONSE,
               hh / 10, hh % 10, mm / 10, mm % 10, ss / 10, ss % 10, 
               latestTempReading, latestHumidityReading, latestDewPoint, 
               latestSkyTemp,latestOutsideTemp,raining);
  return bfill.position();
}

/**
 * Build the 404 HTTP response
 */
static word notFound() {
  bfill = ether.tcpOffset();
  bfill.emit_p(NOTFOUND_RESPONSE);
  return bfill.position();
}

/**
 * Take readings from sensors
 */
void sample() {    
  // Reading temperature or humidity takes about 250 milliseconds!
  double h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  double t = dht.readTemperature();
  //float dp = dewPoint(t,h);
  boolean rain = !digitalRead(RAINPIN);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  latestTempReading = t;
  latestHumidityReading = h;
  latestRainReading = rain;
  latestSkyTemp = mlx.readObjectTempC();
  latestOutsideTemp = mlx.readAmbientTempC();
  latestDewPoint = dewPointFast(latestTempReading,latestHumidityReading);
  changeInConditions = detectChangeInConditions();
}

/**
 * Dew point calc. Reference: http://en.wikipedia.org/wiki/Dew_point
 */
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

/**
 * If there is a change of more than 1degree C in any temp reading from the external sensor between samples or if a change in the rain reading then return true.
 * Ignore the internal DHT11 sensor (temp&humidity)
 */
boolean detectChangeInConditions() {
  boolean result = false;
  if(previousRainReading != latestRainReading || abs(previousSkyTemp-latestSkyTemp) > 1 || abs(previousOutsideTemp-latestOutsideTemp) > 1) {
    result = true;
    Serial.println("Change in conditions");
  }
  previousSkyTemp = latestSkyTemp;
  previousOutsideTemp = latestOutsideTemp;
  previousRainReading = latestRainReading;
  return result;
}



