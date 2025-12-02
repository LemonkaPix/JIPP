#include <stdio.h>
#include <stdlib.h>
#include "BandMatrix.h"

int writeOutput(const char *filename, const double *y, int n) {
    if (!filename || !y || n <= 0)
        return 1;

    FILE *f = fopen(filename, "w");
    if (!f) return 2;

    for (int i = 0; i < n; ++i) {
        if (fprintf(f, "%.6f\n", y[i]) < 0) {
            fclose(f);
            return 3;
        }
    }
    fclose(f);
    return 0;
}

void multiplyBandMatrixVector(double **A, const double *x, double *y, int n, int m) {
    int center = m / 2;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;

        for (int offset = -center; offset <= (m - center - 1); ++offset) {

            int col = i + offset;
            int bandIndex = offset + center;

            if (col >= 0 && col < n) {
                sum += A[i][bandIndex] * x[col];
            }
        }
        y[i] = sum;
    }
}

int readInput(const char *filename, double ***A, double **x, int *n, int *m) {
    if (!filename || !A || !x || !n || !m) return 1;

    FILE *f = fopen(filename, "r");
    if (!f) return 2;

    if (fscanf_s(f, "%d %d", n, m) != 2 || *n <= 0 || *m <= 0) {
        fclose(f);
        return 3;
    }

    *A = (double**)malloc(sizeof(double*) * (*n));
    if (!*A) {
        fclose(f);
        return 4;
    }

    for (int i = 0; i < *n; ++i) {
        (*A)[i] = (double*)malloc(sizeof(double) * (*m));

        if (!(*A)[i]) {
            for (int k = 0; k < i; ++k) 
                free((*A)[k]);
            free(*A);
            fclose(f);
            return 5;
        }
    }

    for (int i = 0; i < *n; ++i) {
        for (int j = 0; j < *m; ++j) {
            if (fscanf_s(f, "%lf", &(*A)[i][j]) != 1) {
                for (int k = 0; k < *n; ++k)
                    free((*A)[k]);
                free(*A);
                fclose(f);
                return 6;
            }
        }
    }

    *x = (double*)malloc(sizeof(double) * (*n));
    if (!*x) {
        for (int k = 0; k < *n; ++k) 
            free((*A)[k]);
        free(*A);
        fclose(f);
        return 7;
    }

    for (int i = 0; i < *n; ++i) {
        if (fscanf_s(f, "%lf", &(*x)[i]) != 1) {
            for (int k = 0; k < *n; ++k) 
                free((*A)[k]);
            free(*A);
            free(*x);
            fclose(f);
            return 8;
        }
    }

    fclose(f);
    return 0;
}
