#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/*
 Program szyfruj¹cy/deszyfruj¹cy oparty na kluczu liczbowym.
 Zasada: dla ka¿dego bajtu (wartoœci 32..255) przesuwamy jego pozycjê
 w cyklicznym zbiorze K = 224 (32..255).

 U¿ycie:
  TextCipher.exe e input.txt output.txt "3,1,4"
  TextCipher.exe d input.txt output.txt key.txt

 - tryb: 'e' szyfruj, 'd' deszyfruj
 - input: plik wejœciowy (wieloliniowy)
 - output: plik wyjœciowy (zapis i wyœwietlenie na ekranie)
 - key: albo œcie¿ka do pliku z liczbami klucza, albo ci¹g liczb rozdzielonych przecinkami/spacjami

 Wszystkie bufory s¹ alokowane dynamicznie. Program wykonuje podstawow¹ obs³ugê b³êdów.
*/

#define MIN_CHAR 32
#define MAX_CHAR 255
#define K_COUNT (MAX_CHAR - MIN_CHAR + 1)

static unsigned char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    size_t cap = 4096;
    unsigned char *buf = malloc(cap);
    if (!buf) { fclose(f); return NULL; }
    size_t len = 0;
    while (1) {
        if (len + 1024 > cap) {
            cap *= 2;
            unsigned char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); fclose(f); return NULL; }
            buf = tmp;
        }
        size_t r = fread(buf + len, 1, 1024, f);
        len += r;
        if (r < 1024) break;
    }
    fclose(f);
    *out_len = len;
    return buf;
}

static int write_file(const char *path, const unsigned char *buf, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    size_t w = fwrite(buf, 1, len, f);
    fclose(f);
    return (w == len) ? 0 : -2;
}

static int *parse_key_from_string(const char *s, size_t *out_count) {
    // Kopiujemy na bezpieczny bufor, zamieniamy separatory na spacje, u¿ywamy strtok_s
    size_t slen = strlen(s);
    char *tmp = malloc(slen + 1);
    if (!tmp) return NULL;
    memcpy(tmp, s, slen + 1);
    for (char *p = tmp; *p; ++p) if (*p == ',' || *p == ';' || *p == '\t' || *p == '\n') *p = ' ';
    size_t cap = 16;
    int *key = malloc(cap * sizeof(int));
    if (!key) { free(tmp); return NULL; }
    size_t count = 0;
    char *context = NULL;
    char *tok = NULL;
#ifdef _MSC_VER
    tok = strtok_s(tmp, " ", &context);
#else
    tok = strtok(tmp, " ");
#endif
    while (tok) {
        if (*tok != '\0') {
            char *endptr;
            long v = strtol(tok, &endptr, 10);
            if (endptr == tok) { // nieprawid³owa liczba
                free(tmp); free(key); return NULL;
            }
            if (count + 1 > cap) {
                cap *= 2;
                int *t2 = realloc(key, cap * sizeof(int));
                if (!t2) { free(tmp); free(key); return NULL; }
                key = t2;
            }
            key[count++] = (int)v;
        }
#ifdef _MSC_VER
        tok = strtok_s(NULL, " ", &context);
#else
        tok = strtok(NULL, " ");
#endif
    }
    free(tmp);
    if (count == 0) { free(key); return NULL; }
    *out_count = count;
    return key;
}

static int *read_key_file(const char *path, size_t *out_count) {
    size_t len;
    unsigned char *data = read_file(path, &len);
    if (!data) return NULL;
    // treat file as text, null-terminate
    char *s = malloc(len + 1);
    if (!s) { free(data); return NULL; }
    memcpy(s, data, len);
    s[len] = '\0';
    free(data);
    int *key = parse_key_from_string(s, out_count);
    free(s);
    return key;
}

static void process_buffer(unsigned char *buf, size_t len, const int *key, size_t keylen, int encrypt) {
    if (keylen == 0) return;
    size_t k = keylen;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = buf[i];
        if (c >= MIN_CHAR) {
            unsigned int idx = c - MIN_CHAR; // 0..K_COUNT-1
            int shift = key[i % k];
            // normalize shift to positive within [0, K_COUNT-1]
            int s = shift % (int)K_COUNT;
            if (s < 0) s += K_COUNT;
            unsigned int newidx;
            if (encrypt) {
                newidx = (idx + (unsigned int)s) % K_COUNT;
            } else {
                newidx = (idx + K_COUNT - (unsigned int)s) % K_COUNT;
            }
            buf[i] = (unsigned char)(newidx + MIN_CHAR);
        }
    }
}

static void print_usage(const char *prog) {
    fprintf(stderr, "U¿ycie: %s [e|d] input.txt output.txt key_or_keyfile\n", prog);
    fprintf(stderr, "  e - szyfruj, d - deszyfruj\n");
    fprintf(stderr, "  key: np. \"3,1,4\" lub œcie¿ka do pliku z liczbami\n");
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");

    if (argc < 5) {
        print_usage(argv[0]);
        return 1;
    }
    char mode = argv[1][0];
    int encrypt;
    if (mode == 'e' || mode == 'E') encrypt = 1;
    else if (mode == 'd' || mode == 'D') encrypt = 0;
    else { print_usage(argv[0]); return 1; }

    const char *input_path = argv[2];
    const char *output_path = argv[3];
    const char *key_arg = argv[4];

    size_t in_len;
    unsigned char *in_buf = read_file(input_path, &in_len);
    if (!in_buf) {
        fprintf(stderr, "B³¹d: nie mo¿na otworzyæ pliku wejœciowego '%s'\n", input_path);
        return 2;
    }

    size_t keylen = 0;
    int *key = NULL;
    // najpierw spróbuj otworzyæ jako plik
    FILE *kf = fopen(key_arg, "r");
    if (kf) { fclose(kf); key = read_key_file(key_arg, &keylen); }
    else key = parse_key_from_string(key_arg, &keylen);

    if (!key || keylen == 0) {
        fprintf(stderr, "B³¹d: nieprawid³owy klucz\n");
        free(in_buf);
        return 3;
    }

    // kopiujemy bufor, aby nie nadpisaæ orygina³u (chocia¿ nie jest konieczne)
    unsigned char *out_buf = malloc(in_len);
    if (!out_buf) { fprintf(stderr, "B³¹d alokacji\n"); free(in_buf); free(key); return 4; }
    memcpy(out_buf, in_buf, in_len);

    process_buffer(out_buf, in_len, key, keylen, encrypt);

    // zapis do pliku
    int wres = write_file(output_path, out_buf, in_len);
    if (wres != 0) {
        fprintf(stderr, "B³¹d zapisu pliku '%s' (kod %d)\n", output_path, wres);
        free(in_buf); free(out_buf); free(key);
        return 5;
    }

    // wyœwietlenie na monitorze (w trybie binarnym piszemy dok³adnie bajty)
    fwrite(out_buf, 1, in_len, stdout);

    free(in_buf); free(out_buf); free(key);
    return 0;
}
