# cloud-rain-monitor

Arduino firmware for a device to measure rain and cloud cover for an astronomical observatory located in Ireland.

The device offers an HTTP interface to GET current conditions as well as posting messages to http://dweet.io.

Example request to /weather
```
{
	"uptime":"01:20:58",
	"insideTemp":    8.000,
	"insideHumidity":    41.000,
	"dewPoint":    3.504,
	"skyTemp":    -11.710,
	"outsideTemp":    8.770,
	"rain":false
}
```

Clouds are detected using an Melexis [MLX90614](http://www.melexis.com/Infrared-Thermometer-Sensors/Infrared-Thermometer-Sensors/MLX90614-615.aspx) sensor to measure IR radiation from the sky. 

Rain is detected using a [cheap rain sensor](http://www.dx.com/p/cg05sz-063-rain-sensor-for-arduino-black-silver-works-with-official-arduino-boards-266534#.VmdKgt_hBE4) 

An [Indi Driver](http://www.indilib.org/devices/weather-stations.html) for observatory automation is currently a work in progress
