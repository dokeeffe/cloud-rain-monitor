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
        # super(Comm,self).__init__(Comm.NAME)
        self.board = Arduino(port)
        # start an iterator thread so that serial buffer doesn't overflow
        it = util.Iterator(self.board)
        it.start()
        self.board.add_cmd_handler(pyfirmata.pyfirmata.STRING_DATA, self._messageHandler)

    def _messageHandler(self, *args, **kwargs):
        conn = sqlite3.connect('weather_sensor.db')
        c = conn.cursor()
        readings = json.loads(util.two_byte_iter_to_str(args))
        sky_temperature = readings['sky']
        ambient_temperature = readings['ambient']
        rain = readings['rain']
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