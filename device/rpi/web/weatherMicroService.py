import sqlite3
from bottle import route, run, debug, error

from collector import Collector

'''
A very simple python bottle micro service for weather
'''

@route('/weather/current')
def current_weather():
    '''
    GET the current weather state. the latest reading persisted in the DB.
    TODO: If the reading is older than 5min then return 404?
    :return:
    '''
    conn = sqlite3.connect('weather_sensor.db')
    c = conn.cursor()
    c.execute("SELECT rain,sky_temperature,ambient_temperature, date_sensor_read FROM weather_sensor order by id desc limit 1")
    result = c.fetchall()
    c.close()
    return {'rain': result[0][0]==1, 'skyTemp': result[0][1], 'outsideTemp': result[0][2], 'readingTimestamp': result[0][3]}

@route('/weather')
def current_weather():
    '''
    Return all the weather data recorded
    TODO: pagination or trim the data to 1 day
    :return:
    '''
    conn = sqlite3.connect('weather_sensor.db')
    c = conn.cursor()
    c.execute("SELECT rain,sky_temperature, ambient_temperature, date_sensor_read FROM weather_sensor")
    result = c.fetchall()
    c.close()
    history = []
    count =0
    for row in result:
        count += 1
        history.append({'rain': row[0] == 1, 'skyTemp': row[1], 'outsideTemp': row[2], 'readingTimestamp' : row[3]})
    return {'count': count, 'history': history}


con = sqlite3.connect('weather_sensor.db')
con.execute("DROP TABLE IF EXISTS weather_sensor")
con.execute("CREATE TABLE weather_sensor (id INTEGER PRIMARY KEY, rain bool NOT NULL, sky_temperature NUMBER NOT NULL, ambient_temperature NUMBER NOT NULL, date_sensor_read DATETIME DEFAULT CURRENT_TIMESTAMP)")
con.commit()
collector = Collector('/dev/ttyACM0')
run(host='0.0.0.0', port=8080)
# remember to remove reloader=True and debug(True) when you move your
# application from development to a productive environment