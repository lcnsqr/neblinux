#ifndef MAT_h
#define MAT_h

#include <math.h>

int mat_leastsquares(int m, int n, float x[], float y[], float c[]);

int mat_elem(int rows, int cols, int r, int c);

// https://rosettacode.org/wiki/Matrix_multiplication#C
void mat_mult(const int m, const int n, const int p, float *const a, float *const b,
          float *c);

/*----------------------------------------------------------------------
gjinv - Invert a matrix, Gauss-Jordan algorithm
A is destroyed.  Returns 1 for a singular matrix.
https://rosettacode.org/wiki/Gauss-Jordan_matrix_inversion#C

___Name_____Type______In/Out____Description_____________________________
   a[n*n]   float*   In        An N by N matrix
   n        int       In        Order of matrix
   b[n*n]   float*   Out       Inverse of A
----------------------------------------------------------------------*/
int mat_inv(const int n, float *a, float *b);

#endif
