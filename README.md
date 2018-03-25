# cloud-rain-monitor

An Arduino firmware, Raspberry PI application and indi-driver for a device to measure rain and cloud cover for an astronomical observatory. Cloud cover is determined by measuring the difference between ambient temperature and sky temperature.

**Outside**

![](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/readme-files/weatherstation1.png)

**Inside**

![](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/readme-files/weatherstation2.png)

The device offers an HTTP interface to GET current conditions. See below for the full API 

Example request to /weather
```
{
    "outsideTemp": 2.75, 
    "skyTemp": -21.77, 
    "readingTimestamp": 
    "2018-03-25 18:02:46", 
    "rain": false
}
```

Clouds are detected using an Melexis [MLX90614](http://www.melexis.com/Infrared-Thermometer-Sensors/Infrared-Thermometer-Sensors/MLX90614-615.aspx) sensor to measure IR radiation from the sky. 
![MLX90614](https://www.melexis.com/-/media/images/product-media/mlx90614/mlx90614-infrared-thermometer-melexis.jpg?h=275&w=340&hash=327FA5D17A6484712BE79EDAE1A8D6282C376334)

Rain is detected using an [RG-11 rain sensor](http://rainsensors.com/) 
![rg-11](http://hydreon.com/wp-content/uploads/sites/3/2015/rg_wht_sm.jpg)

An [Indi Driver](http://www.indilib.org/devices/weather-stations.html) for observatory automation is also included.

![indi-driver](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/indi-driver/docs/indi.png)


The device and driver is currently used in Ballyhoura Observatory

![observatory](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/indi-driver/docs/obs.jpeg)

# HTTP API

**Current weather status**
----

* **URL**

  /weather/current

* **Method:**
  
  <_The request type_>

  `GET`
  
* **Success Response:**
  
  * **Code:** 200 <br />
    **Content:** `{"outsideTemp": 2.75, "skyTemp": -21.77, "readingTimestamp": "2018-03-25 18:02:46", "rain": false}`

**Historical weather**
----

Returns history for the previous 24 hours

* **URL**

  /weather/history

* **Method:**

  <_The request type_>

  `GET`

* **Success Response:**

  * **Code:** 200 <br />
    **Content:** `[{"outsideTemp": 2.75, "skyTemp": -21.77, "readingTimestamp": "2018-03-25 18:02:46", "rain": false}, {"outsideTemp": 2.75, "skyTemp": -21.77, "readingTimestamp": "2018-03-25 18:02:46", "rain": false}....]`


**Charts**
----

Returns an image weather chart

* **URL**

  /weather/chart/<chart>

* **Method:**

  <_The request type_>

  `GET`

*  **URL Params**

   **Required:**
 
   `<chart>`

   Valid values are 'cloud.png' and 'temperature.png'

* **Sample Call:**

    http://192.168.2.227:8080/weather/chart/cloud.png

* **Success Response:**

  * **Code:** 200 <br />
    **Content:** 

    ![example measurements](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/readme-files/temperature.png "Example measurements")
 
    ![example measurements](https://raw.githubusercontent.com/dokeeffe/cloud-rain-monitor/master/readme-files/cloud.png "Example measurements")



