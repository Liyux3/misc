/*
 * Jacobi solver with periodic grid snapshots for visualization.
 * Based on jacobi_seq.c, dumps grid state at scheduled iterations.
 *
 * gcc -O2 -o jacobi_viz jacobi_viz.c -lm
 * ./jacobi_viz <rows> <cols> <snapshot_dir>
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#define EPSILON 0.001
#define MAX_SNAPSHOTS 80

double **u, **w;
int M = 200, N = 200;
long max_its = 100000;
int snapshot_count = 0;
char snapshot_dir[256] = "snapshots";

/* Dense early, sparse late */
int snapshot_schedule[] = {
    0, 1, 3, 6, 12, 25, 50, 100, 180, 300, 500, 800,
    1200, 1800, 2500, 3500, 5000, 7000, 9000, 11000,
    14000, 18000, 22000, 27000, 33000, 40000, 50000,
    60000, 70000, 78000, -1
};
int sched_idx = 0;

void allocate_2d(int r, int c, double ***a) {
    double *storage = (double *)malloc(r * c * sizeof(double));
    *a = (double **)malloc(r * sizeof(double *));
    for (int i = 0; i < r; i++)
        (*a)[i] = &storage[i * c];
}

void initialize(double ***u) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++)
            (*u)[i][j] = 25.0;
        (*u)[i][0] = 0.0;
        (*u)[i][N-1] = 0.0;
    }
    for (int j = 0; j < N; j++) {
        (*u)[0][j] = 0.0;
        (*u)[M-1][j] = 1000.0;
    }
}

void dump_snapshot(double **grid, int iteration, double diff) {
    if (snapshot_count >= MAX_SNAPSHOTS) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/snap_%03d.csv", snapshot_dir, snapshot_count);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%d,%d,%d,%.6f\n", M, N, iteration, diff);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(f, "%.2f", grid[i][j]);
            if (j < N-1) fprintf(f, ",");
        }
        fprintf(f, "\n");
    }
    fclose(f);
    printf("  snap %02d: iter %6d, diff=%.4f\n", snapshot_count, iteration, diff);
    snapshot_count++;
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        M = atoi(argv[1]);
        N = atoi(argv[2]);
    }
    if (argc >= 4)
        strncpy(snapshot_dir, argv[3], sizeof(snapshot_dir) - 1);

    mkdir(snapshot_dir, 0755);
    printf("Jacobi viz: %dx%d, snapshots -> %s/\n", M, N, snapshot_dir);

    allocate_2d(M, N, &u);
    allocate_2d(M, N, &w);
    initialize(&u);
    initialize(&w);

    dump_snapshot(u, 0, 1000.0);
    sched_idx = 1;

    double diff;
    double **temp;
    int its;
    for (its = 1; its <= max_its; its++) {
        diff = 0.0;
        for (int i = 1; i < M-1; i++)
            for (int j = 1; j < N-1; j++) {
                w[i][j] = 0.25 * (u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1]);
                double d = fabs(w[i][j] - u[i][j]);
                if (d > diff) diff = d;
            }
        temp = u; u = w; w = temp;

        if (snapshot_schedule[sched_idx] >= 0 && its >= snapshot_schedule[sched_idx]) {
            dump_snapshot(u, its, diff);
            sched_idx++;
        }

        if (diff <= EPSILON) break;
    }

    dump_snapshot(u, its, diff);
    printf("Converged after %d iterations, %d snapshots\n", its, snapshot_count);
    return 0;
}
