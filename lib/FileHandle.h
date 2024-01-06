#ifndef _FileHandle_H_
#define _FileHandle_H_

#include <stdio.h>
#include <windows.h>
#include <string.h>

#include "defVar.h"

//Untuk Menyiapkan Program Setiap Kali program dijalankan
int setup(Borrower* bData, int* bTotal) {
	FILE* fmptr;
	int buffer;

	fmptr = fopen("MemberDatabase.bin", "rb");

	if (fmptr == NULL) {
		//Membuat file untuk menyimpan data peminjam
		fmptr = fopen("MemberDatabase.bin", "wb");
		fclose(fmptr);
	}
	else {
		//jika file sudah ada akan membaca value pada file
		if (fread(bTotal, sizeof(int), 1, fmptr) != 1) {
			fclose(fmptr);
			return 0;
		}
		fread(bData, sizeof(Borrower), *bTotal, fmptr);
		fclose(fmptr);
	}

	return 1;
}

int borrowerExist(char* name) {
	Borrower* bData;
	FILE* fptr;
	int i;

	fptr = fopen("MemberDatabase.bin", "rb");

	if (fptr == NULL) {
		printf("Error!\n");
		exit(1);
	}
	fseek(fptr, 0, SEEK_SET);
	while (!feof(fptr)) {
		fread(bData, sizeof(Borrower), 1, fptr);
		if (strcmp(bData->name, name) == 0)
			return 1;
	}
	fclose(fptr);
	return 0;
}

int addBorrowerToFile(Borrower* bData, int* total) {
	FILE* fptr;
	int i, idBuffer;
	fptr = fopen("MemberDatabase.bin", "wb");
	if (fptr == NULL) {
		printf("Error!\n");
		exit(1);
	}
	//menyimpan jumlah data
	fwrite(total, sizeof(int), 1, fptr);

	//menyimpan isi data dari struct borrower
	for (i = 0; i < *total; i++)
		fwrite(&bData[i], sizeof(Borrower), 1, fptr);
	fclose(fptr);
	return 1;
}

// Borrower *getBorrowerById(int id) {
//     FILE* fptr;
//     Borrower *borrower;
//     fptr = fopen("MemberDatabase.bin", "rb");

//     if (fptr == NULL) {
//         printf("Error!\n");
//         exit(1);
//     }

//     // Read the total number of borrowers
//     int total;
//     fread(&total, sizeof(int), 1, fptr);

//     // Search for the borrower with the given ID
//     int found = 0;
//     for (int i = 0; i < total; i++) {
//         fread(borrower, sizeof(Borrower), 1, fptr);
//         if (borrower->memberId == id) {
//             found = 1;
//             break;
//         }
//     }

//     fclose(fptr);

//     if (!found) {
//         // Handle the case when the borrower is not found
//         printf("Borrower with ID %d not found!\n", id);
//         exit(1);
//     }

//     return borrower;
// }


#endif
