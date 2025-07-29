import numpy as np
from nearest_neighbors import KNNClassifier

np.int = int


def kfold(n, n_folds):
    length = n // n_folds
    delta = n % n_folds
    indices = []
    for i in range(1, n_folds, 1):
        if i <= delta:
            indices.append(i * length + i)
        else:
            indices.append(i * length + delta)
    val = np.arange(0, indices[0])
    train = np.arange(indices[0], n)
    ans = [(train, val)]
    for i in range(len(indices) - 1):
        val = np.arange(indices[i], indices[i+1])
        train = np.array([j for j in range(n)
                          if ((j < indices[i]) or (j >= indices[i+1]))])
        ans.append((train, val))
    val = np.arange(indices[len(indices) - 1], n)
    train = np.arange(0, indices[len(indices) - 1])
    ans.append((train, val))
    return ans


def knn_cross_val_score(X, y, k_list, score, cv, **kwargs):
    def accuracy(y1, y2):
        return np.sum(y1 == y2) / y1.size
    ans = {}
    if cv is None:
        cv = kfold(y.size, 3)
    for k in k_list:
        ans[k] = []
        model = KNNClassifier(k, **kwargs)
        for (train_index, val_index) in cv:
            X_train = X[train_index]
            y_train = y[train_index]
            X_val = X[val_index]
            y_val = y[val_index]
            model.fit(X_train, y_train)
            y_pred = model.predict(X_val)
            if score == 'accuracy':
                ans[k].append(accuracy(y_val, y_pred))
        ans[k] = np.array(ans[k])
    return ans
