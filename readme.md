###Setup

Setting up the query executables
```
cd lib
make
./NNTopkUsers data_GSR_Austin/queries.txt data_GSR_Austin/checkins GF1 32 100 data_GSR_Austin/socialGraph sum 1 4 1 -97.8997278214 -97.5564050674 30.1313942671 30.4098181886 390 0.485 2640
```

Setting up the web application
```
pip install flask
python web_app.py
# Running on http://127.0.0.1:5000/ (Press CTRL+C to quit)
```

Setting up the web views
```
cd web-app
npm install
bower install
grunt serve
```
