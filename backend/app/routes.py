from flask import request, render_template

from app import app, db
from app.models import Data
from app.utils import is_positive


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
        device_id=data.get('device_id') if is_positive(
            data.get('device_id')) else None,
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
def graph():
    return render_template('graph.html')


@app.route('/graph/<stat>')
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
def daily_averages():
    temperature = round(
            db.session.query(db.func.avg(Data.temperature)).filter(
                Data.created_at >= db.func.current_date()).scalar(), 2)
    light = round(
            db.session.query(db.func.avg(Data.light)).filter(
                Data.created_at >= db.func.current_date()).scalar(), 2)
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
def last():
    previous = Data.query.order_by(Data.id.desc()).offset(1).first()
    last = Data.query.order_by(Data.id.desc()).first()
    return render_template('partials/last.html', last=last, previous=previous)
