from pyfirmata import Arduino, util
import pyfirmata
import sqlite3
import json
import time


class Collector():
    """
    Handles the communication to arduino SensorReader
    The arduino firmware publishes messages using firmata once per second with sensor readings.
    This class is responsible for collection and persistance of readings.
    AttachTo: ""
    """

    NAME = "Collector"

    def __init__(self, port):
        self.last_rain_reading_saved = None;
        self.last_sky_temperature_reading_saved = 0;
        self.last_ambient_temperature_reading_saved = 0;
        self.last_reading_saved_time = time.time()
        self.board = Arduino(port)
        # start an iterator thread so that serial buffer doesn't overflow
        it = util.Iterator(self.board)
        it.start()
        self.board.add_cmd_handler(pyfirmata.pyfirmata.STRING_DATA, self._messageHandler)

    def _messageHandler(self, *args, **kwargs):
        '''
        Calback method envoked by the firmata library. Handles the string message sent from the arduino.
        Grabs the sensor data from the message and persists in the DB table.
        :param args:
        :param kwargs:
        :return:
        '''
        readings = json.loads(util.two_byte_iter_to_str(args))
        sky_temperature = readings['sky']
        ambient_temperature = readings['ambient']
        rain = readings['rain']
        if self.should_persist_sensor_reading(sky_temperature, ambient_temperature, rain):
            conn = sqlite3.connect('weather_sensor.db')
            c = conn.cursor()
            c.execute("INSERT INTO weather_sensor (rain,sky_temperature,ambient_temperature) VALUES (?,?,?)",
                      (rain, sky_temperature, ambient_temperature))
            # new_id = c.lastrowid
            conn.commit()
            self.last_rain_reading_saved = rain
            self.last_sky_temperature_reading_saved = sky_temperature
            self.last_ambient_temperature_reading_saved = ambient_temperature
            c.close()

    def update(self):
        super(Collector, self).update()

    def writeData(self, data):
        self.board.send_sysex(pyfirmata.pyfirmata.STRING_DATA, data)

    def dispose(self):
        super(Collector, self).dispose()
        try:
            self.board.exit()
        except AttributeError:
            print "exit() raised an AttributeError unexpectedly!" + self.toString()


    def should_persist_sensor_reading(self, sky_temperature, ambient_temperature, rain):
        '''
        Returns true if the sensor data should be persisted in the DB.
        The rules are if any temp change is more than 1 degree, or the rain state changes
        or 60sec have elapsed since the last save
        :param sky_temperature:
        :param ambient_temperature:
        :param rain:
        :return:
        '''
        if abs(sky_temperature - self.last_sky_temperature_reading_saved) > 1 or abs(
                        ambient_temperature - self.last_ambient_temperature_reading_saved) > 1 or rain != self.last_rain_reading_saved or time.time() - self.last_reading_saved_time > 60:
            return True
        else:
            return False
