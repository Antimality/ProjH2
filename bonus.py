import numpy as np
from sklearn.datasets import load_iris
from sklearn.cluster import KMeans
from numpy.linalg import norm

import matplotlib.pyplot as plt

# Load Iris dataset
iris = load_iris()
X = iris.data

# Store inertia for each k
inertias = []

# Try k from 1 to 10
ks = range(1, 11)
for k in ks:
    kmeans = KMeans(n_clusters=k, init="k-means++", random_state=0).fit(X)
    inertias.append(kmeans.inertia_)


### Detect the "elbow" point using the "maximum distance to line" method
# Points: (k, inertia, 0)    (third dimention required by np when using cross product)
points = np.array(list(zip(ks, inertias)))
points = np.hstack([points, np.zeros((points.shape[0], 1))])
start, end = points[0], points[-1]

# Compute distances from each point to the line between start and end
line_vec = end - start
line_vec_norm = line_vec / norm(line_vec)
distances = [np.abs(norm(np.cross(point - start, line_vec_norm))) for point in points]
elbow_idx = np.argmax(distances)
elbow_k = ks[elbow_idx]
elbow_inertia = inertias[elbow_idx]

# Plot
plt.figure(figsize=(8, 5))
plt.plot(ks, inertias, marker="o", label="Iris Dataset")
plt.scatter(
    [elbow_k],
    [elbow_inertia],
    s=200,
    facecolors="none",
    edgecolors="red",
    linewidths=2,
    label="Elbow Point",
)
plt.legend()
plt.title("Elbow Point Identification via Distance From Line Method")
plt.xlabel("Number of clusters (k)")
plt.ylabel("Inertia")
plt.xticks(ks)
plt.grid(True)
# plt.show()
plt.savefig("elbow.png")
