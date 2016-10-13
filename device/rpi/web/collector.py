from pyfirmata import Arduino, util
import pyfirmata
import sqlite3
import json

class Collector():
    """
    Handles the communication to arduino SensorReader
    The arduino firmware publishes messages using firmata once per second with sensor readings.
    This class is responsible for collection and persistance of readings.
    AttachTo: ""
    """

    NAME = "Collector"

    def __init__(self, port):
        self.last_rain_reading = False;
        self.last_sky_temperature_reading = 0;
        self.last_ambient_temperature_reading = 0;
        self.board = Arduino(port)
        # start an iterator thread so that serial buffer doesn't overflow
        it = util.Iterator(self.board)
        it.start()
        self.board.add_cmd_handler(pyfirmata.pyfirmata.STRING_DATA, self._messageHandler)

    def _messageHandler(self, *args, **kwargs):
        readings = json.loads(util.two_byte_iter_to_str(args))
        sky_temperature = readings['sky']
        ambient_temperature = readings['ambient']
        rain = readings['rain']
        if self.conditions_changed(sky_temperature,ambient_temperature,rain):
            conn = sqlite3.connect('weather_sensor.db')
            c = conn.cursor()
            c.execute("INSERT INTO weather_sensor (rain,sky_temperature,ambient_temperature) VALUES (?,?,?)", (rain, sky_temperature,ambient_temperature))
            # new_id = c.lastrowid
            conn.commit()
            c.close()

    def update(self):
        super(Collector,self).update()

    def writeData(self,data):
        self.board.send_sysex(pyfirmata.pyfirmata.STRING_DATA,data)

    def dispose(self):
        super(Collector,self).dispose()
        try:
            self.board.exit()
        except AttributeError:
            print "exit() raised an AttributeError unexpectedly!"+self.toString()

    def latestWeatherStatus(self):
        return self.sky_temperature

    def conditions_changed(self, sky_temperature, ambient_temperature, rain):
        return True