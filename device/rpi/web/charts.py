import os

import matplotlib
import sqlite3
matplotlib.use('Agg')
from matplotlib import pyplot as plt
import pandas as pd

class ChartGenerator:

    NAME = 'ChartGenerator'
    SQL = "SELECT rain,sky_temperature, ambient_temperature, date_sensor_read FROM weather_sensor WHERE date_sensor_read >= date('now','-1 day')"

    def __init__(self, root_dir):
        self.root_dir = root_dir
        plt.style.use('fivethirtyeight')


    def _calculate_cloud_cover(self, sky, ambient):
        cc = ((sky+20)*7)-ambient
        if cc > 100:
            cc = 100
        if cc < 10:
            cc = 0
        return cc

    def generate_cloud_chart(self):
        conn = sqlite3.connect('weather_sensor.db')
        weather = pd.read_sql(self.SQL, conn)
        conn.close()
        weather.index = pd.to_datetime(weather.date_sensor_read)
        weather['CloudCover'] = weather.apply(lambda x: self._calculate_cloud_cover(x['sky_temperature'], x['ambient_temperature']), axis=1)
        weather['Rain'] = weather[['rain']]*100
        rain_clouds = weather.loc[:,['Rain','CloudCover']]
        matplotlib.rcParams['figure.figsize'] = (6.0, 4.0)
        rain_clouds.resample('H').mean().plot();
        plt.savefig(os.path.join(self.root_dir, './cloud.png'))
