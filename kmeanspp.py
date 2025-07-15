import sys
import csv
import numpy as np
import mykmeanspp as km

# Consts
DEFAULT_ITER = 300


def read_input(args: list) -> (dict[int, list[float]], int, int, float):
    n = len(args)
    if not (n >= 4 and n <= 5):
        print("An Error Has Occurred")
        exit(1)
    offset = 0 if n == 5 else -1

    # Read K
    try:
        kf = float(args[0])
        if kf != int(kf):
            raise ValueError
    except ValueError:
        print("Invalid number of clusters!")
        exit(1)
    k = int(kf)
    
    # Read iter if present
    max_iter = DEFAULT_ITER
    if n == 5:
        try:
            mif = float(args[1])
            if mif != int(mif) or mif <= 1 or mif >= 1000:
                raise ValueError
        except ValueError:
            print("Invalid maximum iteration!")
            exit(1)
        max_iter = int(mif)

    # Read eps
    try:
        eps = float(args[2 + offset])
        if eps < 0:
            raise ValueError
    except ValueError:
        print("Invalid epsilon!")
        exit(1)

    # Read points
    points_dict = {}
    for idx in [3, 4]:
        # Read file1 into a dict: key -> list of floats (excluding key)
        with open(args[idx + offset], "r") as f1:
            reader = csv.reader(f1)
            for row in reader:
                key = int(float(row[0]))
                values = [float(x) for x in row[1:]]
                if key not in points_dict:
                    points_dict[key] = values
                else:
                    points_dict[key] += values

    # Make sure K < N
    if not (k > 1 and k < len(points_dict.keys())):
        print("Invalid number of clusters!")
        exit(1)

    return points_dict, k, max_iter, eps


def kmeanspp(
    points_dict: dict[int, list[tuple]], k: int
) -> (list[tuple], list[tuple], list[int]):
    points = dict_to_list(points_dict)
    n = len(points)

    # kmeans++
    np.random.seed(1234)
    init_centroids = [points[np.random.choice(n)]]
    for j in range(1, k):
        distances = [nearest_distance(point, init_centroids) for point in points]
        sum = np.sum(distances)
        probs = np.array(distances) / sum
        init_centroids.append(points[np.random.choice(n, p=probs)])

    return points, init_centroids, get_keys(points_dict, init_centroids)


def nearest_distance(point: tuple, centroids: list[tuple]) -> float:
    min_dist = float("inf")
    for centroid in centroids:
        dist = euclidean_distance(point, centroid)
        min_dist = np.min((min_dist, dist))
    return min_dist


def euclidean_distance(point1: tuple, point2: tuple) -> float:
    # Check dimentional consistancy
    if not len(point1) == len(point2):
        print("An Error Has Occurred")
        exit(1)
    distance = 0
    for i in range(len(point1)):
        distance += (point1[i] - point2[i]) ** 2
    return np.sqrt(distance)


def dict_to_list(points_dict: dict[int, list[tuple]]) -> list[tuple]:
    return [tuple(point[1]) for point in sorted(points_dict.items())]


def get_keys(points_dict: dict[int, list[tuple]], points: list[tuple]) -> list[int]:
    point_to_key = {tuple(v): k for k, v in points_dict.items()}
    return [point_to_key[point] for point in points]


def kmeans(
    points: list[tuple], init_centroids: list[tuple], k: int, max_iter: int, eps: float
) -> list[tuple]:
    n = len(points)
    dim = len(points[0])
    centroids = km.fit(points, init_centroids, k, max_iter, dim, n, eps)
    if centroids == None:
        print("An Error Has Occurred")
        exit(1)
    return centroids

def average_point(lst: list[tuple], sub_lst: list[tuple]) -> tuple[float]:
    dim = len(lst[0])
    n = len(sub_lst)

    # Initialize a list of zeros for each dimension
    totals = [0] * dim

    # Sum each dimension
    for j in range(0, len(sub_lst)):
        for i in range(dim):
            totals[i] += lst[sub_lst[j]][i]

    # Compute the average for each dimension
    return tuple(tot / n for tot in totals)


def print_result(indices: list[int], centroids: list[tuple]):
    print(",".join(f"{idx}" for idx in indices))
    for centroid in centroids:
        print(",".join(f"{num:.4f}" for num in centroid))


if __name__ == "__main__":
    points_dict, k, max_iter, eps = read_input(sys.argv[1:])
    points, init_centroids, indices = kmeanspp(points_dict, k)
    centroids = kmeans(points, init_centroids, k, max_iter, eps)
    print_result(indices, centroids)
