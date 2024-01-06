#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

#include "lib\mdatabase.h"
#include "lib\FileHandle.h"
#include "lib\DataHandle.h"
#include "lib\defVar.h"
#include "lib\ConsoleHandle.h"

const char bookFile[] = "book.db";

int main(){
	unsigned int bTotal = 0, lbTotal = 0;
	int mode = 0, idTemp;
	
	system("color B");
	
	//membuat struct dengan tipenya masing masing
	Borrower * bData;
	
	//mengalocasi data untuk struct
	Table *table = db_open(bookFile);
	bData = (Borrower*) malloc(sizeof(Borrower) * 50);
	
	//fungsi yang dijalankan setiap setiap kali program dinyalakan
	setup(bData, &bTotal);

	//loop utama
	while(mode != -1){
		//fungsi menu utama
		mode = mainMenu();
		
		fflush(stdin);
		Sleep(5);
		
		//mendapatkan banyak data
		bTotal = getbTotal(bData);
		
		//mengupdate File
		addBorrowerToFile(bData, &bTotal);	
		
		switch (mode){
		case 1:
			if(table->num_rows == 0){
				printf("Library Masih Kosong\n");
				Sleep(1000);
				break;
			}
			
			list(bData, table);
			break;
		
		case 3:
			addBook(table);
			break;

		case 5:
			amountMenu(table);
			break;
		
		case 4:
			borrowerMenu(table, bData);
			break;
		
		case 2:
			deleteMenu(table);
			break;
			
		case 7:
			mode = -1;
			break;
		
		default:
			printf("Pilihan Salah!\n");
			break;
		}
		addBorrowerToFile(bData, &bTotal);
		
		system("cls");
		lbTotal = bTotal;
	}
	db_close(table);
	free(bData);
	return 0;
}
