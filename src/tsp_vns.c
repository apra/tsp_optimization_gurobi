#include "tsp_vns.h"

void tsp_vns_create(Tsp_prob *instance) {
    srand((unsigned) time(NULL));
    struct timespec start, cur;
    double time_elapsed = 0;
    //TODO CALCULATE COSTS ARRAY

    int n_node = instance->nnode;
    int n_edge = (n_node * (n_node - 1)) / 2;

    int *costs = malloc(n_edge * sizeof(double));
    int coord;
    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            coord = x_pos_metaheuristic(i, j, instance);
            costs[coord] = distance(i, j, instance);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    double *incumbent_solution = calloc(n_edge, sizeof(double));
    double *cur_solution = calloc(n_edge, sizeof(double));
    //double *local_best_solution = calloc(n_edge, sizeof(double));
    int *alloc_node_sequence = calloc(n_node + 1, sizeof(int));
    int *node_sequence = alloc_node_sequence;

    get_initial_heuristic_sol(instance, incumbent_solution, x_pos_metaheuristic);

    int best_value = two_opt(instance, incumbent_solution, node_sequence, costs);

    int delta;

    int iteration_count = 0;

    double kick_number = 1;
    int drag = 5;
    int max_kick_number = 5;
    int new_value = 0;

    do {
        //Shaking
        for (int i = 0; i < (int) kick_number; i++) {
            kick(instance, cur_solution, n_node, incumbent_solution);
        }

        new_value = two_opt(instance, cur_solution, node_sequence, costs);

        printf("HEURSOL %d, %d\n", iteration_count, new_value);

        if (new_value < best_value) {
            best_value = new_value;
            new_solution(instance, node_sequence, incumbent_solution);
            printf("Incumbent updated, iteration: %d \n", iteration_count);
            kick_number = 1;
        } else if (kick_number == max_kick_number) {
            kick_number = 1;
        } else {
            //increase size of neighborhood
            kick_number = (kick_number + 1.0 / drag > max_kick_number) ? kick_number : kick_number + 1.0 / drag;
        }
        iteration_count++;
        clock_gettime(CLOCK_MONOTONIC, &cur);
    } while ((cur.tv_sec - start.tv_sec) < instance->time_limit);

    printf("BEST SOL: %d \n", best_value);

    free(alloc_node_sequence);
    //free(incumbent_solution);
}