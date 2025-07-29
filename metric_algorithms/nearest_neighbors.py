import numpy as np
from distances import euclidean_distance, cosine_distance
from sklearn.neighbors import NearestNeighbors


class KNNClassifier:
    def __init__(self, k, strategy, metric, weights, test_block_size):
        self.y = None
        self.k = k
        self.strategy = strategy
        if metric == 'euclidean':
            self.metric = euclidean_distance
        else:
            self.metric = cosine_distance
        self.weights = weights
        self.test_block_size = test_block_size
        if strategy != 'my_own':
            self.model = NearestNeighbors(n_neighbors=k,
                                          algorithm=strategy,
                                          metric=metric)
        else:
            self.X = None

    def fit(self, X, y):
        if self.strategy == 'my_own':
            self.X = X
        else:
            self.model.fit(X)
        self.y = y

    def find_kneighbors(self, X, return_distance):
        if self.strategy == 'my_own':
            temp = self.metric(self.X, X).T
            indices = np.argsort(temp, axis=1)[:, :self.k]
            if return_distance:
                distances = temp[np.arange(temp.shape[0])[:, None], indices]
                return distances, indices
            return indices
        return self.model.kneighbors(X, n_neighbors=self.k,
                                     return_distance=return_distance)

    def predict(self, X):
        def func(x):
            return np.argmax(np.bincount(x))
        ans = np.zeros(X.shape[0], dtype=self.y.dtype)
        for start in range(0, X.shape[0], self.test_block_size):
            end = min(start + self.test_block_size, X.shape[0])
            X_temp = X[start:end]
            if not self.weights:
                indices = self.find_kneighbors(X_temp,
                                               return_distance=False)
                temp = self.y[indices]
                ans[start:end] = np.apply_along_axis(func, 1, temp)
            else:
                distances, indices = self.find_kneighbors(X_temp,
                                                          return_distance=True)
                weights = 1 / (distances + 10 ** (-5))
                temp = self.y[indices]
                for i in range(temp.shape[0]):
                    label, label_idx = np.unique(temp[i], return_inverse=True)
                    label_idx = np.bincount(label_idx, weights=weights[i])
                    ans[start + i] = label[np.argmax(label_idx)]
        return ans
