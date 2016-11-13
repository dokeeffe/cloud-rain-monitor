#!/usr/bin/env python

import sqlite3
from bottle import route, run, debug, error

from collector import Collector

'''
A very simple python bottle micro service for weather
'''


def estimate_cloud_cover(sky_temp, outside_temp):
    '''
    PURE guesswork currently!!! Will update this when more data collected
    :param sky_temp:
    :param outside_temp:
    :return:
    '''
    if sky_temp < -12:
        return 0
    elif sky_temp > -12 and sky_temp < -10:
        return 10
    elif sky_temp > -10 and sky_temp < -5:
        return 75
    elif sky_temp > -5:
        return 100


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
    sky_temp = result[0][1]
    outside_temp = result[0][2]
    reading_timestamp = result[0][3]
    rain = result[0][0] == 1
    cloud_cover = estimate_cloud_cover(float(sky_temp), float(outside_temp))
    return {'rain': rain, 'skyTemp': sky_temp, 'outsideTemp': outside_temp, 'readingTimestamp': reading_timestamp, 'cloudCover':cloud_cover}

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
