"""Add humidity configuration

Revision ID: caef7ac6867c
Revises: 9aa73ef1f9ba
Create Date: 2023-11-26 00:13:04.350487

"""
from app import db
from app.models import Configuration

# revision identifiers, used by Alembic.
revision = 'caef7ac6867c'
down_revision = '9aa73ef1f9ba'
branch_labels = None
depends_on = None


def upgrade():
    db.session.add(Configuration(key='humidity', value='all'))
    db.session.commit()


def downgrade():
    db.session.query(Configuration).filter_by(key='humidity').delete()
    db.session.commit()
