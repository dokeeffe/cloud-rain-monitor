# cloud-rain-monitor

Arduino firmware and indi-driver for a device to measure rain and cloud cover for an astronomical observatory. Cloud cover is determined by measuring the difference between ambient temperature and IR radiation from the sky.

![example measurements](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/d3-graph/example.png "Example measurements")


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
![MLX90614](http://www.melexis.com/prodfiles/0005150_IR_sensor.jpg?sNU=83173.3492981)

Rain is detected using an [RG-11 rain sensor](http://rainsensors.com/) 
![rg-11](http://hydreon.com/wp-content/uploads/sites/3/2015/rg_wht_sm.jpg)

An [Indi Driver](http://www.indilib.org/devices/weather-stations.html) for observatory automation is also included.

![indi-driver](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/indi-driver/docs/indi.png)


The device and driver is currently used in Ballyhoura Observatory

![observatory](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/indi-driver/docs/obs.jpeg)

