#include "kmeansmodule.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef INFINITY
#define INFINITY 1e100
#endif

void free_cords(cord *head) {
    cord *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void free_vectors(vector *vec) {
    vector *temp;
    while (vec != NULL) {
        temp = vec;
        free_cords(vec->cords);
        vec = vec->next;
        free(temp);
    }
}

void free_centroids(centroid *centroid_list) {
    centroid *temp;
    while (centroid_list != NULL) {
        // Free cords of center vector
        free_cords(centroid_list->center.cords);

        // Free attached vectors (if any)
        free_vectors(centroid_list->vectors);

        // Free centroid struct
        temp = centroid_list;
        centroid_list = centroid_list->next;
        free(temp);
    }
}

void free_all(vector *head_vec, centroid *head_cent) {
    free_centroids(head_cent);
    free_vectors(head_vec);
}

cord *copy_cords(cord *original) {
    cord *new_head, *prev, *new_cord;

    if (original == NULL) {
        printf("Error Has Occurred\n");
        return NULL;
    }

    new_head = NULL;
    prev = NULL;

    while (original != NULL) {
        new_cord = (cord *)malloc(sizeof(cord));
        if (new_cord == NULL) {
            printf("Error Has Occurred\n");
            free_cords(new_head);
            return NULL;
        }
        new_cord->value = original->value;
        new_cord->next = NULL;

        if (prev == NULL) {
            new_head = new_cord;
        } else {
            prev->next = new_cord;
        }

        prev = new_cord;
        original = original->next;
    }

    return new_head;
}

double euclidean_distance(vector *vec, centroid *cen) {
    double dist, diff;
    cord *cord1, *cord2;

    dist = 0.0;
    cord1 = vec->cords;
    cord2 = cen->center.cords;
    while (cord1 != NULL && cord2 != NULL) {
        diff = cord1->value - cord2->value;
        dist += diff * diff;
        cord1 = cord1->next;
        cord2 = cord2->next;
    }
    return sqrt(dist);
}

centroid *create_centroid(vector *center_input) {
    centroid *new_cent;
    new_cent = (centroid *)malloc(sizeof(centroid));
    if (new_cent == NULL) {
        printf("Error Has Occurred\n");
        return NULL;
    }

    new_cent->next = NULL;
    new_cent->vectors = NULL;

    // Deep copy of the center vector
    new_cent->center.next = NULL;
    new_cent->center.cords = copy_cords(center_input->cords);
    if (new_cent->center.cords == NULL) {
        printf("Error Has Occurred\n");
        return NULL;
    }

    return new_cent;
}

short update_centroids(centroid *centroid_list, double eps) {
    bool flag;
    vector *vec, *avg_vec;
    cord *first_cord, *c, *s, *avg_cords, *sum_tail, *new_cord;
    int dimension, count, i;

    flag = 0;
    while (centroid_list != NULL) {
        vec = centroid_list->vectors;
        if (vec == NULL) {
            centroid_list = centroid_list->next;
            continue; // Skip if no vectors
        }

        // Get dimension from the first vector
        first_cord = vec->cords;
        dimension = 0;
        for (c = first_cord; c != NULL; c = c->next)
            dimension++;

        // Initialize avg_cords as a linked list of zeros
        avg_cords = NULL, sum_tail = NULL;
        for (i = 0; i < dimension; i++) {
            new_cord = (cord *)malloc(sizeof(cord));
            if (new_cord == NULL) {
                printf("Error Has Occurred\n");
                free_cords(avg_cords);
                return -1;
            }
            new_cord->value = 0.0;
            new_cord->next = NULL;
            if (avg_cords == NULL)
                avg_cords = new_cord;
            else
                sum_tail->next = new_cord;
            sum_tail = new_cord;
        }

        count = 0;
        // Sum values across all vectors
        while (vec != NULL) {
            c = vec->cords;
            s = avg_cords;
            while (c != NULL && s != NULL) {
                s->value += c->value;
                c = c->next;
                s = s->next;
            }
            count++;
            vec = vec->next;
        }

        // Compute average
        s = avg_cords;
        while (s != NULL) {
            s->value /= count;
            s = s->next;
        }

        // Compare with old center cords to check for convergence
        avg_vec = (vector *)malloc(sizeof(vector));
        if (avg_vec == NULL) {
            printf("Error Has Occurred\n");
            free_cords(avg_cords);
            return -1;
        }

        avg_vec->cords = avg_cords;
        if (euclidean_distance(avg_vec, centroid_list) >= eps)
            flag = 1;

        free(avg_vec);

        // Free old center cords if any
        free_cords(centroid_list->center.cords);

        // Assign new cords as center
        centroid_list->center.cords = avg_cords;

        // Free the vector list and reset to NULL
        free_vectors(centroid_list->vectors);
        centroid_list->vectors = NULL;

        centroid_list = centroid_list->next;
    }
    return flag;
}

bool add_vector_to_centroid(centroid *centroid_list, int i, vector *new_vector) {
    centroid *curr;
    vector *copy;
    int j;

    // Traverse to the i-th centroid
    curr = centroid_list;
    for (j = 0; j < i && curr != NULL; j++) {
        curr = curr->next;
    }

    // If i is out of bounds
    if (curr == NULL) {
        printf("Error Has Occurred\n");
        return 1;
    }

    // Create a deep copy of the vector
    copy = (vector *)malloc(sizeof(vector));
    if (copy == NULL) {
        printf("Error Has Occurred\n");
        return 1;
    }

    copy->cords = copy_cords(new_vector->cords);
    if (copy->cords == NULL) {
        printf("Error Has Occurred\n");
        free_vectors(copy);
        return 1;
    }

    // Add the new vector to the front of the centroid's vector list
    copy->next = curr->vectors;
    curr->vectors = copy;

    return 0;
}

centroid *initialize_centroids(int k, vector *head_vec) {
    centroid *head_cent, *curr_cent, *next_cent;
    vector *curr_vec;
    int i;

    head_cent = create_centroid(head_vec);
    curr_cent = head_cent;
    if (curr_cent == NULL) {
        printf("Error Has Occurred\n");
        return NULL;
    }
    next_cent = NULL;
    curr_vec = head_vec->next;
    for (i = 1; i < k; i++) {
        next_cent = create_centroid(curr_vec);
        curr_cent->next = next_cent;
        curr_cent = next_cent;
        if (curr_cent == NULL) {
            printf("Error Has Occurred\n");
            free_centroids(head_cent);
            return NULL;
        }
        curr_vec = curr_vec->next;
    }

    return head_cent;
}

bool assign_clusters(int max_iter, vector *head_vec, centroid *head_cent, double eps) {
    centroid *curr_cent;
    vector *curr_vec;
    bool flag, exit;
    double min;
    int iter, min_index, temp_index;
    double temp_min;

    curr_vec = head_vec;
    curr_cent = head_cent;
    iter = 0, min_index = 0;
    flag = 1;
    min = INFINITY;

    while (flag == 1 && iter < max_iter) {
        curr_vec = head_vec;
        while (curr_vec->cords != NULL) {
            min = INFINITY;
            min_index = 0;
            curr_cent = head_cent;
            temp_index = 0;
            while (curr_cent != NULL) {
                temp_min = euclidean_distance(curr_vec, curr_cent);
                if (temp_min < min) {
                    min = temp_min;
                    min_index = temp_index;
                }
                temp_index++;
                curr_cent = curr_cent->next;
            }
            exit = add_vector_to_centroid(head_cent, min_index, curr_vec);
            if (exit) {
                printf("Error Has Occurred\n");
                return 1;
            }
            curr_vec = curr_vec->next;
        }
        iter++;
        flag = update_centroids(head_cent, eps);
        if (flag == -1) {
            printf("Error Has Occurred\n");
            return 1;
        }
    }
    return 0;
}