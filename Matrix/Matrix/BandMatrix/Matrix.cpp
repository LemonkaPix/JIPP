#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "BandMatrix.h"

/*
y = Ax

gdzie  A = {a_{ij}}, x = {x_i}, y = {y_i}, i,j = 0,1,2,...,n-1 

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

Uwagi do projektu nr 3.
− Wszystkie tablice alokowane dynamicznie.
− Dane czytamy z pliku.
− Wyniki zapisujemy do pliku i wyświetlamy na monitorze.
− Pełna obsługa błędów dla wszystkich funkcji .
− Program podzielony na kilka plików (co najmniej 2 pliki).
*/

int main() {
    setlocale(LC_ALL, "");

    const char *inputFile = "input.txt";
    const char *outputFile = "output.txt";

    double **A = NULL;
    double *x = NULL;
    double *y = NULL;
    int n = 0, m = 0;

    int r = readInput(inputFile, &A, &x, &n, &m);
    if (r != 0) {
        switch (r) {
        case 1: printf("Błąd %d: Niepoprawne argumenty funkcji odczytu.\n", r); break;
        case 2: printf("Błąd %d: Nie można otworzyć pliku wejściowego.\n", r); break;
        case 3: printf("Błąd %d: Niepoprawny odczyt n lub m (wartości ujemne albo brak danych).\n", r); break;
        case 4: printf("Błąd %d: Alokacja pamięci dla wierszy macierzy \"A\" nie powiodła się.\n", r); break;
        case 5: printf("Błąd %d: Alokacja pamięci dla kolumn macierzy \"A\" nie powiodła się.\n", r); break;
        case 6: printf("Błąd %d: Odczyt elementu macierzy \"A\" nie powiódł się.\n", r); break;
        case 7: printf("Błąd %d: Alokacja pamięci dla wektora \"x\" nie powiodła się.\n", r); break;
        case 8: printf("Błąd %d: Odczyt elementu wektora \"x\" nie powiódł się.\n", r); break;
        default: printf("Nieznany błąd odczytu danych o kodzie %d.\n", r); break;
        }
        return 1;
    }

    y = (double*)malloc(sizeof(double) * n);
    if (!y) {
        printf("Błąd: Alokacja wektora y nie powiodła się.\n");

        for (int k = 0; k < n; ++k) 
            free(A[k]);
        
        free(A);
        free(x);
        return 1;
    }

    multiplyBandMatrixVector(A, x, y, n, m);

    printf("Wynik y = A x:\n");
    for (int i = 0; i < n; ++i) {
        printf("y[%d] = %.6f\n", i, y[i]);
    }

    int err = writeOutput(outputFile, y, n);
    if (err != 0) {
        switch (err) {
        case 1: printf("Błąd zapisu %d: Niepoprawne argumenty funkcji writeOutput.\n", err); break;
        case 2: printf("Błąd zapisu %d: Nie można otworzyć pliku wyjściowego.\n", err); break;
        case 3: printf("Błąd zapisu %d: Zapis do pliku nie powiódł się.\n", err); break;
        default: printf("Nieznany błąd zapisu o kodzie %d.\n", err); break;
        }
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
