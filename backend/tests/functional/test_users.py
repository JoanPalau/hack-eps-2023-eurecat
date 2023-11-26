"""
This file (test_users.py) contains the functional tests for the `users` blueprint.

These tests use GETs and POSTs to different URLs to check for the proper behavior
of the `users` blueprint.
"""


def test_login_page(test_client):
    """
    GIVEN a Flask application configured for testing
    WHEN the '/login' page is requested (GET)
    THEN check the response is valid
    """
    response = test_client.get('/login')
    assert response.status_code == 200
    assert b'Login' in response.data
    assert b'Email' in response.data
    assert b'Password' in response.data


def test_valid_login_logout(test_client, init_database):
    """
    GIVEN a Flask application configured for testing
    WHEN the '/login' page is posted to (POST)
    THEN check the response is valid
    """
    response = test_client.post('/login',
                                data=dict(email='admin@admin.com',
                                          password='FlaskIsAwesome'),
                                follow_redirects=True)
    assert response.status_code == 200
    assert b'Logout' in response.data
    assert b'Login' not in response.data

    """
    GIVEN a Flask application configured for testing
    WHEN the '/logout' page is requested (GET)
    THEN check the response is valid
    """
    response = test_client.get('/logout', follow_redirects=True)
    assert response.status_code == 200
    assert b'Logout' not in response.data
    assert b'Login' in response.data


def test_invalid_login(test_client, init_database):
    """
    GIVEN a Flask application configured for testing
    WHEN the '/login' page is posted to with invalid credentials (POST)
    THEN check an error message is returned to the user
    """
    response = test_client.post('/login',
                                data=dict(email='notadmin@admin.com',
                                          password='FlaskIsNotAwesome'),
                                follow_redirects=True)
    assert response.status_code == 200
    assert b'Logout' not in response.data
    assert b'Login' in response.data
    assert b'Register' in response.data


def test_login_already_logged_in(test_client, init_database, log_in_default_user):
    """
    GIVEN a Flask application configured for testing
    WHEN the '/login' page is posted to (POST) when the user is already logged in
    THEN check an error message is returned to the user
    """
    response = test_client.post('/login',
                                data=dict(email='admin@admin.com',
                                          password='FlaskIsNotAwesome'),
                                follow_redirects=True)
    assert response.status_code == 200
    assert b'Logout' in response.data
    assert b'Login' not in response.data
    assert b'Register' not in response.data
