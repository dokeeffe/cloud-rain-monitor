/*******************************************************************************
  Copyright(c) 2016 Derek OKeeffe. All rights reserved.

  INDI driver for a Cloud and rain monitor for an astronomical observatory.
  The device hardware is arduino based and provides an HTTP api.
  Example GET request to http://192.168.1.204/weather
  Returns a 200 response like below
  {
        "uptime":"05:14:12",
        "skyTemp":     4.230,
        "outsideTemp":    10.090,
        "rain":false
  }
 
  This driver is responsible for providing weather state information to INDI clients. 
  The INDI client will decide what action to take (for example: initiate an observatory shutdown or startup process).

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#include <memory>
#include "cloudrainmonitor.h"
#include "gason.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include <curl/curl.h>

std::unique_ptr<IndiCloudRainMonitor> indicloudrainmonitor(new IndiCloudRainMonitor());

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void ISGetProperties(const char *dev)
{
        indicloudrainmonitor->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
        indicloudrainmonitor->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
        indicloudrainmonitor->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
        indicloudrainmonitor->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}
void ISSnoopDevice (XMLEle *root)
{
    indicloudrainmonitor->ISSnoopDevice(root);
}

IndiCloudRainMonitor::IndiCloudRainMonitor()
{
   setVersion(1,0);
}

IndiCloudRainMonitor::~IndiCloudRainMonitor()
{

}

const char * IndiCloudRainMonitor::getDefaultName()
{
    return (char *)"Indi Cloud Rain Monitor";
}

bool IndiCloudRainMonitor::Connect()
{
    DEBUG(INDI::Logger::DBG_SESSION, "CONNECTING.");
    if (httpEndpointT[0].text == NULL)
    {
        DEBUG(INDI::Logger::DBG_ERROR, "cloudrainmonitor HTTP API endpoint is not available. Set it in the options tab");
        return false;   
    }
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, httpEndpointT[0].text);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); //10 sec timeout
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK) {
            DEBUG(INDI::Logger::DBG_ERROR, "Connecttion to HTTP endpoint failed");
            DEBUG(INDI::Logger::DBG_ERROR, "Is the HTTP API endpoint correct? Set it in the options tab. Can you ping the weather device?");
            return false;
        }
        /* always cleanup */ 
        curl_easy_cleanup(curl);

        char srcBuffer[readBuffer.size()];
        strncpy(srcBuffer, readBuffer.c_str(), readBuffer.size());
        char *source = srcBuffer;
        // do not forget terminate source string with 0
        char *endptr;
        JsonValue value;
        JsonAllocator allocator;
        int status = jsonParse(source, &endptr, &value, allocator);
        if (status != JSON_OK)
        {
            DEBUGF(INDI::Logger::DBG_ERROR, "%s at %zd", jsonStrError(status), endptr - source);
            DEBUGF(INDI::Logger::DBG_DEBUG, "%s", readBuffer.c_str());
            return IPS_ALERT;
        }
        DEBUGF(INDI::Logger::DBG_DEBUG, "HTTP response %s", readBuffer.c_str());
        JsonIterator it;
        for (it = begin(value); it!= end(value); ++it) {
            DEBUGF(INDI::Logger::DBG_DEBUG, "iterating %s", it->key);
        }
    }

    return true;
}

bool IndiCloudRainMonitor::Disconnect()
{
    return true;
}

bool IndiCloudRainMonitor::initProperties()
{
    INDI::Weather::initProperties();
    
    IUFillText(&httpEndpointT[0], "API_ENDPOINT", "API Endpoint", "http://192.168.1.204/weather"); //this is the default but can be changed in the options OPTIONS_TAB.
    IUFillTextVector(&httpEndpointTP, httpEndpointT, 1, getDeviceName(), "HTTP_API_ENDPOINT", "HTTP endpoint", OPTIONS_TAB, IP_RW, 5, IPS_IDLE);
    
    addParameter("WEATHER_RAIN", "Rain", 0, 0, 0);
    setCriticalParameter("WEATHER_RAIN");
    addParameter("WEATHER_STATION_ONLINE", "Station Online", 0, 0, 0);
    setCriticalParameter("WEATHER_STATION_ONLINE");
    addParameter("WEATHER_CLOUD_COVER", "Cloud Cover", 0, 0, 0);
    setCriticalParameter("WEATHER_CLOUD_COVER");
    
    addDebugControl();
    return true;
}

void IndiCloudRainMonitor::ISGetProperties(const char *dev)
{
    INDI::Weather::ISGetProperties(dev);

    defineProperty(&httpEndpointTP);
    loadConfig(true, "HTTP_API_ENDPOINT");
}

bool IndiCloudRainMonitor::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if(!strcmp(dev,getDeviceName()))
    {
        if (!strcmp(httpEndpointTP.name, name))
        {
            IUUpdateText(&httpEndpointTP, texts, names, n);
            httpEndpointTP.s = IPS_OK;
            IDSetText(&httpEndpointTP, NULL);
            return true;
        }

    }

     return INDI::Weather::ISNewText(dev,name,texts,names,n);
}

/**
 * Poll the device's HTTP API to get the latest measurements. Set the state based on these measurements.
 */
IPState IndiCloudRainMonitor::updateWeather()
{
    DEBUG(INDI::Logger::DBG_DEBUG, "Updating from weather API_ENDPOINT");
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, httpEndpointT[0].text);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); //10 sec timeout
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* Check for errors */ 
            
        DEBUG(INDI::Logger::DBG_DEBUG, "curl complete");

        if(res != CURLE_OK) {
            DEBUG(INDI::Logger::DBG_ERROR, "curl failed!!!!!!");
//            DEBUGF(INDI::Logger::DBG_ERROR, "Connecttion to HTTP_API_ENDPOINT failed:%s",curl_easy_strerror(res));
            DEBUG(INDI::Logger::DBG_ERROR, "Cant contact weather device setting alert state");
            setParameterValue("WEATHER_STATION_ONLINE", 1);
            return IPS_OK;
        }
        DEBUG(INDI::Logger::DBG_DEBUG, "curl ok");

        /* always cleanup */ 
        curl_easy_cleanup(curl);

        char srcBuffer[readBuffer.size()];
        strncpy(srcBuffer, readBuffer.c_str(), readBuffer.size());
        char *source = srcBuffer;
        // do not forget terminate source string with 0
        char *endptr;
        JsonValue value;
        JsonAllocator allocator;
        int status = jsonParse(source, &endptr, &value, allocator);
        if (status != JSON_OK)
        {
            DEBUG(INDI::Logger::DBG_ERROR, "NON OK response, setting alert state");
            setParameterValue("WEATHER_STATION_ONLINE", 1);
            return IPS_OK;
        }
        DEBUGF(INDI::Logger::DBG_DEBUG, "http response %s", readBuffer.c_str());
        JsonIterator it;
        double skyTemp;
        double ambientTemp;
        for (it = begin(value); it!= end(value); ++it) {
            LOGF_DEBUG("iterating %s", it->key);
            if (!strcmp(it->key, "rain")) {
                //bool raining = it->value.toBool();
                //DEBUGF(INDI::Logger::DBG_DEBUG, "Setting rain value from response %s", it->value);
                if (it->value.getTag()==JSON_TRUE) {
                    setParameterValue("WEATHER_RAIN", 1);
                } else {
                    setParameterValue("WEATHER_RAIN", 0);
                }
            }
            if (!strcmp(it->key, "skyTemp")) {
                LOGF_DEBUG("Got skyTemp from response %g", it->value.toNumber());
                skyTemp = it->value.toNumber();
            }
            if (!strcmp(it->key, "outsideTemp")) {
                LOGF_DEBUG("Got outsideTemp from response %g", it->value.toNumber());
                ambientTemp = it->value.toNumber();
            }
        }
        //Function based on data collected, guesswork and linear regression using wolframalpha https://www.wolframalpha.com/input/?i=linear+fit+{0,+100},{-1,+90},{-2,+80},{-3,+50},{-4,+25},{-5,+10},{-6,0}&lk=3
        double result = (18 * skyTemp) + 100;
        if(result > 100) {
            result = 100;
        }
        if(result < 0) {
            result = 0;
        }
        LOGF_DEBUG("CLOUD %g", result);
        setParameterValue("WEATHER_CLOUD_COVER", result);      
    }
    setParameterValue("WEATHER_STATION_ONLINE", 0);
    return IPS_OK;
}

bool IndiCloudRainMonitor::saveConfigItems(FILE *fp)
{
    INDI::Weather::saveConfigItems(fp);

    IUSaveConfigText(fp, &httpEndpointTP);

    return true;
}
