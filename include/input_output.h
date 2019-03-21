//
// Created by samuele on 10/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H
#define TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H
/**
 * Parse the input from command line
 * @param argc The number of arguments
 * @param argv The arguments themselves
 * @param instance The pointer to the problem instance we are using
 */
void parse_input(int argc, char **argv, Tsp_prob *instance);

/**
 * Initialize the problem instance
 * @param instance The pointer to the problem instance
 * @return 0 if not a valid instance, 1 otherwise
 */
int init_instance(Tsp_prob *instance);

/**
 * plot the solution of a tsp problem, solved using gurobi, here we assume that the variables available are the ones
 * describing all edges (e.g. y(i,j) is the variable that is 1 if the edge is selected and 0 otherwise), this means also
 * self loops. If the variable is not defined, then var_pos should return -1. In the case of symmetric instances, the
 * graph will appear correctly even if both y(i,j) and y(j,i) are plotted because there will be 2 lines on top of
 * one another. it requires functions from plot_graph.c to function correctly.
 * @param instance  the tsp_prob instance
 * @param model the GRB model where the solution is stored
 * @param env the GRB environment, used for error printing
 * @param var_pos the function that return the index of the variable in the model
 * @return 1 if no error happened
 */
int plot_solution(Tsp_prob *instance, GRBmodel *model, GRBenv *env, int (*var_pos)(int, int, Tsp_prob*));

// print the help text for command line
void print_help();
#endif //TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H