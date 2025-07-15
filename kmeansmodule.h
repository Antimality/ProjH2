#ifndef KMEANSMODULE_H
#define KMEANSMODULE_H

/*
struct definitions
*/

struct cord {
    double value;
    struct cord *next;
};

struct vector {
    struct cord *cords;
    struct vector *next;
};

struct centroid {
    struct vector center;
    struct vector *vectors;
    struct centroid *next;
};

typedef struct cord cord;
typedef struct vector vector;
typedef struct centroid centroid;
typedef char bool;

/* Function prototypes */

/* Utility functions */
void free_cords(cord *head);
void free_vectors(vector *vec);
void free_centroids(centroid *centroid_list);
void free_all(vector *head_vec, centroid *head_cent);
cord *copy_cords(cord *original);
centroid *create_centroid(vector *center_input);
double euclidean_distance(vector *vec, centroid *cen);
short update_centroids(centroid *centroid_list, double eps);
bool add_vector_to_centroid(centroid *centroid_list, int i, vector *new_vector);

/* Main operation functions */

vector *initialize_vectors(int k, int max_iter);
centroid *initialize_centroids(int k, vector *head_vec);
bool assign_clusters(int max_iter, vector *head_vec, centroid *head_cent, double eps);

#endif // KMEANSMODULE_H
