//
// Created by samuele on 19/05/19.
//

#include "tsp_grasp.h"


void grasp_randomized_construction(Tsp_prob *instance, double *solution, int *edge_cost);

void grasp_local_search(Tsp_prob *instance, double *solution, double time_limit, int method,
                        int (*var_pos)(int, int, Tsp_prob *));

//void update_alpha(double *alpha_list, int selected_alpha, int incumbent_value, double *avg_incumb_value);

int x_pos_grasp(int i, int j, Tsp_prob *instance);

int reverse_x_pos_grasp(int pos, int i, Tsp_prob *instance);

double prob_in_range(double min, double max);

double grasp_cost_solution(Tsp_prob *instance, double *solution);

void GRASP(Tsp_prob *instance) {
    //the time limit is interpreted as an iteration count in GRASP
    int iteration_count = (int) (instance->time_limit);
    int cur_iteration = 0;
    int num_var = (instance->nnode * (instance->nnode - 1)) / 2;
    int *edge_cost = calloc(num_var, sizeof(int));
    double *solution = calloc(num_var, sizeof(double));

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            edge_cost[x_pos_grasp(i, j, instance)] = distance(i, j, instance);
        }
    }

    /*int alpha_list_size = 5;
    double *alpha_list = calloc(alpha_list_size, sizeof(double));
    double *avg_incumb_value = calloc(alpha_list_size, sizeof(double));

    for (int i = 0; i < alpha_list_size; i++) {
        alpha_list[i] = 1.0 / alpha_list_size;
    }*/

    while (cur_iteration < iteration_count) {

        grasp_randomized_construction(instance, solution, edge_cost);

        grasp_local_search(instance, solution, instance->time_limit, 0, x_pos_grasp);

        /*if (cur_iteration > 1) {
            int selected_sol_pool = (genrand64_int64() / ULLONG_MAX) * (num_pool_element - 1);
            path_relinking(sol_pool[selected_sol_pool], solution);
        }*/

        /*int selected_alpha = (genrand64_int64() / ULLONG_MAX) * (alpha_list_size - 1);
        double alpha = alpha_list[selected_alpha];*/

        printf("GENSOL %d, %g\n", cur_iteration, grasp_cost_solution(instance, solution));
        cur_iteration++;
    }

    /*free(alpha_list);
   free(avg_incumb_value);*/
    free(edge_cost);
    free(solution);
}

void grasp_randomized_construction(Tsp_prob *instance, double *solution, int *edge_cost) {

    int n_edge = (instance->nnode * (instance->nnode - 1)) / 2;
    int *not_available_edge = calloc(n_edge, sizeof(int));

    int *rcl = calloc(instance->nnode - 1, sizeof(int));
    int n_element_rcl;

    int l;

    int start_node = (genrand64_int64() / ULLONG_MAX) * (instance->nnode - 1);
    int cur_node = start_node;
    int n_selected_edge = 0;
    int incumbent_value = 0;

    init_genrand64(time(NULL));

    while (n_selected_edge < instance->nnode - 1) {

        l = 0;
        n_element_rcl = 0;

        int c_min = INT_MAX;
        int c_max = 0;

        for (int i = 0; i < n_edge && !not_available_edge[i]; i++) {
            if (edge_cost[i] < c_min) {
                c_min = edge_cost[i];
            }

            if (edge_cost[i] > c_max) {
                c_max = edge_cost[i];
            }
        }

        double alpha = prob_in_range(0.5, 0.9);

        double threshold = c_min + alpha * (c_max - c_min);

        for (int j = 0; j < instance->nnode; j++) { //populate the rcl
            if (j != cur_node) {
                int pos = x_pos_grasp(cur_node, j, instance);
                if (edge_cost[pos] <= threshold && !not_available_edge[pos]) {
                    rcl[l] = pos;
                    not_available_edge[pos] = 1;
                    n_element_rcl++;
                    l++;
                }
            }

        }

        int choose_pos_rcl = (genrand64_int64() / ULLONG_MAX) * (n_element_rcl - 1);

        int choose_edge = rcl[choose_pos_rcl];

        solution[choose_edge] = 1.0;

        incumbent_value += edge_cost[choose_edge];

        //update_alpha(alpha_list, selected_alpha, incumbent_value, avg_incumb_value);

        for (int i = 0; i < instance->nnode; i++) {
            if (i != cur_node) {
                int pos = x_pos_grasp(cur_node, i, instance);
                not_available_edge[pos] = 1;
            }
        }

        cur_node = reverse_x_pos_grasp(choose_edge, cur_node, instance);

        n_selected_edge++;
    }

    solution[x_pos_grasp(start_node, cur_node, instance)] = 1.0;

    free(rcl);
    free(not_available_edge);

    /*struct timespec start, cur;
    double time_elapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    unsigned long long int seed = (unsigned long long int)start.tv_nsec;
    init_genrand64(seed);


    double breathing = 0.5;

    while (time_elapsed < time_limit) {

        clock_gettime(CLOCK_MONOTONIC, &cur);
        time_elapsed = (cur.tv_sec - start.tv_sec);
        printf("GENSOL %g\n", cost_solution(instance, solution, var_pos));
    }*/


}

void grasp_local_search(Tsp_prob *instance, double *solution, double time_limit, int method,
                           int (*var_pos)(int, int, Tsp_prob *)) {
    //2-opt
}

void grasp_get_solution_pool(Tsp_prob *instance, double **solution_pool, double time_limit, int method) {

}

int x_pos_grasp(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return x_pos_grasp(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

double prob_in_range(double min, double max) {

    double p_value;

    do {
        p_value = genrand64_real1();

    } while(p_value < min || p_value > max);

    return p_value;
}

int reverse_x_pos_grasp(int pos, int i, Tsp_prob *instance) {

    int j = pos - i * instance->nnode + ((i + 1) * (i + 2)) / 2;

    if (i == j) {
        return -1;
    }

    return j;
}

double grasp_cost_solution(Tsp_prob *instance, double *solution) {
    double cost = 0;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            int coord = x_pos_grasp(i, j, instance);
            if (solution[coord] > TOLERANCE) {
                cost += distance(i, j, instance);
            }
        }
    }
    return cost;
}