from flask import Flask
from flask import request
from query.gsr import GSR

import json

app = Flask(__name__)

@app.route('/api/gsr', methods=['GET'])
def index():
  r        = float(request.args.get('radius'))
  lat      = float(request.args.get('lat'))
  lon      = float(request.args.get('lon'))
  K        = int(request.args.get('K'))
  data_set = str(request.args.get('data_set'))
  query = GSR(data_set).query((lat,lon), r, K)
  return json.dumps(query)

if __name__ == '__main__':
    app.run(debug=True)
