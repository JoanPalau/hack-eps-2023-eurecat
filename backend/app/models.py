from app import db


class Data(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    device_id = db.Column(db.String, nullable=False)
    created_at = db.Column(db.DateTime, default=db.func.now())
    temperature = db.Column(db.Float, nullable=False)
    light = db.Column(db.Float, nullable=False)
    soil_humidity = db.Column(db.Float, nullable=False)
    air_humidity = db.Column(db.Float, nullable=False)
    extra_data = db.Column(db.JSON)


class Configuration(db.Model):
    key = db.Column(db.String, primary_key=True)
    value = db.Column(db.String, nullable=False)
