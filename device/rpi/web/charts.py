import os
import matplotlib
import sqlite3
matplotlib.use('Agg')
from matplotlib import pyplot as plt
import pandas as pd
import math 

class ChartGenerator:

    NAME = 'ChartGenerator'
    HISTORICAL_DATA_SQL = "SELECT rain,sky_temperature, ambient_temperature, date_sensor_read FROM weather_sensor WHERE date_sensor_read >= date('now','-240 hour')"
    MIN_SKY_TEMP_HISTORY =  "SELECT min(sky_temperature) as min_sky_temperature FROM weather_sensor WHERE date_sensor_read >= date('now','-10 day')"

    def __init__(self, root_dir):
        self.root_dir = root_dir
        plt.rcParams["figure.figsize"] = (10, 3)
        plt.style.use('bmh')
        self._conn = sqlite3.connect('weather_sensor.db')


    def _calculate_cloud_cover(self, sky, ambient, recent_clear_sky_temp):
        '''
        :param sky:
        :param ambient:
        :param recent_clear_sky_temp
        :return:
        '''
        scaling_factor = 100 / abs(recent_clear_sky_temp)
        x = abs(((sky * -1) * scaling_factor) - 100)
        if x < 0:
            return 0
        elif x > 100:
            return 100
        else:
            return x

    def generate_cloud_chart(self):
        weather = self._last_24hrs_data()
        recent_clear_sky_temp = self._min_sky_last_month()
        weather['CloudCover'] = weather.apply(lambda x: self._calculate_cloud_cover(x['sky_temperature'], x['ambient_temperature'], recent_clear_sky_temp), axis=1)
        weather['Rain'] = weather[['rain']]*100
        rain_clouds = weather.loc[:,['Rain','CloudCover']]
        rain_clouds.resample('5T').mean().plot();
        ax = rain_clouds.plot();
        ax.set_ylabel("%")
        plt.savefig(os.path.join(self.root_dir, './cloud.png'), bbox_inches='tight')
        plt.clf()
        plt.close()

    def _last_24hrs_data(self):
        conn = sqlite3.connect('weather_sensor.db')
        with conn:
            weather = pd.read_sql(self.HISTORICAL_DATA_SQL, self._conn)
            weather = weather.rename(columns={'date_sensor_read': 'Time'})
            weather.index = pd.to_datetime(weather.Time)
            return weather

    def _min_sky_last_month(self):
        conn = sqlite3.connect('weather_sensor.db')
        with conn:
            cursor=conn.cursor()
            cursor.execute(self.MIN_SKY_TEMP_HISTORY)
            mintemp = cursor.fetchone()[0]
            if mintemp > -5:
                return -20 #-20 is a good guess for 100% clear sky
            else:
                return mintemp

    def generate_temperature_chart(self):
        weather = self._last_24hrs_data()
        temperatures = weather.loc[:, ['sky_temperature','ambient_temperature']]
        ax = temperatures.plot();
        ax.set_ylabel("Temp Deg C")
        plt.savefig(os.path.join(self.root_dir, './temperature.png'), bbox_inches='tight')
        plt.clf()
        plt.close()
