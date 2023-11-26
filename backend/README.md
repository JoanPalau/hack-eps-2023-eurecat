# Flask-App with SQLAlchemy and Flask-Migrate

This is a simple Flask web application template that utilizes Flask,
Flask-SQLAlchemy, and Flask-Migrate. The application includes a basic setup 
for a database with SQLAlchemy and migration support with Flask-Migrate. 
The SQLAlchemy URI is configured in the `config.py` file.

## Installation

1. Clone the repository:

```bash
git clone https://github.com/JoanPalau/hack-eps-2023-eurecat
cd your-repo
```

2. Create a virtual environment and activate it:

```bash
python -m venv venv
source venv/bin/activate  # On Windows, use `venv\Scripts\activate`
```

3. Install dependencies:

```bash
pip install -r requirements.txt
```

4. Create a `config.py` file in the root directory and define your SQLAlchemy URI:

```python
# config.py
import os

class Config:
    SQLALCHEMY_DATABASE_URI = "your_database_uri_here"
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    # Add other configurations as needed
```

## Database Initialization

To initialize the database, run the following commands in your terminal:

```bash
flask db init
flask db migrate -m "Initial migration"
flask db upgrade
```

- `flask db init`: Initializes the migration directory.
- `flask db migrate -m "Initial migration"`: Creates an initial migration.
- `flask db upgrade`: Applies the migration to the database.

## Running the Application

To run the application, use the following commands:

```bash
export FLASK_APP=app
export FLASK_ENV=development  # Set to 'production' for production environment
flask run
```

Visit [http://localhost:5000](http://localhost:5000) in your web browser to access the application.

## Adding users

To add users you must use the following commands:
```bash
flask users create email
flask users activate email
```
