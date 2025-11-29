#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <locale.h>
#include <errno.h>


#define MAX_STRING_INPUT_LENGTH 100

int safeReadInt() {
    char buf[MAX_STRING_INPUT_LENGTH];
    long val;
    char* endptr;

    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Błąd odczytu\n");
            continue;
        }
        errno = 0;
        val = strtol(buf, &endptr, 10);
        if (errno == ERANGE || endptr == buf || (*endptr != '\n' && *endptr != '\0')) {
            printf("Niepoprawny format liczby\nSpróbuj ponownie: ");
            continue;
        }
        return (int)val;
    }
}

double safeReadDouble() {
    char buf[MAX_STRING_INPUT_LENGTH];
    double val;
    char* endptr;

    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Błąd odczytu\n");
            continue;
        }
        errno = 0;
        val = strtod(buf, &endptr);
        if (errno == ERANGE || endptr == buf || (*endptr != '\n' && *endptr != '\0')) {
            printf("Niepoprawny format liczby\nSpróbuj ponownie: ");
            continue;
        }

        if (isnan(val) || isinf(val)) {
            printf("Niepoprawna wartość (NaN lub nieskończoność)\nSpróbuj ponownie: ");
            continue;
        }
        return val;
    }
}

void reportError(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

double sinh_series(double x, int M, int* terms_used, int* stopped_by_eps) {
    double term = x;
    double sum = term;
    int n = 1;
    *stopped_by_eps = 0;

    if (M <= 0) {
        *terms_used = 0;
        return NAN;
    }

    while (n < M) {
        double prev_sum = sum;
        term *= x * x / ((2.0 * n) * (2.0 * n + 1.0));
        sum += term;
        n++;

        if (isinf(term) || isinf(sum) || isnan(term) || isnan(sum)) {
            reportError("Wykryto wartość nieskończoną lub NaN w obliczeniach. Przerywam program");
        }

        if (fabs(sum - prev_sum) < DBL_EPSILON) {
            *stopped_by_eps = 1;
            break;
        }
    }

    *terms_used = n;

    return sum;
}

int main() {
    setlocale(LC_ALL, "pl_PL");

    printf("Podaj początek przedziału (a): ");
    double a = safeReadDouble();
    printf("Podaj koniec przedziału (b): ");
    double b = safeReadDouble();
    printf("Podaj ilość punktów (n): ");
    
    int n = safeReadInt();
    if (n < 1) {
        reportError("Liczba punktów (n) musi być >= 1");
    }
    printf("Podaj maksymalną ilość iteracji funkcji (M): ");
    int M = safeReadInt();
    if (M < 1) {
        reportError("Musi być podane M >= 1");
    }
    printf("\n");

    FILE* f = fopen("wyniki.txt", "w");
    if (!f) {
        reportError("Nie udało się otworzyć pliku!\n");
        return 1;
    }

    fprintf(f, "x\tf_szereg(x)\tf_ściśle(x)\tliczba_wyrazów\twarunek_stopu\n");
    printf("x\tf_szereg(x)\tf_ściśle(x)\tliczba_wyrazów\twarunek_stopu\n");

    for (int i = 0; i < n; i++) {
        double x;
        if (n == 1) {
            x = a; // single point
        } else {
            x = a + (double)i * (b - a) / (double)(n - 1);
        }

        if (isnan(x) || isinf(x)) {
            reportError("Wartość x jest NaN lub nieskończona. Przerywam program");
        }

        int terms_used, stopped_by_eps;

        double f_series = sinh_series(x, M, &terms_used, &stopped_by_eps);
        double f_exact = sinh(x);

        if (isinf(f_series) || isinf(f_exact) || isnan(f_series) || isnan(f_exact)) {
            reportError("Wykryto wartość nieskończoną lub NaN w obliczeniach. Przerywam program");
        }

        fprintf(f, "%.4f\t%.8f\t%.8f\t%d\t%s\n", x, f_series, f_exact, terms_used,
            stopped_by_eps ? "dokładność" : "limit M");
        printf("%.4f\t%.8f\t%.8f\t%d\t%s\n", x, f_series, f_exact, terms_used,
            stopped_by_eps ? "dokładność" : "limit M");
    }

    fclose(f);
    return 0;
}
