#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H
struct array_list;
struct array_list *alloc_array_list(size_t element_size);
void free_array_list(struct array_list *list);
unsigned int length_array_list(struct array_list *list);
void add_to_array_list(struct array_list *list, void *element);
void remove_from_array_list(struct array_list *list, unsigned int position);
void *array_list_get(struct array_list *list, unsigned int position);
#endif
