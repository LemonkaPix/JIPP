#include <stdio.h>
#include <stdlib.h>

/*
y = Ax

gdzie ( A = {a_{ij}},; x = {x_i},; y = {y_i},; i,j = 0,1,2,,n-1 )

Ponadto macierz A jest macierzą pasmową. Macierz pasmowa to macierz, w
której niezerowe elementy skupione są wokół przekątnej. Poniżej
przedstawiona jest schematycznie struktura macierzy pasmowej, w której
elementy przekątniowe zaznaczone zostały gwiazdkami, elementy niezerowe
literą x.

* x x x 0 0 0 0 0 0 0 0
x * x x x 0 0 0 0 0 0 0
x x * x x x 0 0 0 0 0 0
x x x * x x x 0 0 0 0 0
0 x x x * x x x 0 0 0 0
0 0 x x x * x x x 0 0 0
0 0 0 x x x * x x x 0 0
0 0 0 0 x x x * x x x 0
0 0 0 0 0 x x x * x x x
0 0 0 0 0 0 x x x * x x
0 0 0 0 0 0 0 x x x * x
0 0 0 0 0 0 0 0 x x x *

W powyższym przykładzie rozmiar macierzy n = 12, a szerokość pasma m = 7.
Zwykle n >> m, tak że w budowanym algorytmie należy uwzględnić strukturę podanej macierzy.
Należy założyć, że jedynie elementy z pasma macierzy będą przechowywane w pamięci operacyjnej.
Tak, więc zamiast macierzy o rozmiarze n×n wystarczy przechowywać macierz o rozmiarze n×m,
tak więc dane będą miały postać:

0 0 0 * x x x
0 0 x * x x x
0 x x * x x x
x x x * x x x
x x x * x x x
x x x * x x x
x x x * x x x
x x x * x x x
x x x * x x x
x x x * x x 0
x x x * x 0 0
x x x * 0 0 0

Do takiej właśnie postaci danych należy dostosować algorytm mnożenia
macierzy przez wektor. Algorytm zapisz w funkcji.

Uwagi do projektu nr3.
− Wszystkie tablice alokowane dynamicznie.
− Dane czytamy z pliku.
− Wyniki zapisujemy do pliku i wyświetlamy na monitorze.
− Pełna obsługa błędów dla wszystkich funkcji .
− Program podzielony na kilka plików (co najmniej 2 pliki).
*/

// Prosty zapis wektora y do pliku
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

int main() {
    const char *inputFile = "input.txt";
    const char *outputFile = "output.txt";

    FILE *f = fopen(inputFile, "r");
    if (!f) {
        printf("Nie moge otworzyc pliku %s\n", inputFile);
        return 1;
    }

    int n = 0, m = 0;
    if (fscanf_s(f, "%d %d", &n, &m) != 2 || n <= 0 || m <= 0) {
        printf("Blad odczytu n i m\n");
        fclose(f);
        return 1;
    }

    double **A = (double**)malloc(sizeof(double*) * n);

    if (!A) { 
        fclose(f); printf("Blad alokacji wierszy macierzy\n"); 
        return 1; 
    }

    for (int i = 0; i < n; ++i) {
        A[i] = (double*)malloc(sizeof(double) * m);

        if (!A[i]) {
            printf("Blad alokacji kolumn macierzy\n");

            for (int k = 0; k < i; ++k) 
                free(A[k]);

            free(A);
            fclose(f);
            return 1;
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) 
        {
            if (fscanf_s(f, "%lf", &A[i][j]) != 1) {
                printf("Blad odczytu macierzy (i=%d, j=%d)\n", i, j);

                for (int k = 0; k < n; ++k)
                    free(A[k]);
                free(A);

                fclose(f);
                return 1;
            }
        }
    }

    double *x = (double*)malloc(sizeof(double) * n);

    if (!x) {
        printf("Blad alokacji wektora x\n");

        for (int k = 0; k < n; ++k)
            free(A[k]);

        free(A);
        fclose(f);
        return 1;
    }

    for (int i = 0; i < n; ++i) {
        if (fscanf_s(f, "%lf", &x[i]) != 1) 
        {
            printf("Blad odczytu wektora x (i=%d)\n", i);

            for (int k = 0; k < n; ++k) 
                free(A[k]);

            free(A);
            free(x);
            fclose(f);
            return 1;
        }
    }
    fclose(f);

    double *y = (double*)malloc(sizeof(double) * n);

    if (!y) {
        printf("Blad alokacji wektora y\n");

        for (int k = 0; k < n; ++k) 
            free(A[k]);
        
        free(A);
        free(x);
        return 1;
    }

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

    printf("Wynik y = A x:\n");
    for (int i = 0; i < n; ++i) {
        printf("y[%d] = %.6f\n", i, y[i]);
    }

    int err = writeOutput(outputFile, y, n);
    if (err != 0) {
        printf("Blad zapisu do pliku (kod=%d)\n", err);
    } else {
        printf("Wynik zapisany do pliku %s\n", outputFile);
    }

    for (int i = 0; i < n; ++i) 
        free(A[i]);

    free(A);
    free(x);
    free(y);

    return 0;
}
