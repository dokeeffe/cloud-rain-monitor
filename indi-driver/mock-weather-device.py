# HTTP server to be used for testing
# Run the indi driver and configure to connect to a weather station at http://localhost:8080/weather/true to simulate rain conditions
#
from bottle import route, run, template

@route('/weather/<rain>')
def index(rain):
    return template('{"uptime":"05:14:12", "skyTemp":    4.230, "outsideTemp":    1.090, "rain":{{rain}} }', rain=rain)

run(host='localhost', port=8080)
