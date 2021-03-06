//
// Created by samuele on 04/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_COMMON_H
#define TSP_OPTIMIZATION_GUROBI_COMMON_H


#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "gurobi_c.h"
#include "mt64.h"

#define DEBUG
#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

#define TOLERANCE 10E-4

typedef struct {
    int verbosity;
    double time_limit;
    double time_taken;
    int status;

    char *name; //TODO free memory after allocation
    char *comment; //TODO free memory after allocation

    int type; // The type of problem 0: TSP | in future add multiple types
    int nnode; //number of nodes
    int weight_type; //type of weight between edges (maybe pointer to some function?)
    /**for symmetric travelling salesman problems:
     *  0 = EUC_2D       : weights are Euclidean distances in 2-D
     *  1 = MAX_2D       : weights are maximum distances in 2-D
     *  2 = MAN_2D       : weights are Manhattan distances in 2-D
     *  3 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
     *  4 = GEO          : weights are geographical distances
     *  5 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
     */

    int model_type; //The user selected model

    double *coord_x; //list of x coordinates //TODO free memory after allocation
    double *coord_y; //list of y coordinates //TODO free memory after allocation

    // **weight_matrix; //weight value matrix

    char *filename; //TODO free memory after allocation

    int **solution; //TODO free memory after allocation
    int solution_size;

    int seed;

    GRBenv *env;
    GRBmodel *model;

    //best solution found for this instance using this model
    double best_solution;

    //solution informations for the heuristic model
    double *heuristic_solution;
    int heuristic_repetition;

    //latest best heuristic solution value
    double best_heur_sol_value;

    //probability of choose an edge
    double prob;

    //selected method to use in matheuristic model
    int black_box;

    //method used for the first heuristic solution of the problem (bad name, but better than first_heuristic_sol)
    int warm_start;

    //the constraint position in the heuristic model
    int heuristic_constr_index;

    //keep track if it is the first iteration
    int first_heur_iteration;

    //the k value for local branch constraint
    int k_value;

    //list of edges in the solution, all variables are present
    double *solution_edges;

} Tsp_prob;

typedef struct {
    ///the trials file
    char *filename;
    char *name;
    ///number of runs to perform
    int n_runs;
    ///number of instances
    int n_instances;
    ///number of models present
    int n_models;
    ///the list of seeds
    int *seeds;
    ///the list of instances to run
    char **instances;
    ///the list of models to use
    int *models;
    ///time limit for each run
    double time_limit;

    Tsp_prob **problems;
} Trial;

typedef struct {
    int *comps; //list correlation node component
    int number_of_comps; //number of component
    int *number_of_items; //number of node in each component
    int *list_of_comps; //list of component name
    //int *visit_flag; //visited node
} Connected_comp;

typedef struct {
    int **solution;
    int size;
} Solution_list;

#endif //TSP_OPTIMIZATION_GUROBI_COMMON_H
