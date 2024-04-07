#include "string.h"

#include <stdlib.h>

struct string *string_create(const char *str, size_t size)
{
    struct string *toret = malloc(sizeof(struct string));
    if (toret == NULL)
    {
        return NULL;
    }
    toret->size = size;
    if (size == 0 || str == NULL)
    {
        toret->data = NULL;
        return toret;
    }
    toret->data = malloc(size * sizeof(char));
    for (size_t i = 0; i < size; i++)
    {
        toret->data[i] = str[i];
    }
    return toret;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    size_t i = 0;
    while (i < n)
    {
        if (str1->data[i] != str2[i])
        {
            return str1->data[i] - str2[i];
        }
        i = i + 1;
    }
    return 0;
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    if (str == NULL)
    {
        return;
    }
    str->data = realloc(str->data, size + str->size);
    for (size_t i = 0; i < size; i++)
    {
        str->data[i + str->size] = to_concat[i];
    }
    str->size = str->size + size;
}
void string_destroy(struct string *str)
{
    free(str->data);
    free(str);
}
