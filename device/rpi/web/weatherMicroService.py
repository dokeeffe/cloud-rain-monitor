#!/usr/bin/env python

import sqlite3
from bottle import route, run, debug, error, request, static_file
from collector import Collector
from charts import ChartGenerator

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
    sky_temp = result[0][1]
    outside_temp = result[0][2]
    reading_timestamp = result[0][3]
    #rain = (result[0][0] == 1 and sky_temp - outside_temp > -20 and sky_temp > -10) #hack to prevent a true for rain when the sky is clear. Sometimes dew triggers a false positive for rain
    rain = result[0][0] == 1 
    return {'rain': rain, 'skyTemp': sky_temp, 'outsideTemp': outside_temp, 'readingTimestamp': reading_timestamp}

@route('/weather/history')
def historical_weather():
    '''
    Return historical weather data. Pass days=x as a qurey string arg to get the previous x days data.
    :return:
    '''
    conn = sqlite3.connect('weather_sensor.db')
    c = conn.cursor()
    days = request.query.days or 1
    c.execute("SELECT rain,sky_temperature, ambient_temperature, date_sensor_read FROM weather_sensor WHERE date_sensor_read >= date('now','-{} day')".format(days))
    result = c.fetchall()
    c.close()
    history = []
    count =0
    for row in result:
        count += 1
        history.append({'rain': row[0] == 1, 'skyTemp': row[1], 'outsideTemp': row[2], 'readingTimestamp' : row[3]})
    return {'count': count, 'history': history}

@route('/weather/chart/<chart>')
def cloud_chart(chart):
    if 'cloud.png' in chart:
        _chartGenerator.generate_cloud_chart()
    else:
        _chartGenerator.generate_temperature_chart()
    return static_file(chart, root='/tmp')

@route('/weather/safety')
def safety():
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
    raining = (result[0][0] == 1 and sky_temp - outside_temp > -20 and sky_temp > -10) #hack to prevent a true for rain when the sky is clear. Sometimes dew triggers a false positive for rain
    roof_status = {'open_ok': 1, 'reasons': ''}
    if sky_temp - outside_temp > -17:
        roof_status = {'open_ok': 0, 'reasons': 'Cloudy'}
    if raining:
        roof_status = {'open_ok': 0, 'reasons': 'Raining'}
    return {'timestamp_utc': reading_timestamp, 'roof_status': roof_status}


_chartGenerator = ChartGenerator('/tmp')
con = sqlite3.connect('weather_sensor.db')
con.execute("CREATE TABLE IF NOT EXISTS weather_sensor (id INTEGER PRIMARY KEY, rain bool NOT NULL, sky_temperature NUMBER NOT NULL, ambient_temperature NUMBER NOT NULL, date_sensor_read DATETIME DEFAULT CURRENT_TIMESTAMP)")
con.commit()
collector = Collector('/dev/ttyACM0')
run(host='0.0.0.0', port=8080)
