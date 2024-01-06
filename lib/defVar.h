/*Header yang berisi struct yang digunakan pada program*/

#ifndef DEFVAE_H_
#define DEFVAE_H_

#include <time.h>


//struct untuk menyimpan peminjam
typedef struct{
    unsigned int memberId;
    unsigned int bookId;
    char name[50];
    time_t retTime;
}Borrower;

#endif