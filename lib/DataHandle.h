#ifndef DATAHANDLE_H_
#define DATAHANDLE_H_

#include <time.h>
#include <string.h>
#include <windows.h>

#include "FileHandle.h"
#include "defVar.h"

static const Borrower nBorr = {}; 

//menambah peminjam ke struct yang ada
void addBorrower(Borrower * bData, Borrower *member){
    int i;
    for(i = 0; i < 50; i++){
        if(bData[i].memberId == 0 && strlen(bData[i].name) == 0){
            bData[i].memberId = member->memberId;
            bData[i].bookId = member->bookId;
            bData[i].retTime = member->retTime;
            strcpy(bData[i].name, member->name);
            return;
        }
    }
}

//ketika ada peminjaman
int borrowBook(Borrower * borrData, Borrower * member){
    int buffer;
	time_t rawtime = time(NULL);
    rawtime += 432000; //waktu peminjaman (5 Hari)

    member->retTime = rawtime;
    addBorrower(borrData, member);
}

//mencari lokasi pada borrower dengan id
int getBorrowerById(Borrower * bData, int id){
    int i;
    for(i = 0; i < 50; i++){
        if(bData[i].memberId == id){
            return i;
        }
    }
    printf("Tidak Ditemukan!\n");
    Sleep(1);
    return -1;
}

//mencetak data peminjam dan buku yang dipinjam
int borrowerOutput(Borrower *bData, int index){
    int i;
    time_t now = time(NULL);
    printf("\nID : %d\n Name : %s\n", bData[index].memberId, bData[index].name);
    printf("Sisa Waktu Peminjaman : %d hari\n", (now - bData->retTime) / (24 * 3600 * 4000));
}

void allBorrow(Borrower *bData){
    for(size_t i = 0; i < 50; i++){
        borrowerOutput((bData), i);
    }
}

int deleteMember(Borrower *borr, int index){
	borr[index] = nBorr;
}

int getbTotal(Borrower *borr){
	int i, total = 0;
	for(i = 0; i < 50; i++){
		if(borr[i].memberId > 1) total++; 
	}
	return 0;
}

#endif
