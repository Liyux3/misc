/*
 * File name: jacobi_cond_3036100349.c
 * Student name: Yuxian LI
 * Student number: 3036100349
 * Development platform: Apple MacBook Air M2, Docker Ubuntu Linux
 * Remark: Complete implementation of parallel Jacobi iterative method.
 *         Uses pthread mutex and condition variables for barrier synchronization.
 *         Worker threads compute assigned row ranges, synchronize after each
 *         iteration, and return execution statistics upon termination via pthread_exit.
 */



#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>              /* For timing */
#include <sys/time.h>            /* For timing */
#include <sys/resource.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>


/****************Global****************************/

#define MAX(a,b) ((a)>(b)?(a):(b))
#define EPSILON 0.001            /* Termination condition */

char filename[30];                  /* File name of output file */

/* Grid size */
int M = 200;                     /* Number of rows */
int N = 200;                     /* Number of cols */
long max_its = 1000000;          /* Maximum iterations, a safe bound to avoid infinite loop */
double final_diff;               /* Temperature difference between iterations at the end */

/* Thread count */
int thr_count = 2;

/* shared variables between threads */
/*************************************************************/
double** u;                   /* Previous temperatures */
double** w;                   /* New temperatures */


// (1) Add your variables here

// Synchronization primitives
pthread_mutex_t mutex;
pthread_cond_t workers_done;
pthread_cond_t master_ready;

// Shared state locked by mutex
int workers_finished = 0;
bool should_terminate = false;
double global_diff = 0.0;

// Thread management
pthread_t *threads;

// For passing data to worker threads
typedef struct {
    int thread_id;
    int start_row; // First row this thread computes (inclusive)
    int end_row; // Last row this thread computes (inclusive)
} thread_data_t;

thread_data_t *thread_args;

typedef struct {
    double user_time;
    double system_time;
} thread_stats_t;


/**************************************************************/

int main (int argc, char *argv[])
{
   int      its;                 /* Iterations to converge */
   double   elapsed;             /* Execution time */
   struct timeval stime, etime;  /* Start and end times */
   struct rusage usage;

   void allocate_2d_array (int, int, double ***);
   void initialize_array (double ***);
   void print_solution (char *, double **);
   int  find_steady_state (void);

   /* For convenience of other problem size testing */
   if ((argc == 1) || (argc == 4)) {
      if (argc == 4) {
         M = atoi(argv[1]);
         N = atoi(argv[2]);
         thr_count = atoi(argv[3]);
      } // Otherwise use default grid and thread size
   } else {
     printf("Usage: %s [ <rows> <cols> <threads ]>\n", argv[0]);
     exit(-1);
   }

   printf("Problem size: M=%d, N=%d\nThread count: T=%d\n", M, N, thr_count);

   /* Create the output file */
   sprintf(filename, "%s_%d_%d_%d.dat", argv[0], M, N, thr_count);

   allocate_2d_array (M, N, &u);
   allocate_2d_array (M, N, &w);
   initialize_array (&u);
   initialize_array (&w);

   gettimeofday (&stime, NULL);
   its = find_steady_state();
   gettimeofday (&etime, NULL);

   elapsed = ((etime.tv_sec*1000000+etime.tv_usec)-(stime.tv_sec*1000000+stime.tv_usec))/1000000.0;

   printf("Converged after %d iterations with error: %8.6f.\n", its, final_diff);
   printf("Elapsed time = %8.4f sec.\n", elapsed);

   /* get the resource usage of the whole process */
   getrusage(RUSAGE_SELF, &usage);
   printf("Program completed - user: %.4f s, system: %.4f s\n",
      (usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0),
    (usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0));
   printf("no. of context switches: vol %ld, invol %ld\n\n",
  		  usage.ru_nvcsw, usage.ru_nivcsw);

   print_solution (filename, w);
}

/* Allocate two-dimensional array. */
void allocate_2d_array (int r, int c, double ***a)
{
   double *storage;
   int     i;
   storage = (double *) malloc (r * c * sizeof(double));
   *a = (double **) malloc (r * sizeof(double *));
   for (i = 0; i < r; i++)
      (*a)[i] = &storage[i * c];
}

/* Set initial and boundary conditions */
void initialize_array (double ***u)
{
   int i, j;

   /* Set initial values and boundary conditions */
   for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++)
         (*u)[i][j] = 25.0;      /* Room temperature */
      (*u)[i][0] = 0.0;
      (*u)[i][N-1] = 0.0;
   }

   for (j = 0; j < N; j++) {
      (*u)[0][j] = 0.0;
      (*u)[M-1][j] = 1000.0;     /* Heat source */
   }
}

/* Print solution to standard output or a file */
void print_solution (char *filename, double **u)
{
   int i, j;
   char sep;
   FILE *outfile;

   if (!filename) { /* if no filename specified, print on screen */
      sep = '\t';   /* tab added for easier view */
      outfile = stdout;
   } else {
      sep = '\n';   /* for gnuplot format */
      outfile = fopen(filename,"w");
      if (outfile == NULL) {
         printf("Can't open output file.");
         exit(-1);
      }
   }

   /* Print the solution array */
   for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++)
         fprintf (outfile, "%6.2f%c", u[i][j], sep);
      fprintf(outfile, "\n"); /* Empty line for gnuplot */
   }
   if (outfile != stdout)
      fclose(outfile);

}

/* Entry function of the worker threads */
void *thr_func(void *arg) {
    // (2) Add the worker's logic here
    // Extract my work
    thread_data_t *data = (thread_data_t *)arg;
    int thread_id = data->thread_id;
    int start_row = data->start_row;
    int end_row = data->end_row;

    int i, j;
    double local_diff;
    bool terminate = false;

    // Main loop, only stop when master signals
    while (!terminate) {
        local_diff = 0.0;
        for (i = start_row; i <= end_row; i++) {
            for (j = 1; j < N-1; j++) {
                w[i][j] = 0.25 * (u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1]);
                double change = fabs(w[i][j] - u[i][j]);
                if (change > local_diff)
                    local_diff = change;
            }
        }

        // Synchronize with other workers/master
        pthread_mutex_lock(&mutex);

        workers_finished++;
        if (local_diff > global_diff)
            global_diff = local_diff;

        if (workers_finished == thr_count) {
            pthread_cond_signal(&workers_done);
        }

        // Wait for master to decide
        pthread_cond_wait(&master_ready, &mutex);

        terminate = should_terminate;

        pthread_mutex_unlock(&mutex);
    }

    // Cleanup and collect stats

    struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);

    thread_stats_t *stats = malloc(sizeof(thread_stats_t));
    stats->user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
    stats->system_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;

    pthread_exit(stats);
}


int find_steady_state (void) {
    // (3) Implement the thread creation and the main control logic here

    // Initialization
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&workers_done, NULL);
    pthread_cond_init(&master_ready, NULL);

    threads = malloc(thr_count * sizeof(pthread_t));
    thread_args = malloc(thr_count * sizeof(thread_data_t));

    // Calculate row distribution
    int interior_rows = M - 2;
    int rows_per_thread = interior_rows / thr_count;
    int extra_rows = interior_rows % thr_count;

    for (int t = 0; t < thr_count; t++) {
        thread_args[t].thread_id = t;
        thread_args[t].start_row = 1 + t * rows_per_thread + (t < extra_rows ? t : extra_rows);
        thread_args[t].end_row = thread_args[t].start_row + rows_per_thread - 1;
        if (t < extra_rows)
            thread_args[t].end_row++;
    }

    // Create worker threads
    for (int t = 0; t < thr_count; t++) {
        pthread_create(&threads[t], NULL, thr_func, &thread_args[t]);
    }

    // Iterations
    int its;
    double diff;
    double **temp;

    for (its = 1; its <= max_its; its++) {

        // Wait for all workers
        pthread_mutex_lock(&mutex);
        while (workers_finished < thr_count) {
            pthread_cond_wait(&workers_done, &mutex);
        }

        // All done, get diff
        diff = global_diff;

        // Reset for next iteration
        workers_finished = 0;
        global_diff = 0.0;

        pthread_mutex_unlock(&mutex);

        // Update matrices
        temp = u;
        u = w;
        w = temp;

        // Check convergence
        if (diff <= EPSILON) {
            pthread_mutex_lock(&mutex);
            should_terminate = true;
            pthread_cond_broadcast(&master_ready);
            pthread_mutex_unlock(&mutex);
            break;
        }


        pthread_mutex_lock(&mutex);
        pthread_cond_broadcast(&master_ready);
        pthread_mutex_unlock(&mutex);
    }

    // If not converged
    if (its > max_its) {
        pthread_mutex_lock(&mutex);
        should_terminate = true;
        pthread_cond_broadcast(&master_ready);
        pthread_mutex_unlock(&mutex);
        diff = global_diff;
    }

    for (int t = 0; t < thr_count; t++) {
        thread_stats_t *stats;
        pthread_join(threads[t], (void**)&stats);
        printf("Thread %d has completed - user: %.4f s, system: %.4f s\n",
               t, stats->user_time, stats->system_time);
        free(stats);
    }

    // Cleanup
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&workers_done);
    pthread_cond_destroy(&master_ready);
    free(threads);
    free(thread_args);

    // Get Master's stats
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("find_steady_state - user: %.4f s, system: %.4f s\n",
           (usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0),
           (usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0));

    final_diff = diff;
    return its;
}
