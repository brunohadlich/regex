#include <stdlib.h>
#include <string.h>
#include "../headers/array_list.h"

struct array_list {
	void *data;
	size_t element_size;
	unsigned int length;
	unsigned int capacity;
};

struct array_list *alloc_array_list(size_t element_size) {
	struct array_list *list = (struct array_list *) malloc(sizeof(struct array_list));
	list->data = (void *)malloc(element_size);
	list->element_size = element_size;
	list->length = 0;
	list->capacity = 1;
	return list;
}

void free_array_list(struct array_list *list) {
	if (list) {
		free(list->data);
		free(list);
	}
}

unsigned int length_array_list(struct array_list *list) {
	return list->length;
}

void add_to_array_list(struct array_list *list, void *element) {
	if (list->length == list->capacity) {
		list->capacity *= 2;
		list->data = (void *)realloc(list->data, list->element_size * list->capacity);
	}
	memcpy((void *)((char *)list->data + list->length * list->element_size), element, list->element_size);
	list->length++;
}

void remove_from_array_list(struct array_list *list, unsigned int position) {
	if (position < list->length) {
		memcpy(	(void *)((char *)list->data + list->element_size * position),
				(void *)((char *)list->data + list->element_size * (position + 1)),
				(list->length - position) * list->element_size);
		list->length--;
		if (list->length < (int)(2./5 * list->capacity)) {
			list->capacity *= 3./5;
			list->data = reallocarray(list->data, list->element_size, list->capacity);
		}
	}
}

struct array_list * clear_array_list(struct array_list *list) {
	list->length = 0;
	list->capacity = 1;
	list->data = reallocarray(list->data, list->element_size, list->capacity);
	return list;
}

struct array_list *concatenate_array_lists(struct array_list *l1, struct array_list *l2) {
	if (l1->element_size != l2->element_size) {
		return NULL;
	}
	struct array_list *new_list = (struct array_list *)malloc(sizeof(struct array_list));
	new_list->element_size = l1->element_size;
	new_list->capacity = new_list->length = length_array_list(l1) + length_array_list(l2);
	new_list->data = (void *)malloc(new_list->capacity * new_list->element_size);
	memcpy(new_list->data, l1->data, l1->length * l1->element_size);
	memcpy(((char *)new_list->data) + l1->length * l1->element_size, l2->data, l2->length * l2->element_size);
	return new_list;
}

void *array_list_get(struct array_list *list, unsigned int position) {
	if (position < list->length) {
		return (void *)((char *)list->data + position * list->element_size);
	}
	return NULL;
}

void __fill_array(struct array_list *list, int values[]) {
	int i;
	for (i = 0; values[i] != -1; i++) {
		add_to_array_list(list, &values[i]);
	}
}

int __check_array_list_values(struct array_list *list, int values[]) {
	int i;
	for (i = 0; values[i] != -1; i++) {
		if (*(int *)array_list_get(list, i) != values[i]) {
			return 1;
		}
	}
	return 0;
}

int __test_array_list() {
	struct array_list *al = alloc_array_list(sizeof(int));
	struct array_list *al2 = alloc_array_list(sizeof(int));
	struct array_list *al3;
	int values[] = {5, 1, 8, 5, 4, 4, 58, 20, 13, -1};
	int values2[] = {34, 13, 79, 532, 3, -1};
	int values3[] = {5, 1, 8, 5, 4, 4, 58, 20, 13, 34, 13, 79, 532, 3, -1};//[values + values2]
	__fill_array(al, values);
	__fill_array(al2, values2);
	al3 = concatenate_array_lists(al, al2);
	if (__check_array_list_values(al, values) ||
		__check_array_list_values(al2, values2) ||
		__check_array_list_values(al3, values3)) {
		return 1;
	}
	clear_array_list(al);
	if (al->length != 0) {
		return 1;
	}
	free_array_list(al);
	free_array_list(al2);
	free_array_list(al3);
	return 0;
}
