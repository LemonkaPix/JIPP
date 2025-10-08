#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

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

Node* initTree() {
    return NULL;
}

Node* addNode(Node* root, int value) {
    if (root == NULL) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }
    if (value < root->value) {
        root->left = addNode(root->left, value);
    }
    else if (value > root->value) {
        root->right = addNode(root->right, value);
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
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
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
        int marker = 0;
        fwrite(&marker, sizeof(int), 1, file); 
        return;
    }
    int marker = 1;
    fwrite(&marker, sizeof(int), 1, file);
    fwrite(&root->value, sizeof(int), 1, file);
    saveTree(file, root->left);
    saveTree(file, root->right);
}

Node* loadTree(FILE* file) {
    int marker;
    if (fread(&marker, sizeof(int), 1, file) != 1)
        return NULL;
    if (marker == 0)
        return NULL;
    int value;
    if (fread(&value, sizeof(int), 1, file) != 1)
        return NULL;
    Node* node = (Node*)malloc(sizeof(Node));
    node->value = value;
    node->left = loadTree(file);
    node->right = loadTree(file);
    return node;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "pl_PL");

    Node* tree = initTree();
    tree = addNode(tree, 8);
    tree = addNode(tree, 3);
    tree = addNode(tree, 10);
    tree = addNode(tree, 1);
    tree = addNode(tree, 6);
    tree = addNode(tree, 14);
    tree = addNode(tree, 4);
    tree = addNode(tree, 7);
    tree = addNode(tree, 13);

    printTree(tree);

	Node* found = findNode(tree, 3);
    if(found != NULL)
	    printf("\nznalezionoąąą: %d\n", found->value);
    else
		printf("nie znaleziono\n");

    tree = removeNode(tree, 3);

    found = findNode(tree, 3);
    if (found != NULL)
        printf("znaleziono: %d\n", found->value);
    else
        printf("nie znaleziono\n");

    printTree(tree);
    printf("\n");

    FILE* file = fopen("tree.bin", "wb");
    if (file != NULL) {
        saveTree(file, tree);
        fclose(file);
    }
    else
		printf("Nie można otworzyć pliku do zapisu.\n");

    freeTree(tree);

    if (fopen_s(&file, "tree.bin", "rb") != 0) {
        file = NULL;
    }

    file = fopen("tree.bin", "rb");
    if (file != NULL) {
        Node* loadedTree = loadTree(file);
        fclose(file);
        printf("Załadowane drzewo\n");
        printTree(loadedTree);
        printf("\n");
        freeTree(loadedTree);
    }

    return 0;
}

