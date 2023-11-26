"""Add fruit humidity configuration

Revision ID: 3f2ab1b70aa1
Revises: caef7ac6867c
Create Date: 2023-11-26 00:51:15.553331

"""
from app import db
from app.models import Configuration


# revision identifiers, used by Alembic.
revision = '3f2ab1b70aa1'
down_revision = 'caef7ac6867c'
branch_labels = None
depends_on = None


def upgrade():
    db.session.add(Configuration(key='humidity-nuts', value='all'))
    db.session.add(Configuration(key='humidity-potatoe', value='all'))
    db.session.add(Configuration(key='humidity-onion', value='all'))
    db.session.add(Configuration(key='humidity-tomatoe', value='all'))
    db.session.commit()


def downgrade():
    db.session.query(Configuration).filter_by(key='humidity-nuts').delete()
    db.session.query(Configuration).filter_by(key='humidity-potatoe').delete()
    db.session.query(Configuration).filter_by(key='humidity-onion').delete()
    db.session.query(Configuration).filter_by(key='humidity-tomatoe').delete()
    db.session.commit()
