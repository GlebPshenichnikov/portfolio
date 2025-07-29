import oracles
import time
from scipy.special import expit
import numpy as np


class GDClassifier:
    """
    Реализация метода градиентного спуска для произвольного
    оракула, соответствующего спецификации оракулов из модуля oracles.py
    """

    def __init__(
            self, loss_function, step_alpha=1, step_beta=0,
            tolerance=1e-5, max_iter=1000, **kwargs
    ):
        """
        loss_function - строка, отвечающая за функцию потерь классификатора.
        Может принимать значения:
        - 'binary_logistic' - бинарная логистическая регрессия

        step_alpha - float, параметр выбора шага из текста задания

        step_beta- float, параметр выбора шага из текста задания

        tolerance - точность, по достижении которой, необходимо прекратить оптимизацию.
        Необходимо использовать критерий выхода по модулю разности соседних значений функции:
        если |f(x_{k+1}) - f(x_{k})| < tolerance: то выход

        max_iter - максимальное число итераций

        **kwargs - аргументы, необходимые для инициализации
        """

        self.step_alpha = step_alpha
        self.step_beta = step_beta
        self.tolerance = tolerance
        self.max_iter = max_iter
        self.w = None
        if loss_function == 'binary_logistic':
            self.model = oracles.BinaryLogistic(**kwargs)

    def fit(self, X, y, w_0=None, trace=False, X_test=None, y_test=None):
        """
        Обучение метода по выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        y - одномерный numpy array

        w_0 - начальное приближение в методе

        trace - переменная типа bool

        Если trace = True, то метод должен вернуть словарь history, содержащий информацию
        о поведении метода. Длина словаря history = количество итераций + 1 (начальное приближение)

        history['time']: list of floats, содержит интервалы времени между двумя итерациями метода
        history['func']: list of floats, содержит значения функции на каждой итерации
        (0 для самой первой точки)
        """

        if w_0 is None:
            self.w = np.zeros(X.shape[1])
        else:
            self.w = w_0.copy()

        new_f = self.model.func(X, y, self.w)

        if trace:
            history = {}
            history['time'] = [0.0]
            history['func'] = [new_f]
            # if X_test is not None:
            #     history['accuracy'] = [accuracy(self.predict(X_test), y_test)]

        for i in range(1, self.max_iter + 1):
            last_f = new_f
            start_time = time.time()
            gw = self.model.grad(X, y, self.w)
            eta = self.step_alpha / (i ** self.step_beta)
            self.w -= eta * gw
            new_f = self.model.func(X, y, self.w)
            end_time = time.time()
            if trace:
                history['time'].append(end_time - start_time)
                history['func'].append(new_f)
                # if X_test is not None:
                #     history['accuracy'].append(accuracy(self.predict(X_test), y_test))
            if abs(new_f - last_f) < self.tolerance:
                if trace:
                    return history
                return
        if trace:
            return history
        return

    def predict(self, X):
        """
        Получение меток ответов на выборке X

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: одномерный numpy array с предсказаниями
        """
        temp = np.sign(np.dot(X, self.w))
        temp = np.where(temp == 0, -1, temp)
        return temp.astype(int)

    def predict_proba(self, X):
        """
        Получение вероятностей принадлежности X к классу k

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: двумерной numpy array, [i, k] значение соответветствует вероятности
        принадлежности i-го объекта к классу k
        """
        temp_1 = expit(np.dot(X, self.w))
        temp_0 = 1 - temp_1
        return np.vstack((temp_0, temp_1)).T

    def get_objective(self, X, y):
        """
        Получение значения целевой функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: float
        """
        return self.model.func(X, y, self.w)

    def get_gradient(self, X, y):
        """
        Получение значения градиента функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: numpy array, размерность зависит от задачи
        """
        return self.model.grad(X, y, self.w)

    def get_weights(self):
        """
        Получение значения весов функционала
        """
        return self.w


class SGDClassifier(GDClassifier):
    """
    Реализация метода стохастического градиентного спуска для произвольного
    оракула, соответствующего спецификации оракулов из модуля oracles.py
    """

    def __init__(
            self, loss_function, batch_size, step_alpha=1, step_beta=0,
            tolerance=1e-5, max_iter=1000, random_seed=153, **kwargs
    ):
        """
        loss_function - строка, отвечающая за функцию потерь классификатора.
        Может принимать значения:
        - 'binary_logistic' - бинарная логистическая регрессия

        batch_size - размер подвыборки, по которой считается градиент

        step_alpha - float, параметр выбора шага из текста задания

        step_beta- float, параметр выбора шага из текста задания

        tolerance - точность, по достижении которой, необходимо прекратить оптимизацию
        Необходимо использовать критерий выхода по модулю разности соседних значений функции:
        если |f(x_{k+1}) - f(x_{k})| < tolerance: то выход

        max_iter - максимальное число итераций (эпох)

        random_seed - в начале метода fit необходимо вызвать np.random.seed(random_seed).
        Этот параметр нужен для воспроизводимости результатов на разных машинах.

        **kwargs - аргументы, необходимые для инициализации
        """

        self.step_alpha = step_alpha
        self.batch_size = batch_size
        self.random_seed = random_seed
        self.step_beta = step_beta
        self.tolerance = tolerance
        self.max_iter = max_iter
        self.w = None
        if loss_function == 'binary_logistic':
            self.model = oracles.BinaryLogistic(**kwargs)

    def fit(self, X, y, w_0=None, trace=False, log_freq=1):
        """
        Обучение метода по выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        y - одномерный numpy array

        w_0 - начальное приближение в методе

        Если trace = True, то метод должен вернуть словарь history, содержащий информацию
        о поведении метода. Если обновлять history после каждой итерации, метод перестанет
        превосходить в скорости метод GD. Поэтому, необходимо обновлять историю метода лишь
        после некоторого числа обработанных объектов в зависимости от приближённого номера эпохи.
        Приближённый номер эпохи:
            {количество объектов, обработанных методом SGD} / {количество объектов в выборке}

        log_freq - float от 0 до 1, параметр, отвечающий за частоту обновления.
        Обновление должно проиходить каждый раз, когда разница между двумя значениями приближённого номера эпохи
        будет превосходить log_freq.

        history['epoch_num']: list of floats, в каждом элементе списка будет записан приближённый номер эпохи:
        history['time']: list of floats, содержит интервалы времени между двумя соседними замерами
        history['func']: list of floats, содержит значения функции после текущего приближённого номера эпохи
        history['weights_diff']: list of floats, содержит квадрат нормы разности векторов весов с соседних замеров
        (0 для самой первой точки)
        """
        self.w = w_0
        new_f = self.model.func(X, y, self.w)
        if trace:
            history = {}
            history['epoch_num'] = []
            history['time'] = []
            history['func'] = []
            history['weights_diff'] = []

        np.random.seed(self.random_seed)
        is_his = False
        i = 1
        cur_obj = 0
        is_max = True
        while i < self.max_iter + 1 and is_max:
            index = np.arange(X.shape[0])
            np.random.shuffle(index)
            X_temp = X[index]
            y_temp = y[index]
            for j in range(0, X.shape[0], self.batch_size):
                last_f = new_f
                last_w = self.w.copy()
                start_time = time.time()
                batch_x = X_temp[j:min(X.shape[0], j + self.batch_size)]
                batch_y = y_temp[j:min(X.shape[0], j + self.batch_size)]
                gw = self.model.grad(batch_x, batch_y, self.w)
                eta = self.step_alpha / (i ** self.step_beta)
                self.w -= eta * gw
                i += 1
                new_f = self.model.func(batch_x, batch_y, self.w)
                end_time = time.time()

                if j + self.batch_size > X.shape[0]:
                    cur_obj += X.shape[0] - j
                else:
                    cur_obj += self.batch_size
                is_his = is_his or (cur_obj / X.shape[0] >= log_freq)
                if is_his:
                    history['epoch_num'].append(cur_obj / X.shape[0])
                    history['time'].append(end_time - start_time)
                    history['func'].append(new_f)
                    if len(history['weights_diff']) == 0:
                        history['weights_diff'].append(0.0)
                    else:
                        history['weights_diff'].append(np.linalg.norm(self.w - last_w))

                if i > self.max_iter + 1:
                    break
                if abs(last_f - new_f) < self.tolerance:
                    is_max = False
                    break
        return history
