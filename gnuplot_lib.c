
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gnuplot_lib.h"


#define GPC_TERM    "x11"
#define GNUPLOT_CMD "gnuplot -p"

/********************************************************
* Function : gpc_init_xy
*
* Parameters :
*   const char *plotTitle,
*   const char *xLabel,
*   const char *yLabel,
*   const enum gpcKeyMode keyMode
 *   const double xmin
 *   const double ymin
*
* Return value :
*   h_GPC_Plot - Plot handle
*
*
********************************************************/

GPC_Plot *gpc_init_xy(const char *plotTitle,
                        const double xmin,
                        const double ymin,
                        const double xmax,
                        const double ymax) {
    GPC_Plot *plotHandle;                                 // Create plot

    plotHandle = (GPC_Plot *) malloc(sizeof(GPC_Plot));    // Malloc plot and check for error
    if (plotHandle == NULL) {
        return (plotHandle);
    }

    plotHandle->pipe = popen(GNUPLOT_CMD, "w");
    if (plotHandle->pipe == NULL) {
        printf("\nGnuplot Error\n");
        printf("Unable to find the required Gnuplot executable or unable to open pipe.\n");

        free(plotHandle);
        return (plotHandle);
    }

    plotHandle->xMin = xmin;// Set dimension in handle
    plotHandle->xMax = xmax;
    plotHandle->yMin = ymin;
    plotHandle->yMax = ymax;
    strcpy(plotHandle->plotTitle, plotTitle);              // Set plot title in handle

    fprintf(plotHandle->pipe, "set term %s 0 title \"%s\" size %u, %u\n", GPC_TERM, plotHandle->plotTitle, CANVAS_WIDTH,
            CANVAS_HEIGHT); // Set the plot

    fprintf(plotHandle->pipe, "unset border\n");
    fprintf(plotHandle->pipe, "set xtics axis nomirror\n");
    fprintf(plotHandle->pipe, "set ytics axis nomirror\n");
    fprintf(plotHandle->pipe, "unset rtics\n");
    fprintf(plotHandle->pipe, "set zeroaxis\n");
    fprintf(plotHandle->pipe, "unset key\n");
    fprintf(plotHandle->pipe, "set multiplot\n");

    // Disable legend / key


    fflush(plotHandle->pipe);                                  // flush the pipe

    return (plotHandle);
}


/********************************************************
* Function : gpc_plot_xy
*
* Parameters :
*   h_GPC_Plot *plotHandle,
*   const ComplexRect_s *pData,
*   const int graphLength,
*   const char *pDataName,
*   const char *plotType,
*   const char *pColour,
*   const enum gpcNewAddGraphMode addMode)
*
* Return value :
*   int - error flag
*
* Description : Generate the XY plot
*
********************************************************/

void gpc_plot_xy(GPC_Plot *plotHandle,
                 const Points *pData,
                 const int pLength,
                 const char *pDataName,
                 const char *plotType,
                 const char *pColour) {
    int i;

    fprintf(plotHandle->pipe, "set xrange[%1.6le:%1.6le]\n", plotHandle->xMin, plotHandle->xMax);
    fprintf(plotHandle->pipe, "set yrange[%1.6le:%1.6le]\n", plotHandle->yMin, plotHandle->yMax);

    fprintf(plotHandle->pipe, "set style fill transparent solid 1.0 noborder\n");  // Set plot format
    fprintf(plotHandle->pipe, "set style circle radius  %1.6le\n", POINT_SIZE);  // Set plot format

    fprintf(plotHandle->pipe, "plot '-' title \"%s\" with %s lc rgb \"%s\" \n", pDataName, plotType, pColour);  // Set plot format
    for (i = 0; i < pLength; i++) {
        fprintf(plotHandle->pipe, "%1.6le %1.6le\n", pData[i].x, pData[i].y);
    }

    fprintf(plotHandle->pipe, "e\n");                      // End of dataset

    fflush(plotHandle->pipe);                              // Flush the pipe


}

void plot_line(Tsp_prob *instance, GPC_Plot *plotHandle, int frompoint, int topoint ){

    fprintf(plotHandle->pipe, "set style line 1 linecolor rgb \"red\" linetype 1 linewidth 2\n");  // Set plot format
    fprintf(plotHandle->pipe, "plot '-' title \" grafico \" with  linespoints linestyle 1 \n");  // Set plot format
    fprintf(plotHandle->pipe, "%1.6le %1.6le\n", instance->coord_x[frompoint], instance->coord_y[frompoint]);
    fprintf(plotHandle->pipe, "%1.6le %1.6le\n", instance->coord_x[topoint], instance->coord_y[topoint]);
    fprintf(plotHandle->pipe, "e\n");                      // End of dataset

    fflush(plotHandle->pipe);
}


/********************************************************
* Function : gpc_close
*
* Parameters :
*   h_GPC_Plot *plotHandle
*
* Return value :
*   void
*
* Description : Delete the plot and temporary files
*   associated with the handle.
*
********************************************************/

void gpc_close(GPC_Plot *plotHandle) {

    fprintf(plotHandle->pipe, "exit\n");                   // Close GNUPlot
    pclose(plotHandle->pipe);                              // Close the pipe to Gnuplot

    free(plotHandle);                                      // Free the plot
}

