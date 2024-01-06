#ifndef DATABASE_H
#define DATABASE_H

#define MAX_BOOKS 100
#define MAX_VISITORS 100
#define MAX_NAME_SIZE 100
#define MAX_TITLE_SIZE 100
#define MAX_AUTHOR_SIZE 100
#define TABLE_MAX_PAGES 100

//tipe data Datatype untuk membedakan antara Book dan Visitor
typedef enum {
	BOOK,
	VISITOR
} DataType;

//tipe data untuk memilih tipe sorting
typedef enum {
	SORTID,
	SORTNAME,
	SORTTITLE,
} SortType;

//struct of data
typedef struct data_t {
	DataType type;
	void* data;
} Data;

//struct of book
typedef struct book_t {
	uint32_t id;
	char title[MAX_TITLE_SIZE];
	char author[MAX_AUTHOR_SIZE];
	uint32_t year;
	uint32_t stock;
} Book;

//struct of visitor
typedef struct visitor_t{
	uint32_t id;
	char name[MAX_NAME_SIZE];
	uint32_t book_id;
	time_t last_borrowed;
} Visitor;

//struct untuk menyimpan dan mengolah semua page
typedef struct pager_t{
	int file_descriptor;
	uint32_t file_length;
	void* pages[TABLE_MAX_PAGES];
	uint32_t num_pages;
	uint32_t *sorted;
} Pager;

//database
typedef struct table_t{
	Pager *pager;
	DataType type;
	uint32_t num_data;
} Table;

void print_data(Data*);
void print_book(void*);
void print_visitor(void*);
void delete_data(Data*);
void delete_book(void*);
void delete_visitor(void*);
Data* new_data(DataType, void*);
Book* new_book(uint32_t, char*, char*, uint32_t, uint32_t);
Visitor* new_visitor(uint32_t, char*, uint32_t, time_t);
void update_data(Data*, DataType, void*);
void update_book(void*, void*);
void update_visitor(void*, void*);
void copy_data(Data*, Data*);
void copy_book(void*, void*);
void copy_visitor(void*, void*);
uint32_t compare_data(Data*, Data*);
uint32_t compare_book(void*, void*);
uint32_t compare_visitor(void*, void*);
uint32_t get_id(Data*);
void update_data_id(Data*, uint32_t);
bool table_sort(Table*, SortType);
bool table_insert(Table*, Data*);
bool table_delete(Table*, uint32_t);
bool table_update(Table*, Data*);
bool table_select(Table*, uint32_t, Data*);
bool table_select_by_name(Table*, char*, Data*);
bool table_print_all(Table*);
bool table_print_in_range(Table*, uint32_t, uint32_t);
bool table_print_by_id(Table*, uint32_t);

#endif // DATABASE_H