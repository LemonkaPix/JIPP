#ifndef BAND_MATRIX_H
#define BAND_MATRIX_H

int writeOutput(const char *filename, const double *y, int n);

void multiplyBandMatrixVector(double **A, const double *x, double *y, int n, int m);

int readInput(const char *filename, double ***A, double **x, int *n, int *m);

#endif
