#include <stdio.h>

/*
Projekt nr 4.

Zaimplementuj obsługę struktury danych „drzewo binarne”.

Napisz funkcje realizujące następujące funkcjonalności:
- inicjowanie drzewa,
- dodawanie nowego elementu w określonym miejscu drzewa,
- pobieranie (i usunięcie) konkretnego elementu z drzewa,
- wyszukanie podanego elementu,
- wypisanie wszystkich elementów drzewa,
- usunięcie całej struktury (wszystkich elementów) – zwolnienie pamięci;
- zakończenie programu - usunięcie wszystkich elementów;
- zapis struktury do pliku binarnego,
- odczyt struktury z pliku binarnego,

Łączenie elementów w drzewo powinno odbywać się przez wskaźniki.
*/

struct Node {
	int value;
	Node* left;
	Node* right;
};

struct BinaryTree {
	// Możesz dodać wskaźnik do korzenia drzewa lub inne pola tutaj
};

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++){
		printf("%s\n", argv[i]);
	}
	return 0;
}

