/*Database for both books and visitors data
it supposed to have 2 .db file*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "database.h"

//print data
void print_data(Data* data) {
	if(data->type == BOOK)
		print_book(data->data);
	else if(data->type == VISITOR)
		print_visitor(data->data);
}

//print book
void print_book(void* book) {
	Book* b = (Book*)book;
	printf("Book ID: %d\n", b->id);
	printf("Title: %s\n", b->title);
	printf("Author: %s\n", b->author);
	printf("Year: %d\n", b->year);
	printf("Stock: %d\n", b->stock);
}

//print visitor
void print_visitor(void* visitor) {
	Visitor* v = (Visitor*)visitor;
	printf("Visitor ID: %d\n", v->id);
	printf("Name: %s\n", v->name);
	printf("Book ID: %d\n", v->book_id);
	printf("Last Borrowed: %s\n", ctime(&v->last_borrowed));
}

//delete data
void delete_data(Data* data) {
	if(data->data != NULL){
		if(data->type == BOOK)
			delete_book(data->data);
		else if(data->type == VISITOR){
			delete_visitor(data->data);
		}
	}
	free(data);
	data = NULL;
}

//delete a book
void delete_book(void* book) {
	Book* b = (Book*)book;
	free(b);
	book = NULL;
}

//delete a visitor
void delete_visitor(void* visitor) {
	Visitor* v = (Visitor*)visitor;
	free(v);
	visitor = NULL;
}

//create a new data
Data *new_data(DataType type, void* data) {
	Data* d = malloc(sizeof(Data));
	d->type = type;
	d->data = data;
	return d;
}

//create a new book
Book* new_book(uint32_t id, char* title, char* author, uint32_t year, uint32_t stock) {
	Book* b = malloc(sizeof(Book));
	b->id = id;
	strncpy(b->title, title, MAX_TITLE_SIZE);
	strncpy(b->author, author, MAX_AUTHOR_SIZE);
	b->year = year;
	b->stock = stock;
	return b;
}

//create a new visitor
Visitor* new_visitor(uint32_t id, char* name, uint32_t book_id, time_t last_borrowed) {
	Visitor* v = malloc(sizeof(Visitor));
	v->id = id;
	strncpy(v->name, name, MAX_NAME_SIZE);
	v->book_id = book_id;
	v->last_borrowed = last_borrowed;
	return v;
}

//update book
void update_book(void* book, void* new_book) {
	Book* b = (Book*)book;
	Book* nb = (Book*)new_book;
	b->id = nb->id;
	strncpy(b->title, nb->title, MAX_TITLE_SIZE);
	strncpy(b->author, nb->author, MAX_AUTHOR_SIZE);
	b->year = nb->year;
	b->stock = nb->stock;
}

//update visitor
void update_visitor(void* visitor, void* new_visitor) {
	Visitor* v = (Visitor*)visitor;
	Visitor* nv = (Visitor*)new_visitor;
	v->id = nv->id;
	strncpy(v->name, nv->name, MAX_NAME_SIZE);
	v->book_id = nv->book_id;
	v->last_borrowed = nv->last_borrowed;
}

//update data
void update_data(Data* data, DataType type, void* new_data) {
	if(type == BOOK)
		update_book(data->data, new_data);
	else if(type == VISITOR)
		update_visitor(data->data, new_data);
	else
		printf("Invalid data type\n");
}

void update_data_id(Data* data, uint32_t id) {
	if(data->type == BOOK)
		((Book*)data->data)->id = id;
	else if(data->type == VISITOR)
		((Visitor*)data->data)->id = id;
	else
		printf("Invalid data type\n");
}

//copy data
void copy_data(Data* dest, Data* src) {
	if(src->type == BOOK)
		copy_book(dest->data, src->data);
	else if(src->type == VISITOR)
		copy_visitor(dest->data, src->data);
}

void copy_book(void* dest, void* src) {
	Book* b = (Book*)dest;
	Book* nb = (Book*)src;
	b->id = nb->id;
	strncpy(b->title, nb->title, MAX_TITLE_SIZE);
	strncpy(b->author, nb->author, MAX_AUTHOR_SIZE);
	b->year = nb->year;
	b->stock = nb->stock;
}

void copy_visitor(void* dest, void* src) {
	Visitor* v = (Visitor*)dest;
	Visitor* nv = (Visitor*)src;
	v->id = nv->id;
	strncpy(v->name, nv->name, MAX_NAME_SIZE);
	v->book_id = nv->book_id;
	v->last_borrowed = nv->last_borrowed;
}

//Comparing Data
uint32_t compare_data(Data* d1, Data* d2) {
	if(d1->type == BOOK)
		return compare_book(d1->data, d2->data);
	else if(d1->type == VISITOR)
		return compare_visitor(d1->data, d2->data);
}

uint32_t compare_book(void* b1, void* b2) {
	Book* book1 = (Book*)b1;
	Book* book2 = (Book*)b2;
	return strncasecmp(book1->title, book2->title, MAX_TITLE_SIZE);
}

uint32_t compare_visitor(void* v1, void* v2) {
	Visitor* visitor1 = (Visitor*)v1;
	Visitor* visitor2 = (Visitor*)v2;
	return strncasecmp(visitor1->name, visitor2->name, MAX_NAME_SIZE);
}

uint32_t get_id(Data *data){
	if(data->type == BOOK)
		return ((Book*)data->data)->id;
	else if(data->type == VISITOR)
		return ((Visitor*)data->data)->id;
	return -1;
}

//ukuran setiap member dari Book ketika disimpan pada file
const uint32_t BOOK_ID_SIZE = sizeof(uint32_t);
const uint32_t BOOK_TITLE_SIZE = sizeof(char) * MAX_TITLE_SIZE;
const uint32_t BOOK_AUTHOR_SIZE = sizeof(char) * MAX_AUTHOR_SIZE;
const uint32_t BOOK_YEAR_SIZE = sizeof(uint32_t);
const uint32_t BOOK_STOCK_SIZE = sizeof(uint32_t);

//lokasi setiap member dari buku didalam file
const uint32_t BOOK_ID_OFFSET = 0;
const uint32_t BOOK_TITLE_OFFSET = BOOK_ID_OFFSET + BOOK_ID_SIZE;
const uint32_t BOOK_AUTHOR_OFFSET = BOOK_TITLE_OFFSET + BOOK_TITLE_SIZE;
const uint32_t BOOK_YEAR_OFFSET = BOOK_AUTHOR_OFFSET + BOOK_AUTHOR_SIZE;
const uint32_t BOOK_STOCK_OFFSET = BOOK_YEAR_OFFSET + BOOK_YEAR_SIZE;

//total ukuran buku
const uint32_t BOOK_SIZE = BOOK_STOCK_OFFSET + BOOK_STOCK_SIZE;

//ukuran setiap member dari Visitor ketika disimpan pada file
const uint32_t VISITOR_ID_SIZE = sizeof(uint32_t);
const uint32_t VISITOR_NAME_SIZE = sizeof(char) * MAX_NAME_SIZE;
const uint32_t VISITOR_BOOK_ID_SIZE = sizeof(uint32_t);
const uint32_t VISITOR_LAST_BORROWED_SIZE = sizeof(time_t);

//lokasi setiap member dari Visitor didalam file
const uint32_t VISITOR_ID_OFFSET = 0;
const uint32_t VISITOR_NAME_OFFSET = VISITOR_ID_OFFSET + VISITOR_ID_SIZE;
const uint32_t VISITOR_BOOK_ID_OFFSET = VISITOR_NAME_OFFSET + VISITOR_NAME_SIZE;
const uint32_t VISITOR_LAST_BORROWED_OFFSET = VISITOR_BOOK_ID_OFFSET + VISITOR_BOOK_ID_SIZE;

//total ukuran Visitor
const uint32_t VISITOR_SIZE = VISITOR_LAST_BORROWED_OFFSET + VISITOR_LAST_BORROWED_SIZE;

//ukuran page dalam file
const uint32_t PAGE_SIZE = 4096;

//untuk menghitung ukuran dari data yang digunakan
const uint32_t DATA_SIZE(DataType type) {
	if (type == BOOK) {
		return BOOK_SIZE;
	}
	else if (type == VISITOR) {
		return VISITOR_SIZE;
	}
}

//jumlah data dalam 1 page
const uint32_t DATA_PER_PAGE(DataType type) {
	if (type == BOOK) {
		return PAGE_SIZE / BOOK_SIZE;
	}
	else if (type == VISITOR) {
		return PAGE_SIZE / VISITOR_SIZE;
	}
}

const uint32_t WASTED_SPACE(DataType type) {
	if (type == BOOK) {
		return PAGE_SIZE % BOOK_SIZE;
	}
	else if (type == VISITOR) {
		return PAGE_SIZE % VISITOR_SIZE;
	}
}

//untuk memindahkan data ke lokasi memory pada page
//dari struct data(book/visitor)
void serialize_book(Book* src, void* dest) {
	memcpy(dest + BOOK_ID_OFFSET, &(src->id), BOOK_ID_SIZE);
	memcpy(dest + BOOK_TITLE_OFFSET, &(src->title), BOOK_TITLE_SIZE);
	memcpy(dest + BOOK_AUTHOR_OFFSET, &(src->author), BOOK_AUTHOR_SIZE);
	memcpy(dest + BOOK_YEAR_OFFSET, &(src->year), BOOK_YEAR_SIZE);
	memcpy(dest + BOOK_STOCK_OFFSET, &(src->stock), BOOK_STOCK_SIZE);
}
void serialize_visitor(Visitor* src, void* dest) {
	memcpy(dest + VISITOR_ID_OFFSET, &(src->id), VISITOR_ID_SIZE);
	memcpy(dest + VISITOR_NAME_OFFSET, &(src->name), VISITOR_NAME_SIZE);
	memcpy(dest + VISITOR_BOOK_ID_OFFSET, &(src->book_id), VISITOR_BOOK_ID_SIZE);
	memcpy(dest + VISITOR_LAST_BORROWED_OFFSET, &(src->last_borrowed), VISITOR_LAST_BORROWED_SIZE);
}
void serialize_data(Data* src, void* dest) {
	if (src->type == BOOK) {
		serialize_book(src->data, dest);
	}
	else if (src->type == VISITOR) {
		serialize_visitor(src->data, dest);
	}
}

//untuk memindahkan data dari page yang berada di memory
//ke struct data(book, visitor)
void deserialize_book(void* src, Book* dest) {
	memcpy(&(dest->id), src + BOOK_ID_OFFSET, BOOK_ID_SIZE);
	memcpy(&(dest->title), src + BOOK_TITLE_OFFSET, BOOK_TITLE_SIZE);
	memcpy(&(dest->author), src + BOOK_AUTHOR_OFFSET, BOOK_AUTHOR_SIZE);
	memcpy(&(dest->year), src + BOOK_YEAR_OFFSET, BOOK_YEAR_SIZE);
	memcpy(&(dest->stock), src + BOOK_STOCK_OFFSET, BOOK_STOCK_SIZE);
}
void deserialize_visitor(void* src, Visitor* dest) {
	memcpy(&(dest->id), src + VISITOR_ID_OFFSET, VISITOR_ID_SIZE);
	memcpy(&(dest->name), src + VISITOR_NAME_OFFSET, VISITOR_NAME_SIZE);
	memcpy(&(dest->book_id), src + VISITOR_BOOK_ID_OFFSET, VISITOR_BOOK_ID_SIZE);
	memcpy(&(dest->last_borrowed), src + VISITOR_LAST_BORROWED_OFFSET, VISITOR_LAST_BORROWED_SIZE);
}
void deserialize_data(void* src, Data* dest, DataType type) {
	if (type == BOOK) {
		Book* book = (Book*)malloc(sizeof(Book));
		deserialize_book(src, book);
		dest->data = book;
		dest->type = BOOK;
	}
	else if (type == VISITOR) {
		Visitor* visitor = (Visitor*)malloc(sizeof(Visitor));
		deserialize_visitor(src, visitor);
		dest->data = visitor;
		dest->type = VISITOR;
	}
}

//membuka pager dan menginisialisasi semua page
Pager *pager_open(const char* filename) {
	int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
	if (fd == -1) {
		printf("Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	off_t file_length = lseek(fd, 0, SEEK_END);
	if(file_length == -1){
		printf("Error seeking file end\n");
		exit(EXIT_FAILURE);
	}

	Pager* pager = (Pager*)malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_length = file_length;
	pager->num_pages = (file_length / PAGE_SIZE) + 1;

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->pages[i] = NULL;
	}

	pager->sorted = (uint32_t*)malloc(sizeof(uint32_t) * TABLE_MAX_PAGES);
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->sorted[i] = i;
	}

	return pager;
}

//untuk membuka database
Table* db_open(const char* filename, DataType type) {
	Pager* pager = pager_open(filename);
	uint32_t num_full_pages = pager->file_length / PAGE_SIZE;
	uint32_t num_data = (pager->file_length - num_full_pages * WASTED_SPACE(type)) / DATA_SIZE(type);

	Table* table = (Table*)malloc(sizeof(Table));
	table->pager = pager;
	table->type = type;
	table->num_data = num_data;

	return table;
}

//untuk menyimpan page ke dalam file
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

	uint32_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);
	if (bytes_written == -1) {
		printf("Error writing: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	printf("Wrote %d bytes\n", bytes_written);
}

//untuk menutup database
void db_close(Table* table) {
	Pager* pager = table->pager;
	uint32_t full_pages = table->num_data / DATA_PER_PAGE(table->type);

	for (uint32_t i = 0; i < full_pages; i++) {
		if (pager->pages[i] == NULL) {
			continue;
		}
		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	//ketika page tidak terisi penuh
	uint32_t num_additional_rows = table->num_data % DATA_PER_PAGE(table->type);
	if (num_additional_rows > 0) {
		uint32_t page_num = full_pages;
		if (pager->pages[page_num] != NULL) {
			pager_flush(pager, page_num, num_additional_rows * DATA_SIZE(table->type));
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
	free(table);
}

//mencari lokasi "page"
void* get_page(Table *table, uint32_t page_num) {
	Pager* pager = table->pager;
	if (page_num > TABLE_MAX_PAGES) {
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
		exit(EXIT_FAILURE);
	}

	//jika page belum tersedia maka akan dibuat atau di load
	if (pager->pages[page_num] == NULL) {
		void* page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_length / PAGE_SIZE;

		if (pager->file_length % DATA_SIZE(table->type)) {
			num_pages += 1;
		}

		if (page_num <= num_pages) {
			lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
			uint32_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
			if (bytes_read == -1) {
				printf("Error reading file: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}

		pager->pages[page_num] = page;
	}
	return pager->pages[page_num];
}

//untuk mendapatkan lokasi penyimpanan di database
void* row_slot(Table* table, uint32_t row_num) {
	uint32_t page_num = row_num / DATA_PER_PAGE(table->type);
	void* page = get_page(table, page_num);
	uint32_t row_offset = row_num % DATA_PER_PAGE(table->type);
	uint32_t byte_offset = row_offset * DATA_SIZE(table->type);
	return page + byte_offset;
}

//untuk di passing ke quicksort
int sort_by_name(const void* a, const void* b) {
	Data* da = (Data*)a;
	Data* db = (Data*)b;
	return strcmp(((Visitor *)(da->data))->name, ((Visitor *)(db->data))->name);
}
int sort_by_title(const void* a, const void* b) {
	Data* da = (Data*)a;
	Data* db = (Data*)b;
	return strcmp(((Book *)(da->data))->title, ((Book *)(db->data))->title);
}

//untuk mengurutkan data
bool table_sort(Table* table, SortType s){
	Data* data = (Data*)malloc(table->num_data * sizeof(Data));

	if(data == NULL){
		printf("Error: Cannot allocate memory: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	if(s == SORTID){
		for(uint32_t i = 0; i < table->num_data; i++)
			table->pager->sorted[i] = i;
		free(data);
		return true;	
	}

	for(uint32_t i = 0; i < table->num_data; i++){
		data[i].data = row_slot(table, i);
	}

	if(s == SORTNAME && table->type == VISITOR){
		qsort(data, table->num_data, sizeof(Data), sort_by_name);
		for(uint32_t i = 0; i < table->num_data; i++)
			table->pager->sorted[i] = ((Visitor *)(data[i].data))->id - 1;
	}
	else{
		perror("Error: Cannot sort by name\n");
		exit(EXIT_FAILURE);
	}
		
	if(s == SORTTITLE && table->type == BOOK){
		qsort(data, table->num_data, sizeof(Data), sort_by_title);
		for(uint32_t i = 0; i < table->num_data; i++)
			table->pager->sorted[i] = ((Book *)(data[i].data))->id - 1;
	}
	else{
		perror("Error: Cannot sort by title\n");
		exit(EXIT_FAILURE);
	}
	return true;
}

//untuk memasukan data ke table
bool table_insert(Table* table, Data* data) {
	uint32_t num_data = table->num_data;
	table->num_data += 1;

	if(num_data >= TABLE_MAX_PAGES * DATA_PER_PAGE(table->type)) {
		printf("Error: Table full.\n");
		exit(EXIT_FAILURE);
	}

	if(table->type == BOOK){
		((Book *)(data->data))->id = table->num_data;
	}
	else if(table->type == VISITOR){
		((Visitor *)(data->data))->id = table->num_data;
	}

	serialize_data(data, row_slot(table, num_data));

	table->pager->sorted = (uint32_t*)realloc(table->pager->sorted, table->num_data * sizeof(uint32_t));
	table_sort(table, SORTID);
	return true;
}

//mencetak data dari database
bool table_print_all(Table* table) {
	Data* data = (Data*)malloc(sizeof(Data));
	if(table->num_data == 0){
		printf("No data\n");
		return false;
	}
	for (uint32_t i = 0; i < table->num_data; i++) {
		deserialize_data(row_slot(table, table->pager->sorted[i]), data, table->type);
		print_data(data);
		printf("\n");
	}
	free(data);
	return true;
}

//untuk mencetak data dari database di suatu range
bool table_print_in_range(Table *table, uint32_t start, uint32_t end){
	start--; end--;
	Data* data = (Data*)malloc(sizeof(Data));
	if(start < 0 || start > end || end > table->num_data){
		printf("Error: Invalid range\n");
		return false;
	}
	for (uint32_t i = start; i < end; i++) {
		deserialize_data(row_slot(table, table->pager->sorted[i]), data, table->type);
		print_data(data);
		printf("\n");
	}
	free(data);
	return true;
}

//untuk mencetak satu data
bool table_print_by_id(Table *table, uint32_t id){
	Data* data = (Data*)malloc(sizeof(Data));
	if(id > table->num_data){
		printf("Error: Invalid id\n");
		return false;
	}
	deserialize_data(row_slot(table, table->pager->sorted[id-1]), data, table->type);
	print_data(data);
	printf("\n");
	free(data);
	return true;
}

//untuk memilih data berdasarkan nama
bool table_select_by_name(Table *table, char *name, Data *data){
	Data* temp = (Data*)malloc(sizeof(Data));
	if(table->type == BOOK){
		data->type = BOOK;
		data->data = malloc(sizeof(Book));
		strncpy(((Book *)(data->data))->title, name, MAX_TITLE_SIZE);
	}
	else if(table->type == VISITOR){
		data->type = VISITOR;
		data->data = malloc(sizeof(Visitor));
		strncpy(((Visitor *)(data->data))->name, name, MAX_NAME_SIZE);
	}
	else{
		perror("Error: Invalid table type\n");
		exit(EXIT_FAILURE);
	}

	for(uint32_t i = 0; i < table->num_data; i++){
		deserialize_data(row_slot(table, i), temp, table->type);
		if(compare_data(data, temp) == 0){
			deserialize_data(row_slot(table, i), data, table->type);
			printf("CHECK\n");
			free(temp);
			return true;
		}
	}
	data->data	= NULL;
	free(temp);
	return false;
}

//untuk memilih data dari database berdasarkan id
bool table_select(Table *table, uint32_t id, Data *data){
	id--;

	if(id < 0 || id > table->num_data){
		printf("Error: Invalid id\n");
		return false;
	}
	deserialize_data(row_slot(table, table->pager->sorted[id]), data, table->type);
	return true;
}

//menghapus data dari table
bool table_delete(Table *table, uint32_t id){
	id--;

	Data *temp = malloc(sizeof(Data));
	if(id < 0 || id > table->num_data){
		printf("Error: Invalid id\n");
		return false;
	}
	for(uint32_t i = id; i < table->num_data - 1; i++){
		deserialize_data(row_slot(table, i + 1), temp, table->type);
		update_data_id(temp, i + 1);
		serialize_data(temp, row_slot(table, i));
	}
	table->num_data -= 1;
	table->pager->sorted = (uint32_t*)realloc(table->pager->sorted, table->num_data * sizeof(uint32_t));
	table_sort(table, SORTID);
	return true;
}

//mengupdate isi data pada table
bool table_update(Table *table, Data *data){
	uint32_t id = get_id(data) - 1;
	if(id < 0 || id > table->num_data){
		printf("Error: Invalid id\n");
		return false;
	}
	serialize_data(data, row_slot(table, id));
	return true;
}

//generate 5 visitors 
Visitor visitor[] = {
	{1, "John", 1, 0},
	{2, "Jane", 2, 0},
	{3, "Jack", 3, 0},
	{4, "Jill", 4, 0},
	{5, "James", 5, 0}
};

// int main(void) {
// 	Table* table = db_open("book.db", BOOK);
// 	Table* visitor_table = db_open("visitor.db", VISITOR);
// 	Data* data = (Data *) malloc(sizeof(Data));
	
// 	data->type = VISITOR;
// 	for(int i = 0; i < 5; i++){
// 		data->data = &visitor[i];
// 		table_insert(visitor_table, data);
// 	}
// 	data->data = NULL;
// 	delete_data(data);

// 	table_print_all(visitor_table);

// 	db_close(table);
// 	db_close(visitor_table);

// 	return 0;
// }
