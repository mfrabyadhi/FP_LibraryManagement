//HEADER FOR COMMAND PROMPT UI RELATED THINGS

// #include "uihandle.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

const uint32_t WIDTH = 50;

typedef enum{
	__MAINMENU,
	__BORROW,
	__RETURN,
	__ADD,
	__DELETE,
	__UPDATE,
	__EXIT
} MENU;

char mainmenu[] = "\
=============================================\n\
|               MAIN MENU                   |\n\
=============================================\n\
| 1. Peminjaman Buku                        |\n\
| 2. Pengembalian Buku                      |\n\
| 3. Tambah Buku                            |\n\
| 4. Hapus Buku                             |\n\
| 5. Update Buku                            |\n\
| 6. Keluar                                 |\n\
=============================================\n\
| Pilihan:								    |\n\
=============================================\n";

char borrow[] = "\
=============================================\n\
|               PEMINJAMAN BUKU             |\n\
=============================================\n\
| 1. Cari Buku                              |\n\
| 2. list Buku                              |\n\
| 3. Kembali                                |\n\
=============================================\n\
| Pilihan:								    |\n\
=============================================\n";

char return_buff[] = "\
=============================================\n\
|               PENGEMBALIAN BUKU           |\n\
=============================================\n\
| 1. Cari Peminjam                          |\n\
| 2. List Peminjam                          |\n\
| 3. Kembali                                |\n\
=============================================\n\
| Pilihan:								    |\n\
=============================================\n";



void gotoxy(int x, int y){
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

typedef struct pos_t{
	int x;
	int y;
} Pos;

typedef struct menu_t{
	MENU type;
	char* output_buffer;
	Pos* cursor;
	void (*function)(void);
} Menu;

Pos *get_pos_from_buffer(char* buffer){
	Pos *pos = (Pos*) malloc(sizeof(Pos));
	int i = 0;
	while(buffer[i] != ':'){
		i++;
	}
	pos->x = i % WIDTH + 1;
	pos->y = i / WIDTH + 2;
	return pos;
}

void new_menu(Menu* menu, MENU type, char* output_buffer, Pos* cursor, void (*function)(void)){
	menu->type = type;
	menu->output_buffer = output_buffer;
	menu->cursor = cursor;
	menu->function = function;
}

void print_menu(Menu* menu){
	printf("%s", menu->output_buffer);
	gotoxy(menu->cursor->x, menu->cursor->y);
	scanf("%d", &menu->type);
}

int main(){
	printf("x: %d, y: %d\n", get_pos_from_buffer(mainmenu)->x, get_pos_from_buffer(mainmenu)->y);

	Menu menu = {__MAINMENU, mainmenu, get_pos_from_buffer(mainmenu), NULL};

	print_menu(&menu);

	return 0;
}
