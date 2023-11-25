from flask import request

from app import app, db
from app.models import Data


@app.route('/')
@app.route('/index')
def index():
    return 'Hello, World!'


@app.route('/save_data', methods=['POST'])
def save_data():
    data = request.json
    db.session.add(Data(data=data))
    db.session.commit()
    return 'Data saved'
