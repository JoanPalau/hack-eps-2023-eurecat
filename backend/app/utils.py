def is_positive(x):
    if x is None:
        return False
    try:
        x = int(x)
    except ValueError:
        return False
    return x > 0
