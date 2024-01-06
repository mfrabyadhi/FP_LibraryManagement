#ifndef CONSOLEHANDLE_H_
#define CONSOLEHANDLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#include "mdatabase.h"
#include "defVar.h"

int borrowMenu(Table *table, Borrower *bData);

char* getline() {
    char* line = NULL;
    DWORD charsRead;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdin == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    INPUT_RECORD inputRecord;
    DWORD eventType;

    while (ReadConsoleInput(hStdin, &inputRecord, 1, &eventType)) {
        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
            if (inputRecord.Event.KeyEvent.wVirtualKeyCode == VK_RETURN) {
                break;
            }
            else {
                char c = inputRecord.Event.KeyEvent.uChar.AsciiChar;
                if (c == 0x08) {  // Backspace key
                    if (line != NULL && strlen(line) > 0) {
                        printf("\b \b");
                        line[strlen(line) - 1] = '\0';
                    }
                }
                else if (isprint(c)) {
                    size_t len = line ? strlen(line) : 0;
                    char* temp = realloc(line, len + 2);
                    if (!temp) {
                        free(line);
                        return NULL;
                    }
                    line = temp;
                    line[len] = c;
                    line[len + 1] = '\0';
                    printf("%c", c);
                }
            }
        }
    }

    printf("\n");
    return line;
}

//Menu Utama dari Program
int mainMenu(){
    int iBuffer;
    printf("#######################################\n");
    printf("Selamat Datang Di Perpustakaan Digital!\n");
    printf("#######################################\n");
    printf("\nPilihan Menu\n");
    printf("1.List Buku.\n2.Hapus Buku.\n");
    printf("3.Request Buku.\n4.Cek Waktu Peminjaman.\n");
    printf("5.Tambahkan Stok Buku\n6.Keluar.\n>");
    scanf("%d", &iBuffer);

    system("cls");

    return iBuffer;
}

void bookOut(Table *table, int lt, int ut){
    execute_select(table);
}

///Menu List
int list(Borrower *bData, Table *table){
    int i = 0, iTemp = 0;
    char *sBuffer;
    while(iTemp != 3){
        printf("------Berikut adalah buku-buku yang tersedia-------\n");
        bookOut(table, 0, 6);
        printf("##########\n1. Sort by Id.\n");
        printf("2. Sort by Name\n");
        printf("3. Pinjam Buku.\n4. Keluar.\n>");
        sBuffer = getline();
        iTemp = atoi(sBuffer);

        switch (iTemp)
        {
        case 1:
            execute_sort(table, new_row(1, "", "", 0, 0));
            break;
        
        case 2:
            execute_sort(table, new_row(-1, "a", "", 0, 0));
            break;
        
        case 3:
        	system("cls");
            borrowMenu(table, bData);
            return 0;

        case 4:
            system("cls");
            return 0;

        default:
            printf("Invalid Input!\n");
            Sleep(1);
            break;
        }
        system("cls");
    }
    system("cls");
    fflush(stdin);
}

//menu untuk meminjam buku
int borrowMenu(Table *table, Borrower *bData){
    Borrower member;
    int iBuffer, index;
    char sBuffer[50], sBuffer_1[50];
    bookOut(table, 0, 6);
	printf ("1. Masukan judul buku.\n");
	printf("2. Exit.\n>");
    fflush(stdin);
	scanf ("%d", &iBuffer);

    if(iBuffer == 1){
        fgetc(stdin);
        printf("\nMasukan Judul Buku :");
        fgets(sBuffer, 50, stdin);
        fflush(stdin);
        if ((strlen(sBuffer) > 0) && (sBuffer[strlen (sBuffer) - 1] == '\n'))
            sBuffer[strlen (sBuffer) - 1] = '\0';
        Row *row = execute_get(table, new_row(-1,sBuffer, "", 0, 0));
        if(row == NULL){
            printf("Buku Tidak Ditemukan.\n");
            Sleep(1);
            return -1;
        }
        if(row->stock == 0){
            printf("Tidak Ada Stock Yang Tersedia!\n");
            Sleep(1);
            return 1;
        }
        row->stock--;
        execute_edit(table, row);
        member.bookId = row->id;

		fflush(stdin);
        printf("Masukan Data Diri!\nNama : ");
        fgets(sBuffer_1, 50, stdin);
        fflush(stdin);
        if ((strlen(sBuffer_1) > 0) && (sBuffer_1[strlen (sBuffer_1) - 1] == '\n'))
            sBuffer_1[strlen (sBuffer_1) - 1] = '\0';
        strcpy(member.name, sBuffer_1);

        printf("ID : ");
        scanf("%d", &member.memberId);
        
        borrowBook(bData, &member);
        printf("Tekan Tombol Untuk Melanjutkan!");
        fflush(stdin);
	    fgets(sBuffer, 1, stdin);
	    fgets(sBuffer, 1, stdin);
        return 1;
    }
	
	//system ("cls");
	return -1;
}

//menu untuk melihat data peminjam
int borrowerMenu(Table *table, Borrower *bData){
    char sBuffer[50];
    int iBuffer, index;
    printf("MENU VISITOR");
    printf("Masukan Id :");
    fgets(sBuffer, 50, stdin);
    iBuffer = atoi(sBuffer);

    if(iBuffer){
        index = getBorrowerById(bData, iBuffer);
    }
    else
    	return -1;

    if(index != -1){
        borrowerOutput(bData, index);
        bookOut(table, bData->bookId, bData->bookId);
    	printf("Kembalikan Buku (1 = Ya)?");
    	scanf("%d", &iBuffer);
	}
	if(iBuffer == 1) {
        Row *row = execute_get(table, new_row(bData->bookId, "", "", 0,0));
        row->stock++;
        execute_edit(table, row);
		deleteMember(bData, index);
	}

}

//menu untuk menambahkan stok buku
int amountMenu(Table *table){
    char *sBuffer;
    int index, amount;
    bookOut(table, 0, table->num_rows);
    printf("Menambah Stock Buku\n");
    printf("Masukan Nama Buku :");

    sBuffer = getline();
    Row *row = execute_get(table, new_row(-1, sBuffer, "", 0, 0));

    printf("");
    if(row == NULL){
        printf("Tidak Ditemukan!\n");
        Sleep(10000);
        return -1;
    };
    
    printf("Masukan Jumlah Buku :");
    scanf("%d", &amount);
    row->stock += amount;
    execute_edit(table, row);
    system("cls");
}

void deleteMenu(Table *table){
    char *sbuffer, *ep;
    bookOut(table, 0, table->num_rows);
    printf("Masukan Id :");
    sbuffer = getline();
    long int buff = strtol(sbuffer, &ep, 10);
    if(sbuffer == ep){
        printf("Invalid Input\n");
        return;
    }

    execute_delete(table, execute_get(table, new_row(buff, "", "", 0, 0)));
}

void addBook(Table *table){
    char *sbuffer, *ep;
    Row *row = empty_row();
    row->id = table->num_rows;
    printf("MASUKAN DATA\n");
    printf("Name :");
    sbuffer = getline();
    if(strtol(sbuffer, &ep, 10) == -1)
        return;
    strcpy(row->name, sbuffer);
    printf("Author :");
    sbuffer = getline();
    strcpy(row->author, sbuffer);
    printf("Year :");
    sbuffer = getline();
    long int buff = strtol(sbuffer, &ep, 10);
    row->year = (uint32_t)buff;
    printf("Stock :");
    sbuffer = getline();
    buff = strtol(sbuffer, &ep, 10);
    row->stock = (uint32_t)buff;
    // print_row(row);
    execute_insert(row, table);
}

#endif
