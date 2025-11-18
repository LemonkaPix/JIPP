#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>

#define MAX_STRING_INPUT_LENGTH 250

int isReservedWindowsName(const char* name) {
    const char* reserved[] = {
        "CON", "PRN", "AUX", "NUL", "CLOCK$",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    size_t len = 0;
    while (name[len] && name[len] != '.' && len < 31) ++len;

    for (int i = 0; i < (int)(sizeof(reserved) / sizeof(reserved[0])); ++i) {
        if (strlen(reserved[i]) == len && _strnicmp(name, reserved[i], len) == 0)
            return 1;
    }
    return 0;
}

void reportError(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void safeReadString(char* buffer, size_t bufsize) {
    if (bufsize < 2) reportError("Buffer too small");
    while (1) {
        if (!fgets(buffer, (int)bufsize, stdin)) reportError("Błąd odczytu napisu");
        if (strchr(buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Przekroczono maksymalną długość napisu %zu znaków\nSpróbuj ponownie: ", bufsize - 1);
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') { printf("Puste wejście. Spróbuj ponownie: "); continue; }
        break;
    }
}

/*
Projekt nr 2 - operacje na tablicach tekstowych z dynamicznym alokowaniem pamięci.
Napisz program szyfrujący i deszyfrujący. Szyfrowanie wykonywane jest na podstawie 
klucza złożonego z kilku liczb (w programie należy założyć dowolną długość klucza). 
Każda liczba określa sposób przekształcenia znaku w inny znak. 
Pierwszy znak jest przekształcany wg pierwszej wartości z klucza, drugi wg drugiej itd., 
po wyczerpaniu wszystkich składowych klucza, należy powrócić do pierwszej wartości. 
Jak będą przekształcane znaki? Wszystkie znaki podlegające szyfrowaniu można ustawić w ciąg 
(np. zgodnie z ich kodami). Znak szyfrowany należy zastąpić znakiem odległym o wartość odpowiedniego 
elementu klucza. Zakładając, że ciąg dopuszczalnych znaków ma K elementów a przekształcenie powyższe 
prowadzi np. do K+4 znaku, to należy pobrać znak stojący na czwartej pozycji w ciągu.

Wytyczne do projektu nr 2.
− Wszystkie tablice alokowane dynamicznie.
− Tekst składa się z wielu linii.
− Dane czytamy z pliku.
− Wyniki zapisujemy do pliku i wyświetlamy na monitorze.
− Pełna obsługa błędów dla wszystkich funkcji.
− Program poprawnie przetwarza polską stronę kodową (polskie znaki w tekstach).

*/


/* Prototypy */
unsigned char* read_file(const char* filename, size_t* out_size);
int write_file(const char* filename, const unsigned char* data, size_t size);
int* parse_key_line(char* line, size_t* out_keylen);
unsigned char* transform(const unsigned char* input, size_t size, const int* key, size_t keylen, int encrypt);

int main() {
    setlocale(LC_ALL, "");

    int encrypt = -1;
    const char* infile = NULL;
    const char* outfile = NULL;
    char infile_buf[MAX_STRING_INPUT_LENGTH + 1];
    char outfile_buf[MAX_STRING_INPUT_LENGTH + 1];

    char modebuf[16];
    while (1) {
        printf("Wybierz tryb: e = szyfruj, d = deszyfruj\n> ");
		safeReadString(modebuf, 16);

        if (strcmp(modebuf, "e") == 0) { encrypt = 1; break; }
        if (strcmp(modebuf, "d") == 0) { encrypt = 0; break; }

        printf("Niepoprawny tryb. Podaj 'e' lub 'd'.\n");
    }

    printf("Podaj nazwę pliku wejściowego:\n> ");
    safeReadString(infile_buf, MAX_STRING_INPUT_LENGTH);
	infile = infile_buf;

    printf("Podaj nazwę pliku wyjściowego:\n> ");
    while (1) {
        safeReadString(outfile_buf, MAX_STRING_INPUT_LENGTH);
        size_t len = strlen(outfile_buf);

        if (strpbrk(outfile_buf, ":*?\"<>|")) {
            printf("Nazwa pliku zawiera niedozwolone znaki\nSpróbuj ponownie: ");
            continue;
        }

        if (outfile_buf[0] == ' ' || outfile_buf[0] == '.') {
            printf("Nazwa pliku nie może zaczynać się od spacji ani kropki\nSpróbuj ponownie: ");
            continue;
        }

        if (outfile_buf[len - 5] == ' ' || outfile_buf[len - 5] == '.') {
            printf("Nazwa pliku nie może kończyć się spacją ani kropką\nSpróbuj ponownie: ");
            continue;
        }

        if (isReservedWindowsName(outfile_buf)) {
            printf("Niedozwolona nazwa pliku w Windows: '%s'\nPodaj inną nazwę: ", outfile_buf);
            continue;
        }

        break;
    }
    outfile = outfile_buf;

    if (encrypt == -1 || !infile || !outfile) {
		reportError("Błąd parametrów programu");
    }

    size_t in_size = 0;
    unsigned char* in_data = read_file(infile, &in_size);
    if (!in_data) return EXIT_FAILURE;

    printf("Podaj klucz — ciąg liczb całkowitych oddzielonych spacjami:\n ");
    char keybuf[MAX_STRING_INPUT_LENGTH + 2];
    safeReadString(keybuf, MAX_STRING_INPUT_LENGTH);

    size_t keylen = 0;
    int* key = parse_key_line(keybuf, &keylen);

    if (!key || keylen == 0) {
        fprintf(stderr, "Niepoprawny klucz\n");
        free(in_data);
        free(key);
        return EXIT_FAILURE;
    }

    unsigned char* out_data = transform(in_data, in_size, key, keylen, encrypt);
    if (!out_data) {
        fprintf(stderr, "Błąd podczas transformacji danych\n");
        free(in_data);
        free(key);
        return EXIT_FAILURE;
    }

    if (write_file(outfile, out_data, in_size) != 0) {
        fprintf(stderr, "Błąd zapisu do pliku '%s'\n", outfile);
        free(in_data);
        free(out_data);
        free(key);
        return EXIT_FAILURE;
    }

    printf("\nWynik %s:\n", encrypt ? "szyfrowania" : "odszyfrowywania");

    if (fwrite(out_data, 1, in_size, stdout) != in_size) {
        fprintf(stderr, "\n Nie udało się całkowicie wypisać wyniku\n");
    }
    printf("\n\nZapisano do pliku: %s\n", outfile);

    free(in_data);
    free(out_data);
    free(key);
    return EXIT_SUCCESS;
}

unsigned char* read_file(const char* filename, size_t* out_size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
		reportError("Nie można otworzyć pliku do odczytu");
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "Błąd fseek\n", strerror(errno));
        fclose(f);
        return NULL;
    }

    long sz = ftell(f);
    if (sz < 0) {
        fprintf(stderr, "Błąd ftell: %s\n", strerror(errno));
        fclose(f);
        return NULL;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Błąd fseek: %s\n", strerror(errno));
        fclose(f);
        return NULL;
    }

    unsigned char* buf = (unsigned char*)malloc((size_t)sz + 1);
    if (!buf) {
        fprintf(stderr, "Błąd alokacji pamięci\n", sz);
        fclose(f);
        return NULL;
    }

    size_t read = fread(buf, 1, (size_t)sz, f);
    if (read != (size_t)sz) {
        fprintf(stderr, "Błąd odczytu pliku (odczytano %zu z %ld)\n", read, sz);
        free(buf);
        fclose(f);
        return NULL;
    }

    buf[read] = '\0';
    fclose(f);
    *out_size = read;
    return buf;
}

int write_file(const char* filename, const unsigned char* data, size_t size) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Nie można otworzyć pliku '%s' do zapisu: %s\n", filename, strerror(errno));
        return -1;
    }

    size_t written = fwrite(data, 1, size, f);
    if (written != size) {
        fprintf(stderr, "Błąd zapisu\n");
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

int* parse_key_line(char* line, size_t* out_keylen) {
	size_t capacity = 8;    // na początek alokuje pamięć dla 8 liczb

    int* key = (int*)malloc(capacity * sizeof(int));
    if (!key) {
        fprintf(stderr, "Błąd alokacji pamięci klucza\n");
        return NULL;
    }

    size_t keycount = 0;

    char* token = strtok(line, " \t\r\n");
    while (token != NULL) {
        char* endptr;
        errno = 0;

        long val = strtol(token, &endptr, 10);
        if (endptr == token || errno != 0) {
            fprintf(stderr, "Niepoprawna liczba w kluczu: '%s'\n", token);
            free(key);
            return NULL;
        }

        if (keycount >= capacity) {
            capacity *= 2;

            int* tmp = (int*)realloc(key, capacity * sizeof(int));
            if (!tmp) {
                free(key);
                fprintf(stderr, "Błąd realokacji pamięci klucza\n");
                return NULL; 
            }
            key = tmp;
        }
        key[keycount++] = (int)val;
        token = strtok(NULL, " \t\r\n");
    }
    *out_keylen = keycount;
    return key;
}

unsigned char* transform(const unsigned char* input, size_t size, const int* key, size_t keylen, int encrypt) {
    if (!input || size == 0 || !key || keylen == 0) return NULL;

    const int MINC = 32;
    const int MAXC = 255;
    const int K = MAXC - MINC + 1;

    unsigned char* out = (unsigned char*)malloc(size);
    if (!out) { 
        fprintf(stderr, "Błąd alokacji pamięci dla buforu wyjścia\n"); 
        return NULL; 
    }

    size_t keypos = 0;
    for (size_t i = 0; i < size; ++i) {
        unsigned char c = input[i];

        if (c < (unsigned char)MINC) {
            out[i] = c;
        }
        else {
            int idx = (int)c - MINC;

            int shift = key[keypos % keylen];

            if (!encrypt) shift = -shift;

            int sum = idx + shift;
            int mod = sum % K;
            if (mod < 0) mod += K;
            int newidx = mod;
            out[i] = (unsigned char)(newidx + MINC);

            keypos++;
        }
    }
    return out;
}

