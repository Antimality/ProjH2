import sys
from math import sqrt
import csv

# Consts
DEFAULT_ITER = 300


def read_input(args: list) -> (dict[int, list[float]], int, int, float):
    n = len(args)
    if not (n >= 4 and n <= 5):
        print("An Error Has Occurred")
        exit(1)
    offset = 0 if n == 5 else -1

    # Read K
    # TODO: Does something like k=42.0 fine?
    if not str.isdecimal(args[0]):
        print("Incorrect number of clusters!")
        exit(1)
    k = int(args[0])

    # Read iter if present
    max_iter = DEFAULT_ITER
    if n == 5:
        if not str.isdecimal(args[1]):
            print("Incorrect maximum iteration!")
            exit(1)
        max_iter = int(args[1])
        if not (max_iter > 1 and max_iter < 1000):
            print("Incorrect maximum iteration!")
            exit(1)

    # Read eps
    # TODO: Handle float
    # try:
    #     eps = float(args[2 + offset])
    # except:
    #     print("Invalid epsilon!")
    #     exit(1)
    # if not str.isnumeric(args[2 + offset]):
    #     print("Invalid epsilon!")
    #     exit(1)
    eps = float(args[2 + offset])
    if eps < 0:
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
        print("Incorrect number of clusters!")
        exit(1)

    return points_dict, k, max_iter, eps


def kmeanspp(
    points_dict: dict[int, list[tuple]], k: int
) -> (list[tuple], list[tuple], list[int]):
    points = dict_to_list(points_dict)

    # TODO: Implement kmeans++
    # YANAI: You don't have to keep the meaningless key, I find the "observation index" using reverse lookup below
    init_centroids = points[:k]

    return points, init_centroids, get_keys(points_dict, init_centroids)


def dict_to_list(points_dict: dict[int, list[tuple]]) -> list[tuple]:
    return [tuple(point) for point in points_dict.values()]


def get_keys(points_dict: dict[int, list[tuple]], points: list[tuple]) -> list[int]:
    point_to_key = {tuple(v): k for k, v in points_dict.items()}
    return [point_to_key[point] for point in points]


def generate_clusters(
    points: list[tuple], init_centroids: list[tuple], k: int, max_iter: int, eps: float
) -> list[tuple]:
    """
    defines a double list, each sub list represent a cluster
    in each sub list the first value is the center of the cluster
    and the following values are indexes of Xi in the cluster
    """
    centroids = init_centroids
    iter = 0
    flag = True
    # flag represents if a cluster center had been updated by a value more distanced the epsilon
    while (iter < max_iter) and flag:
        flag = False
        clusters_indexes = [[] for i in range(k)]
        for i in range(len(points)):
            min_index = -1
            min_value = float("inf")
            for j in range(k):
                # tries to add Xi to a cluster and search for a new minimum for it
                dist = euclidean_distance(points[i], centroids[j])
                if dist < min_value:
                    min_index = j
                    min_value = dist
            clusters_indexes[min_index].append(i)
        for j in range(k):
            # Ignore empty clusters
            if len(clusters_indexes[j]) == 0:
                continue
            new_center = average_point(points, clusters_indexes[j])
            if euclidean_distance(new_center, centroids[j]) >= eps:
                flag = True
            centroids[j] = new_center
        iter += 1
    return centroids


def euclidean_distance(tup1: tuple, tup2: tuple) -> float:
    # Check dimentional consistancy
    if not len(tup1) == len(tup2):
        print("An Error Has Occurred")
        exit(1)
    distance = 0
    for i in range(len(tup1)):
        distance += (tup1[i] - tup2[i]) ** 2
    return sqrt(distance)


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
    centroids = generate_clusters(points, init_centroids, k, max_iter, eps)
    print_result(indices, centroids)
