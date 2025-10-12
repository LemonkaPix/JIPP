#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>

#define MAX_STRING_INPUT_LENGTH 100 //Windows ma ograniczenie do 255 znaków

/*
Projekt nr 4.

Zaimplementuj obsługę struktury danych „drzewo binarne”.

Napisz funkcje realizujące następujące funkcjonalności:
- inicjowanie drzewa,
- dodawanie nowego elementu w określonym miejscu drzewa,
- pobieranie (i usunięcie) konkretnego elementu z drzewa,
? pobranie w sensie że element zostaje usunięty z drzewa i zwrócony do programu, czy poprostu zostaje usunięty?
- wyszukanie podanego elementu,
? wyszukanie w sensie że element zostaje zwrócony do programu, czy poprostu sprawdzamy czy istnieje?
- wypisanie wszystkich elementów drzewa,
- usunięcie całej struktury (wszystkich elementów) – zwolnienie pamięci;
- zakończenie programu - usunięcie wszystkich elementów;
- zapis struktury do pliku binarnego,
- odczyt struktury z pliku binarnego,

Łączenie elementów w drzewo powinno odbywać się przez wskaźniki.
*/

typedef struct Node {
    int value;
    struct Node* left;
    struct Node* right;
} Node;

int isReservedWindowsName(const char* name) {
    const char* reserved[] = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    size_t len = 0;
    while (name[len] && name[len] != '.' && len < 31) ++len;

    for (int i = 0; i < sizeof(reserved) / sizeof(reserved[0]); ++i) {
        if (strlen(reserved[i]) == len && strnicmp(name, reserved[i], len) == 0)
            return 1;
    }
    return 0;
}

void reportError(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

Node* initTree() {
    return NULL;
}

Node* addNode(Node* root, int value) {
    if (root == NULL) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        if (!newNode) {
            fprintf(stderr, "nie można zalokować pamięci dla nowego węzła\n");
            exit(EXIT_FAILURE);
        }
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        printf("Dodano węzeł o wartości %d\n", value);
        return newNode;
    }
    if (value < root->value) {
        root->left = addNode(root->left, value);
    }
    else if (value > root->value) {
        root->right = addNode(root->right, value);
    }
    else {
		printf("Wartość %d już istnieje w drzewie. Duplikaty nie są dozwolone.\n", value);
    }

    return root;
}

Node* removeNode(Node* root, int value) {
    if (root == NULL) return NULL;
    if (value < root->value) {
        root->left = removeNode(root->left, value);
    } else if (value > root->value) {
        root->right = removeNode(root->right, value);
    } else {
        if (root->left == NULL) {
            Node* temp = root->right;
			printf("Usunięto węzeł o wartości %d\n", root->value);
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            printf("Usunięto węzeł o wartości %d\n", root->value);
            free(root);
            return temp;
        } else {
            Node* succParent = root;
            Node* succ = root->right;
            while (succ->left != NULL) {
                succParent = succ;
                succ = succ->left;
            }
            root->value = succ->value;
            if (succParent != root)
                succParent->left = removeNode(succParent->left, succ->value);
            else
                succParent->right = removeNode(succParent->right, succ->value);
        }
    }
    return root;
}

Node* findNode(Node* root, int value) {
    if (root == NULL) return NULL;
    if (value == root->value) return root;
    if (value < root->value)
        return findNode(root->left, value);
    else
        return findNode(root->right, value);
}

void printTree(Node* root) {
    if (root == NULL) return;
    printTree(root->left);
    printf("%d ", root->value);
    printTree(root->right);
}

void freeTree(Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void saveTree(FILE* file, Node* root) {
    if (root == NULL) {
        bool marker = false;
        if (fwrite(&marker, sizeof(bool), 1, file) != 1) {
            reportError("Błąd zapisu markera do pliku");
        }
        return;
    }
    bool marker = true;
    if (fwrite(&marker, sizeof(bool), 1, file) != 1) {
		reportError("Błąd zapisu markera do pliku");
    }
    if (fwrite(&root->value, sizeof(int), 1, file) != 1) {
		reportError("Błąd zapisu wartości do pliku");
    }
    saveTree(file, root->left);
    saveTree(file, root->right);
}

Node* loadTree(FILE* file) {
    bool marker;
    if (fread(&marker, sizeof(bool), 1, file) != 1) {
        if (feof(file)) return NULL;
        reportError("Błąd odczytu markera z pliku");
    }
    if (marker == false)
        return NULL;
    int value;
    if (fread(&value, sizeof(int), 1, file) != 1) {
        reportError("Błąd odczytu wartośći z pliku");
    }
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
		reportError("Błąd alokacji pamięci podczas odczytu drzewa");
    }
    node->value = value;
    node->left = loadTree(file);
    node->right = loadTree(file);
    return node;
}

int safeReadInt() {
    char buf[100];
    long val;
    char* endptr;

    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) {
            printf("Błąd odczytu\n");
            continue;
        }
        val = strtol(buf, &endptr, 10);
        if (endptr == buf || (*endptr != '\n' && *endptr != '\0')) {
            printf("Niepoprawny format liczby\nSpróbuj ponownie: ");
            continue;
        }
        return (int)val;
    }
}

void safeReadString(char* buffer) {
    while (1) {
        if (!fgets(buffer, MAX_STRING_INPUT_LENGTH + 2, stdin)) {
            printf("Błąd odczytu napisu\n");
            exit(1);
        }

		int tempLen = (int)strlen(buffer);

        if (strchr(buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Przekroczono maksymalną długość napisu %zu znaków\nSpróbuj ponownie: ", MAX_STRING_INPUT_LENGTH);
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        break;
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "pl_PL");

    Node* tree = initTree();


    while (true) {
        printf("Wybierz akcję:\n");
		printf("1. Dodaj węzeł\n");
		printf("2. Usuń węzeł\n");
		printf("3. Znajdź węzeł\n");
		printf("4. Wypisz drzewo\n");
		printf("5. Usuń drzewo\n");
		printf("6. Zapisz drzewo do pliku\n");
		printf("7. Wczytaj drzewo z pliku\n");
		printf("8. Zakończ program\n");

		int choice = safeReadInt();

        system("cls");


        switch (choice)
        {
            case 1: {
                printf("Podaj wartość do dodania: ");
                int value = safeReadInt();
                tree = addNode(tree, value);
                break;
            }
            case 2: {
                printf("Podaj wartość do usunięcia: ");
                int value = safeReadInt();
                tree = removeNode(tree, value);
                break;
            }
            case 3: {
                printf("Podaj wartość do znalezienia: ");
                int value = safeReadInt();
                Node* found = findNode(tree, value);
                if (found) {
                    printf("Znaleziono węzeł o wartości %d\n", found->value);
                }
                else {
                    printf("Nie znaleziono węzła o wartości %d\n", value);
                }
                break;
            }
            case 4: {
                printf("Elementy drzewa w kolejności rosnącej: ");
                printTree(tree);
                printf("\n");
                break;
            }
            case 5: {
                freeTree(tree);
                tree = NULL;
                printf("Usunięto całe drzewo.\n");
                break;
		    }
            case 6: {
                printf("Podaj nazwę pliku do zapisu (zakończoną .bin): ");
                char filename[MAX_STRING_INPUT_LENGTH];
                while (true)
                {
                    safeReadString(filename);
                    size_t len = strlen(filename);
                    const char* ext = ".bin";

                    if (len < 5 || strcmp(filename + len - 4, ext) != 0) {
                        printf("Plik musi mieć rozszerzenie .bin\nSpróbuj ponownie: ");
                        continue;
                    }

                    if (strpbrk(filename, "\\/:*?\"<>|")) {
                        printf("Nazwa pliku zawiera niedozwolone znaki\nSpróbuj ponownie: ");
                        continue;
                    }

                    if (filename[0] == ' ' || filename[0] == '.') {
                        printf("Nazwa pliku nie może zaczynać się od spacji ani kropki\nSpróbuj ponownie: ");
                        continue;
                    }

                    if (filename[len - 5] == ' ' || filename[len - 5] == '.') {
                        printf("Nazwa pliku nie może kończyć się spacją ani kropką\nSpróbuj ponownie: ");
                        continue;
                    }

                    if (isReservedWindowsName(filename)) {
                        printf("Nazwa pliku jest zarezerwowana w systemie Windows\nSpróbuj ponownie: ");
                        continue;
                    }

                    break;
                }


                FILE* file = fopen(filename, "wb");
                if (!file) {
                    printf("Nie można otworzyć pliku do zapisu!\n");
                    break;
                }

                saveTree(file, tree);
                fclose(file);
                printf("Drzewo zapisane do pliku %s\n", filename);
                break;
            }
            case 7:
            {
                printf("Podaj nazwę pliku do wczytania z rozszerzeniem .bin: ");
                char filename[MAX_STRING_INPUT_LENGTH];
                while (true)
                {
                    safeReadString(filename);
                    size_t len = strlen(filename);
                    const char* ext = ".bin";
                    if (len < 5 || strcmp(filename + len - 4, ext) != 0) {
                        printf("Plik musi mieć rozszerzenie .bin\nSpróbuj ponownie: ");
                        continue;
                    }

                    if (isReservedWindowsName(filename)) {
                        printf("Plik jest zarezerwowany przez system Windows\nSpróbuj ponownie: ");
                        continue;
                    }

                    break;

                }
			    FILE* file = fopen(filename, "rb");
                if (!file) {
                    printf("Nie można otworzyć pliku do odczytu!\n");
                    break;
                }
                freeTree(tree);
                tree = loadTree(file);
                fclose(file);
                printf("Drzewo wczytane z pliku %s\n", filename);
			    break;
            }
            case 8:
            {
                freeTree(tree);
                printf("Zakończenie programu.\n");
                return 0;
		    }
            default:
            {
                printf("Niepoprawny wybór, spróbuj ponownie.\n");
            }
        }

        printf("\n");

    }

    return 0;
}

