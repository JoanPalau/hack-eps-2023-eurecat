import datetime
from flask import request, render_template
from flask_security import login_required

from app import app, db
from app.models import Data, Configuration
from app.utils import is_positive
from app.mqtt import publish_data


@app.route('/')
@app.route('/index')
@login_required
def index():
    return render_template('index.html')


@app.route('/tables')
@login_required
def tables():
    data = Data.query.all()
    return render_template('tables.html', data=data)


@app.route('/save_data', methods=['POST'])
@login_required
def save_data():
    data = request.json
    db.session.add(Data(
        device_id=data.get('id'),
        temperature=data.get('temperature') if is_positive(
            data.get('temperature')) else None,
        light=data.get('light') if is_positive(
            data.get('light')) else None,
        soil_humidity=data.get('soil_humidity') if is_positive(
            data.get('soil_humidity')) else None,
        air_humidity=data.get('air_humidity') if is_positive(
            data.get('air_humidity')) else None,
        ))
    try:
        db.session.commit()
    except Exception as e:
        return str(e)
    return 'Data saved'


@app.route('/graph')
@login_required
def graph():
    return render_template('graph.html')


@app.route('/graph/<stat>')
@login_required
def graph_stat(stat):
    devices = Data.query.with_entities(Data.device_id).distinct()
    labels = [d[0] for d in devices]
    payload = {"labels": labels, "datasets": []}
    for device in devices:
        data = Data.query.filter_by(device_id=device[0]).all()
        if data:
            values = [getattr(d, stat) for d in data]
            payload["datasets"].append({
                "name": device[0],
                "label": device[0],
                "data": values,
                })
    return payload


# Partial routes for htmx
@app.route('/partial/averages', methods=['GET'])
@login_required
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


@app.route('/partial/daily_averages', methods=['GET'])
@login_required
def daily_averages():
    temperature = round(
            db.session.query(db.func.avg(Data.temperature)).scalar(), 2)
    light = round(
            db.session.query(db.func.avg(Data.light)).scalar(), 2)
    soil_humidity = round(
        db.session.query(db.func.avg(Data.soil_humidity)).filter(
            Data.created_at >= db.func.current_date()).scalar(), 2)
    air_humidity = round(
        db.session.query(db.func.avg(Data.air_humidity)).filter(
            Data.created_at >= db.func.current_date()).scalar(), 2)
    averages = {
            "temperature": temperature,
            "light": light,
            "soil_humidity": soil_humidity,
            "air_humidity": air_humidity,
            }
    return render_template('partials/averages.html', averages=averages)


@app.route('/partial/last', methods=['GET'])
@login_required
def last():
    previous = Data.query.order_by(Data.id.desc()).offset(1).first()
    last = Data.query.order_by(Data.id.desc()).first()
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
    return render_template('partials/last.html', last=last,
                           previous=previous, averages=averages)


@app.route('/partial/last_records', methods=['GET'])
@login_required
def last_records():
    last_records = Data.query.order_by(Data.id.desc()).limit(10).all()
    return render_template('partials/last_records.html', last=last_records)


@app.route('/partial/devices_select', methods=['GET'])
@login_required
def devices_select():
    devices = Data.query.with_entities(Data.device_id).distinct()
    return render_template('partials/devices_select.html', devices=devices)


@app.route('/partial/pump', methods=['POST'])
@app.route('/alexa/pump', methods=['GET'])
def pump():
    # Day it's true if its before 7pm and after 7am
    publish_data({
        "pump": True,
        "day": 7 <= datetime.datetime.now().hour <= 19,
        })
    return render_template('partials/notification.html',
                           message='Pump correctly activated')


@app.route('/partial/humidity-toggle/<fruit>', methods=['GET'])
@login_required
def humidity_toggle(fruit):
    configuration = Configuration.query.filter_by(
            key="humidity-" + fruit).first()
    return render_template('partials/humidity-switch.html',
                           humidity=configuration, fruit=fruit)


@app.route('/partial/humidity/<fruit>/<setting>', methods=['POST'])
@app.route('/alexa/humidity/<fruit>/<setting>', methods=['GET'])
def humidity_switch(fruit, setting):
    configuration = Configuration.query.filter_by(
            key="humidity-" + fruit).first()
    if setting not in ['all', 'air', 'soil']:
        setting = 'all'
    configuration.value = setting
    db.session.commit()
    publish_data({"dhtmode": setting, "fruit": fruit})
    return render_template('partials/humidity-switch.html',
                           humidity=configuration, fruit=fruit)
