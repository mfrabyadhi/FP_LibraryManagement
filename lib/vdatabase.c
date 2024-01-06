#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys\stat.h>
//#include "vdatabase.h"

const char visFile[] = "vis.db";

#define TABLE_MAX_PAGES 100
#define COLUMN_NAME_SIZE 32
#define COLUMN_AUTHOR_SIZE 32

typedef struct row_t {
    char name[COLUMN_NAME_SIZE + 1];
    int id;
    time_t given;
} Row;

#define MEMBER_SIZE(Struct, Member) sizeof(((Struct*)0)->Member)

const uint32_t NAME_SIZE = MEMBER_SIZE(struct row_t, name);
const uint32_t ID_SIZE = MEMBER_SIZE(struct row_t, id);
const uint32_t GIVEN_SIZE = MEMBER_SIZE(struct row_t, given);

const uint32_t NAME_OFFSET = 0;
const uint32_t ID_OFFSET = NAME_OFFSET + NAME_SIZE;
const uint32_t GIVEN_OFFSET = ID_OFFSET + ID_SIZE;

const uint32_t ROW_SIZE = NAME_SIZE + ID_SIZE + GIVEN_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct Pager_t {
    int file_descriptor;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
    uint32_t* sort;
} Pager;

typedef struct Table_t {
    Pager* pager;
    uint32_t num_rows;
} Table;

bool execute_sort(Table*, Row*);

Row* new_row(char* name, int id, time_t given) {
    Row* row = (Row*)malloc(sizeof(Row));
    strncpy(row->name, name, COLUMN_NAME_SIZE);
    row->id = id;
    row->given = given;
    return row;
}

void print_row(Row* row) {
    printf("(%s, %d, %ld)\n", row->name, row->id, row->given);
}

void serialize_row(Row* source, void* destination) {
    memcpy(destination + NAME_OFFSET, &(source->name), NAME_SIZE);
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + GIVEN_OFFSET, &(source->given), GIVEN_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->name), source + NAME_OFFSET, NAME_SIZE);
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->given), source + GIVEN_OFFSET, GIVEN_SIZE);
}

void swap_row(void* first, void* second) {
    void* buff = malloc(ROW_SIZE);
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

void* get_page(Pager* pager, uint32_t page_num) {
    if(page_num > TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL) {
        // Cache miss. Allocate memory and load from file.
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;

        // We might save a partial page at the end of the file
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

        if (page_num >= pager->file_length) {
            pager->file_length = page_num * PAGE_SIZE;
        }
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

void pager_flush(Pager* pager, uint32_t page_num) {
    if (pager->pages[page_num] == NULL) {
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

    if (offset == -1) {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

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
    	pager_flush(pager, i);
    	free(pager->pages[i]);
    	pager->pages[i] = NULL;
    }

    // There may be a partial page to write at the end of the file
    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != NULL) {
            pager_flush(pager, page_num);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }

    int result = close(pager->file_descriptor);
    if (result == -1) {
        printf("Error closing file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = pager->pages[i];
        if (page != NULL) {
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free(pager);
    free(table);
}

bool execute_insert(Row *row, Table* table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return false;
    }
    serialize_row(row, row_slot(table, table->num_rows));
    table->num_rows += 1;
    if(table->num_rows > 1)
		table->pager->sort = (uint32_t *)realloc(table->pager->sort, table->num_rows * sizeof(uint32_t));

    execute_sort(table, new_row("name", -1, 10));

    free(row);
    return true;
}

bool execute_sort(Table *table, Row *row) {
    Row *rows = (Row *)malloc(sizeof(Row) * table->num_rows);
    for (size_t i = 0; i < table->num_rows; i++)
        deserialize_row(row_slot(table, i), &(rows[i]));

    if (!(strcmp(row->name, "")))
        qsort(rows, table->num_rows, sizeof(Row), compare_rows_name);
    else {
        for (size_t i = 0; i < table->num_rows; i++)
            (table->pager->sort)[i] = rows[i].id;
        free(rows);
        free(row);
        return true;
    }

    for (size_t i = 0; i < table->num_rows; i++)
        (table->pager->sort)[i] = rows[i].id;

    free(rows);
    free(row);
    return true;
}

bool execute_select(Table *table) {
    Row row;
    printf("\n");
    for (size_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, (table->pager->sort)[i]), &row);
        print_row(&row);
    }
    printf("\n");
    return true;
}

Row *execute_get(Table *table, Row *row) {
    Row *buff = (Row *)malloc(sizeof(Row));

    if (row->id != -1) {
        deserialize_row(row_slot(table, row->id), buff);
        return buff;
    }

    bool mode = !(strcmp(row->name, ""));
    execute_sort(table, row);

    for (size_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, (table->pager->sort)[i]), buff);
        if (!mode) {
            if (!strcmp(buff->name, row->name)) {
                free(row);
                return buff;
            }
        } 
    }
    free(buff);
    free(row);
    return NULL;
}

//bool execute_edit(Table *table, Row *row) {
//    uint32_t buff = row->stock;
//
//    if (row->id == -1)
//        row = execute_get(table, row);
//
//    if (row == NULL)
//        return false;
//
//    row->stock = buff;
//
//    print_row(row);
//
//    serialize_row(row, row_slot(table, row->id));
//
//    free(row);
//    return true;
//}

bool execute_delete(Table *table, Row *row) {
    if (row->id == -1)
        row = execute_get(table, row);

    if (row == NULL)
        return false;

    if (table->num_rows <= 0)
        return false;

    for (size_t i = row->id; i < table->num_rows; i++) {
    	        deserialize_row(row_slot(table, i + 1), row);
        row->id = i;
        serialize_row(row, row_slot(table, i));
    }
    table->num_rows -= 1;

    Pager* pager = table->pager;
    uint32_t page_num = (table->num_rows + 1) / ROWS_PER_PAGE;

    off_t truncate_result = ftruncate(table->pager->file_descriptor, PAGE_SIZE * (page_num - 1) + table->num_rows * ROW_SIZE);
    if (truncate_result == -1) {
        printf("Error truncating file: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    execute_sort(table, new_row("name", -1, 0));
    free(row);
    return true;
}

int main(int argc, char* argv[]) {
    Table* table = db_open(visFile);
    execute_insert(new_row("name1", 2, 10), table);
    
	printf("--%d--\n", table->num_rows);
    execute_sort(table, new_row("test", 0, 10));
    
    execute_select(table);
    db_close(table);
}
