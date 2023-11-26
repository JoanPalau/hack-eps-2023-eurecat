from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from flask_login import LoginManager
from flask_security import Security

from config import Config

app = Flask(__name__)
app.config.from_object(Config)
app.secret_key = b'aofhwf22jioafjwo2'
app.config['SECURITY_PASSWORD_HASH'] = 'sha512_crypt'
app.config['SECURITY_PASSWORD_SALT'] = '12312ioj12jio'
app.config['SECURITY_RECOVERABLE'] = True

db = SQLAlchemy(app)
migrate = Migrate(app, db)
# Flask security
login = LoginManager(app)
login.login_view = 'login'

from app import routes  # noqa
from app import models  # noqa
assert routes, models

security = Security(app, models.user_datastore)
