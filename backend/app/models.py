import datetime
from sqlalchemy.ext.hybrid import hybrid_property
from sqlalchemy import event
from flask_security import SQLAlchemyUserDatastore, \
        UserMixin, RoleMixin

from app import db, login


@login.user_loader
def load_user(id):
    return User.query.get(int(id))


class TimestampsMixin(object):
    created_at = db.Column(
            db.DateTime,
            default=datetime.datetime.now())
    updated_at = db.Column(db.DateTime,
                           default=datetime.datetime.now())

    @staticmethod
    def create_time(mapper, connection, target):
        target.created_at = datetime.datetime.now()

    @staticmethod
    def update_time(mapper, connection, target):
        target.updated_at = datetime.datetime.now()

    @classmethod
    def register(cls):
        event.listen(cls, 'before_insert', cls.create_time)
        event.listen(cls, 'before_update', cls.update_time)


roles_users = db.Table(
        'roles_users',
        db.Column('user_id', db.Integer(), db.ForeignKey('user.id')),
        db.Column('role_id', db.Integer(), db.ForeignKey('role.id')))


class Role(db.Model, RoleMixin):
    id = db.Column(db.Integer(), primary_key=True)
    name = db.Column(db.String, unique=True)
    description = db.Column(db.String)


class User(db.Model, UserMixin, TimestampsMixin):
    id = db.Column(db.Integer, primary_key=True)
    email = db.Column(db.String, unique=True)
    password = db.Column(db.String)
    active = db.Column(db.Boolean())
    confirmed_at = db.Column(db.DateTime())
    fs_uniquifier = db.Column(db.String(255), unique=True, nullable=False)
    roles = db.relationship('Role', secondary=roles_users,
                            backref=db.backref('users', lazy='dynamic'))

    first_name = db.Column(db.String)
    last_name = db.Column(db.String)
    phone = db.Column(db.String)
    street = db.Column(db.String)
    city = db.Column(db.String)
    zip_code = db.Column(db.Integer)
    country = db.Column(db.String)

    def __repr__(self):
        return '<User {}>'.format(self.id)

    @hybrid_property
    def full_name(self):
        return '%s %s' % (
                self.first_name if self.first_name else '',
                self.last_name if self.last_name else '',
                )


user_datastore = SQLAlchemyUserDatastore(db, User, Role)
User.register()


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
