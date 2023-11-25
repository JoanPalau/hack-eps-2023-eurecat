from flask import request, render_template

from app import app, db
from app.models import Data


@app.route('/')
@app.route('/index')
def index():
    return render_template('index.html')


@app.route('/tables')
def tables():
    data = Data.query.all()
    return render_template('tables.html', data=data)


@app.route('/save_data', methods=['POST'])
def save_data():
    data = request.json
    db.session.add(Data(
        device_id=data['device_id'],
        temperature=data['temperature'],
        light=data['light'],
        soil_humidity=data['soil_humidity'],
        air_humidity=data['air_humidity'],
        ))
    db.session.commit()
    return 'Data saved'


# Partial routes for htmx
@app.route('/partial/averages', methods=['GET'])
def averages():
    temperature = round(
            db.session.query(db.func.avg(Data.temperature)).scalar(), 2)
    light = round(
            db.session.query(db.func.avg(Data.light)).scalar(), 2)
    soil_humidity = round(
        db.session.query(db.func.avg(Data.soil_humidity)).scalar(), 2)
    air_humidity = round(
        db.session.query(db.func.avg(Data.air_humidity)).scalar(), 2)
    averages = {
            "temperature": temperature,
            "light": light,
            "soil_humidity": soil_humidity,
            "air_humidity": air_humidity,
            }
    return render_template('partials/averages.html', averages=averages)


@app.route('/partial/last', methods=['GET'])
def last():
    previous = Data.query.order_by(Data.id.desc()).offset(1).first()
    last = Data.query.order_by(Data.id.desc()).first()
    return render_template('partials/last.html', last=last, previous=previous)
