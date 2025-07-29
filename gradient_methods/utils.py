import numpy as np


def grad_finite_diff(function, w, eps=1e-8):
    """
    Возвращает численное значение градиента, подсчитанное по следующией формуле:
        result_i := (f(w + eps * e_i) - f(w)) / eps,
        где e_i - следующий вектор:
        e_i = (0, 0, ..., 0, 1, 0, ..., 0)
                          >> i <<
    """
    def temp_fun(x):
        return (function(x) - function(w)) / eps
    vect_temp_fun = np.vectorize(temp_fun, signature='(n)->()')
    new_w = w + np.eye(len(w)) * eps

    return vect_temp_fun(new_w)


print(grad_finite_diff (lambda x: np.sum(np.log(x) ** 2), w = np.array([1, 2, 3, 4, 5,6]), eps = 1))