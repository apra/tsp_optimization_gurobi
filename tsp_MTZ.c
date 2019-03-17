#include "common.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "tsp_MTZ.h"
#include "input_output.h"

int ypos(int i, int j, Tsp_prob *instance);
int upos(int i, Tsp_prob *instance);


void MTZ_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *MTZ_model = NULL;
    int error = 0;
    int n_nodes = instance->nnode;
    int n_variables = n_nodes*n_nodes + n_nodes;
    printf("Number of nodes: %d\n Number of variables: %d\n", n_nodes, n_variables);

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    int optim_status;
    double obj_val;
    double sol;

    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;

    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < n_nodes; j++) {
            coord = ypos(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0;
            if (i == j) {
                up_bound[coord] = 0;
            } else {
                up_bound[coord] = 1;
            }
            obj_coeff[coord] = distance(i, j, instance);
            variables_names[coord] = (char*) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "y(%d,%d)", i + 1, j + 1);
            printf("i:%d, j: %d\n", i + 1, j + 1);
        }
    }

    /*Add variables u*/

    for (int i = 0; i < n_nodes; i++) {
        coord = upos(i, instance);
        obj_coeff[coord] = 0.0;
        if (i == 0) {
            low_bound[coord] = 1.0;
            up_bound[coord] = 1.0;
        } else {
            low_bound[coord] = 2.0;
            up_bound[coord] = n_nodes;
        }
        var_type[coord] = GRB_INTEGER;
        variables_names[coord] = calloc(100, sizeof(char));

        sprintf(variables_names[coord], "u(%d)", i + 1);
    }

    /*error = GRBaddvars(MTZ_model, n_nodes, 0, NULL, NULL, NULL, u_obj_val, u_low_bound, u_up_bound, u_type, u_name);
    quit_on_GRB_error(env, MTZ_model, error);*/


    /*create environment*/
    error = GRBloadenv(&env, "MTZ.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &MTZ_model, "MTZ", 0, NULL, NULL, NULL, NULL, NULL);

    quit_on_GRB_error(env, MTZ_model, error);

    /*Add objective function elements*/
    error = GRBaddvars(MTZ_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type,
            variables_names);
    quit_on_GRB_error(env, MTZ_model, error);

    /***********
     * CONSTRAINTS
     ***********/

    int constr_index[n_nodes];
    double constr_value[n_nodes];
    double rhs = 1.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int index_cur_constr = 0;

    /*Add constraints for indegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(j, h, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, MTZ_model, error);
        index_cur_constr++;
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(h, j, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, MTZ_model, error);
        index_cur_constr++;
    }



    /*Add lazy constraints for y(i,j) + y(j, i) <= 1*/
    /*With a value of 1, the constraint can be used to cut off a feasible solution, but it won’t
    necessarily be pulled in if another lazy constraint also cuts off the solution. With a value of 2, all
    lazy constraints that are violated by a feasible solution will be pulled into the model. With a value
    of 3, lazy constraints that cut off the relaxation solution at the root node are also pulled in.
     */

    int var_index[2];
    double constr_val[2] = {1.0, 1.0};

    for (int i = 0; i < n_nodes; i++) {
        for (int j = i+1; j < n_nodes; j++) {
            var_index[0] = ypos(i, j, instance);
            var_index[1] = ypos(j, i, instance);
            sprintf(constr_name, "lazy_constr_(%d,%d)", i+1, j+1);

            error = GRBaddconstr(MTZ_model, 2, var_index, constr_val, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, MTZ_model, error);

            error = GRBsetintattrelement(MTZ_model, "Lazy", index_cur_constr, 1);
            quit_on_GRB_error(env, MTZ_model, error);
            index_cur_constr++;
        }
    }


    /*Add MTZ lazy constraints: u(j) >= u(i) +1 - M * (1 - y(i,j))*/
    // u(i) - u(j) + M * y(i,j) <= M - 1
    int M = n_nodes - 1;
    int MTZ_index[3];
    double MTZ_value[3] = {1.0, -1.0, M};


    for (int i = 1; i < n_nodes; i++) {
        for (int j = 1; j < n_nodes; j++) {
            if(i != j) {
                MTZ_index[0] = upos(i, instance);
                MTZ_index[1] = upos(j, instance);
                MTZ_index[2] = ypos(i, j, instance);
                sprintf(constr_name, "MTZ_constr_(%d,%d)", i+1, j+1);

                error = GRBaddconstr(MTZ_model, 3, MTZ_index, MTZ_value, GRB_LESS_EQUAL, M - 1, constr_name);
                quit_on_GRB_error(env, MTZ_model, error);

                error = GRBsetintattrelement(MTZ_model, "Lazy", index_cur_constr, 1);
                quit_on_GRB_error(env, MTZ_model, error);
                index_cur_constr++;
            }
        }
    }

    /*consolidate the model parameters*/
    error = GRBupdatemodel(MTZ_model);
    quit_on_GRB_error(env, MTZ_model, error);

    /*write model in an  output file*/
    error = GRBwrite(MTZ_model, "output_MTZ_model.lp");
    quit_on_GRB_error(env, MTZ_model, error);

    /*launch gurobi solver with the selected model*/
    error = GRBoptimize(MTZ_model);
    quit_on_GRB_error(env, MTZ_model, error);

    /* Capture solution information */
    error = GRBgetintattr(MTZ_model, GRB_INT_ATTR_STATUS, &optim_status);
    quit_on_GRB_error(env, MTZ_model, error);

    error = GRBgetdblattr(MTZ_model, GRB_DBL_ATTR_OBJVAL, &obj_val);
    quit_on_GRB_error(env, MTZ_model, error);

    /*error = GRBgetdblattrarray(MTZ_model, GRB_DBL_ATTR_X, 0, n_nodes, &sol);
    quit_on_GRB_error(env, MTZ_model, error);*/
    /*print solution in a file*/
    error = GRBwrite(MTZ_model, "MTZ_solution.sol");
    quit_on_GRB_error(env, MTZ_model, error);

    /*print solution informations*/
    printf("\nOptimization complete\n");
    if (optim_status == GRB_OPTIMAL) {
        printf("Optimal objective: %.4e\n", obj_val);
    } else if (optim_status == GRB_INF_OR_UNBD) {
        printf("Model is infeasible or unbounded\n");
    } else {
        printf("Optimization was stopped early\n");
    }

    plot_solution(instance, MTZ_model, env, &ypos);

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);
}

/**
 * Formulation-specific transformation used to get the id of the edge in the model.
 * @param i starting node
 * @param j ending node
 * @param instance the tsp instance
 * @return the id of the variable that identifies the edge from i to j
 */
int ypos(int i, int j, Tsp_prob *instance){
    return i*instance->nnode + j;
}
/**
 * Formulation-specific transformation used to get the id of the variable used to eliminate sub-tours by forcing an
 * ordered cycle.
 * @param i the node
 * @param instance the tsp instance
 * @return the id of the variable which determines the position of node i in the cycle
 */
int upos(int i, Tsp_prob *instance) {
    int latest_y_pos = ypos(instance->nnode - 1, instance->nnode - 1, instance);
    return latest_y_pos + 1 + i;
}
