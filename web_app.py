from flask import Flask
from flask import request
from query.interface import GSQuery

import json

app = Flask(__name__)

@app.route('/api/gsq', methods=['GET'])
def index():

  lat      = float(request.args.get('lat'))
  lon      = float(request.args.get('lon'))

  if request.args.get('keywords'):
    keywords = request.args.get('keywords')
  else:
    keywords = ""

  if request.args.get('K'):
    K = int(request.args.get('K'))
  else:
    K = 3

  if request.args.get('W'):
    W = float(request.args.get('W'))
  else:
    W = 0.5

  if request.args.get('BPower'):
    Bpower = float(request.args.get('BPower'))
  else:
    Bpower = 0

  if request.args.get('addit'):
    addit = float(request.args.get('addit'))
  else:
    addit = 0

  if request.args.get('radius'):
    R  = float(request.args.get('radius'))
  else:
    R  = 1

  if request.args.get('M'):
    M  = float(request.args.get('M'))
  else:
    M  = 5

  if request.args.get('qType'):
    qType  = request.args.get('qType')
  else:
    qType = 'NSG_LAZY'

  d = {
    'geo': (lat,lon),
    'keywords': keywords.split(",")
  }
  # data_set = str(request.args.get('data_set'))
  query = GSQuery(qType).query(d, K, W, Bpower, addit, R, M)
  return json.dumps({ "state": "success", "response": query })

if __name__ == '__main__':
    app.run(debug=True)
