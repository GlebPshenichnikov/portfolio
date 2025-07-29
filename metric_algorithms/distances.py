import numpy as np


def euclidean_distance(x, y):
    sqr_x = np.sum(x**2, axis=1)
    sqr_y = np.sum(y**2, axis=1)
    temp_xy = np.dot(x, y.T)
    return np.sqrt(sqr_x[:, np.newaxis] + sqr_y - 2 * temp_xy)


def cosine_distance(x, y):
    return (1 - np.dot(x, y.T) /
            (np.sqrt(np.sum(x[:, np.newaxis, :] ** 2, axis=-1)) *
             np.sqrt(np.sum(y[np.newaxis, :, :] ** 2, axis=-1))))
