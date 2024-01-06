#ifndef MDATABASE_H
#define MDATABASE_H

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#include <sys\stat.h>
#include "mdatabase.h"

#define TABLE_MAX_PAGES 100
#define COLUMN_NAME_SIZE 32
#define COLUMN_AUTHOR_SIZE 32

#define MEMBER_SIZE(Struct, Member) sizeof(((Struct*)0)->Member)

typedef struct Row_t {
		uint32_t id;
		char name[COLUMN_NAME_SIZE + 1];
		char author[COLUMN_AUTHOR_SIZE + 1];
		uint32_t year;
		uint32_t stock;
}Row;

const uint32_t ID_SIZE 				= MEMBER_SIZE(struct Row_t, id);
const uint32_t NAME_SIZE 			= MEMBER_SIZE(struct Row_t, name);
const uint32_t AUTHOR_SIZE 			= MEMBER_SIZE(struct Row_t, author);
const uint32_t YEAR_SIZE 			= MEMBER_SIZE(struct Row_t, year);
const uint32_t STOCK_SIZE 			= MEMBER_SIZE(struct Row_t, stock);

const uint32_t ID_OFFSET 			= 0;
const uint32_t NAME_OFFSET 			= ID_OFFSET + ID_SIZE;
const uint32_t AUTHOR_OFFSET 		= NAME_OFFSET + NAME_SIZE;
const uint32_t YEAR_OFFSET 			= AUTHOR_OFFSET + AUTHOR_SIZE;
const uint32_t STOCK_OFFSET 		= YEAR_OFFSET + YEAR_SIZE;
const uint32_t ROW_SIZE 			= ID_SIZE + NAME_SIZE + AUTHOR_SIZE + YEAR_SIZE + STOCK_SIZE;

const uint32_t PAGE_SIZE 			= 4096;
const uint32_t ROWS_PER_PAGE 		= PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS 		= ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct Pager_t {
	int file_descriptor;
	uint32_t file_length;
	void* pages[TABLE_MAX_PAGES];
	uint32_t *sort;
} Pager;

typedef struct Table_t {
	Pager* pager;
	uint32_t num_rows;
} Table;

bool execute_sort(Table*, Row);

Row *empty_row(){
	return (Row *) malloc(sizeof(Row));
}

Row* new_row(uint32_t id, char* name, char* auth, uint32_t year, uint32_t stock) {
    Row* row = (Row*)malloc(sizeof(Row));
    row->id = id;
    strncpy(row->name, name, COLUMN_NAME_SIZE);
	strncpy(row->author, auth, COLUMN_AUTHOR_SIZE);
    row->year = year;
    row->stock = stock;
    return row;
}

void print_row(Row* row) {
    printf("(%d, %s, %s, %d, %d)\n", row->id, row->name, row->author, row->year, row->stock);
}

void serialize_row(Row* source, void* destination) {
	memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + NAME_OFFSET, &(source->name), NAME_SIZE);
    memcpy(destination + AUTHOR_OFFSET, &(source->author), AUTHOR_SIZE);
    memcpy(destination + YEAR_OFFSET, &(source->year), YEAR_SIZE);
    memcpy(destination + STOCK_OFFSET, &(source->stock), STOCK_SIZE);
}

void deserialize_row(void* source, Row* destination) {
	memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->name), source + NAME_OFFSET, NAME_SIZE);
    memcpy(&(destination->author), source + AUTHOR_OFFSET, AUTHOR_SIZE);
    memcpy(&(destination->year), source + YEAR_OFFSET, YEAR_SIZE);
    memcpy(&(destination->stock), source + STOCK_OFFSET, STOCK_SIZE);
}

void swap_row(void *first, void *second){
	void *buff = malloc(ROW_SIZE);
	memcpy(buff, first, ROW_SIZE);
	memcpy(first, second, ROW_SIZE);
	memcpy(second, buff, ROW_SIZE);
	free(buff);
}

int compare_rows_name(const void* a, const void* b) {
    Row* rowA = (Row*)a;
    Row* rowB = (Row*)b;
    return strcmp(rowA->name, rowB->name);
}

int compare_rows_auth(const void* a, const void* b) {
    Row* rowA = (Row*)a;
    Row* rowB = (Row*)b;
    return strcmp(rowA->author, rowB->author);
}

void* get_page(Pager* pager, uint32_t page_num) {
	if (page_num > TABLE_MAX_PAGES) {
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
					 TABLE_MAX_PAGES);
		exit(EXIT_FAILURE);
	}

	if (pager->pages[page_num] == NULL) {
		void* page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_length / PAGE_SIZE;

		if (pager->file_length % PAGE_SIZE) {
			num_pages += 1;
		}

		if (page_num <= num_pages) {
			lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
			ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
			if (bytes_read == -1) {
				printf("Error reading file: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}

		pager->pages[page_num] = page;
	}

	return pager->pages[page_num];
}

void* row_slot(Table* table, uint32_t row_num) {
	uint32_t page_num = row_num / ROWS_PER_PAGE;
	void* page = get_page(table->pager, page_num);
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return page + byte_offset;
}

Pager* pager_open(const char* filename) {
	int fd = _open(filename, O_RDWR | O_CREAT);

	if (fd == -1) {
		printf("Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	off_t file_length = lseek(fd, 0, SEEK_END);

	Pager* pager = malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_length = file_length;

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->pages[i] = NULL;
	}

	return pager;
}

Table* db_open(const char* filename) {
	Pager* pager = pager_open(filename);
	uint32_t num_rows = pager->file_length / ROW_SIZE;
	pager->sort = (uint32_t *) malloc(sizeof(uint32_t) * num_rows);

	for(size_t i = 0; i < num_rows; i++)
		(pager->sort)[i] = i;

	Table* table = malloc(sizeof(Table));
	table->pager = pager;
	table->num_rows = num_rows;

	return table;
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
	if (pager->pages[page_num] == NULL) {
		printf("Tried to flush null page\n");
		exit(EXIT_FAILURE);
	}

	off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

	if (offset == -1) {
		printf("Error seeking: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	ssize_t bytes_written =
			write(pager->file_descriptor, pager->pages[page_num], size);

	if (bytes_written == -1) {
		printf("Error writing: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

void db_close(Table* table) {
	Pager* pager = table->pager;
	uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

	for (uint32_t i = 0; i < num_full_pages; i++) {
		if (pager->pages[i] == NULL) {
			continue;
		}
		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
	if (num_additional_rows > 0) {
		uint32_t page_num = num_full_pages;
		if (pager->pages[page_num] != NULL) {
			pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
			free(pager->pages[page_num]);
			pager->pages[page_num] = NULL;
		}
	}

	int result = close(pager->file_descriptor);
	if (result == -1) {
		printf("Error closing db file.\n");
		exit(EXIT_FAILURE);
	}
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		void* page = pager->pages[i];
		if (page) {
			free(page);
			pager->pages[i] = NULL;
		}
	}
	free(pager);
}

//
bool execute_insert(Row *row, Table* table) {
	if (table->num_rows >= TABLE_MAX_ROWS) {
		return false;
	}

	serialize_row(row, row_slot(table, table->num_rows));
	table->num_rows += 1;
	table->pager->sort = (uint32_t *) realloc(table->pager->sort, table->num_rows * sizeof(uint32_t));

	execute_sort(table, new_row(-1, "","",0,0));
	
	free(row);
	return true;
}

bool execute_sort(Table *table, Row row){
	Row *rows = (Row *)malloc(sizeof(Row) * table->num_rows);
	for(size_t i = 0; i < table->num_rows; i++)
		deserialize_row(row_slot(table, i), &(rows[i]));
		
	if(!(strcmp(row.name, "")))
		qsort(rows, table->num_rows, sizeof(Row), compare_rows_name);
		
	else if(!(strcmp(row.author, "")))
		qsort(rows, table->num_rows, sizeof(Row), compare_rows_auth);
		
	else{
		for(size_t i = 0; i < table->num_rows; i++)
			(table->pager->sort)[i] = i;
		free(rows);
		return true;
	}
		
	for(size_t i = 0; i < table->num_rows; i++)
		(table->pager->sort)[i] = rows[i].id;

	free(rows);
}

bool execute_select(Table* table) {
	Row row;
	printf("\n");
	for (size_t i = 0; i < table->num_rows; i++) {
		deserialize_row(row_slot(table, (table->pager->sort)[i]), &row);
		print_row(&row);
	}
	printf("\n");
	return true;
}

Row *execute_get(Table* table, Row *row){
	Row *buff = (Row *) malloc(sizeof(Row));

	if(row->id != -1){
		deserialize_row(row_slot(table, row->id), buff);
		return buff;
	}

	bool mode = !(strcmp(row->name, ""));
	execute_sort(table, row);

	for (size_t i = 0; i < table->num_rows; i++){
		deserialize_row(row_slot(table, (table->pager->sort)[i]), buff);
		if(!mode){
			if(!strcmp(buff->name, row->name)){
				free(row);
				return buff;
			}
		}
		else{
			if(!strcmp(buff->author, row->author)){
				free(row);
				return buff;
			}

		}

	}
	free(buff);
	free(row);
	return NULL;
}

 bool execute_edit(Table *table, Row *row){
 	uint32_t buff = row->stock;
 	
 	if(row->id == -1)
 		row = execute_get(table, row);
 	
 	if(row == NULL)
 		return false;
 		
 	row->stock = buff;
 	
 	print_row(row);
 	
 	serialize_row(row, row_slot(table, row->id));
 	
 	free(row);
 	return true;
}

bool execute_delete(Table* table, Row *row) {
	uint32_t id;

	if(!strcmp(row->name, ""))
		row = execute_get(table, row);
	id = row->id;
	
    if (id >= table->num_rows) {
        return false;
    }

    // Get the row to be deleted
    row = (Row*)malloc(sizeof(Row));
    deserialize_row(row_slot(table, (table->pager->sort)[id]), row);

    // Shift rows after the deleted row to fill the gap
    for (uint32_t i = id + 1; i < table->num_rows; i++) {
        void* source = row_slot(table, (table->pager->sort)[i]);
        void* destination = row_slot(table, (table->pager->sort)[i - 1]);
		*((int*)source) -= 1;
        memcpy(destination, source, ROW_SIZE);
    }

    // Update the sort array and free memory
    table->num_rows--;
    table->pager->sort = (uint32_t*)realloc(table->pager->sort, table->num_rows * sizeof(uint32_t));
    free(row);

    return true;
}

// bool execute_delete(Table *table, Row *row){
// 	if(row->id == -1)
//  		row = execute_get(table, row);
 	
//  	if(row == NULL)
//  		return false;
 	
 	
//  	if(table->num_rows <= 0)
//  		return false;
 		
// 	for(size_t i = row->id; i < table->num_rows; i++){
// 		deserialize_row(row_slot(table, i+1), row);
// 		row->id = i;
// 		serialize_row(row, row_slot(table, i));
// 	}
// 	table->num_rows -= 1;
	
// 	Pager *pager = table->pager;
// 	uint32_t page_num = (table->num_rows + 1) / ROWS_PER_PAGE;
// 	printf("|%d|", page_num);
// 	off_t seek = lseek(pager->file_descriptor, )
// 	off_t truncate_result = ftruncate(table->pager->file_descriptor, (PAGE_SIZE * (page_num - 1)) + (table->num_rows * ROW_SIZE));
//     if (truncate_result == -1) {
//         printf("Error truncating file: %d\n", errno);
//         exit(EXIT_FAILURE);
//     }
	
// 	execute_sort(table, new_row(1,"","",0,0));
//  	free(row);
//  	return true;
// }

// int main(int argc, char* argv[]) {
// 	Table* table = db_open(bookFile);
	
// 	execute_sort(table, new_row(-1,"name", "", 0, 0));
	
// 	execute_select(table);
// 	db_close(table);
// }


#endif  // MDATABASE_H
