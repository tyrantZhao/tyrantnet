#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <tyrant/common/array.h>

struct array_s
{
    char*   buffer;
    int     buffer_size;
    int     element_size;
    int     element_num;
};

struct array_s*
ox_array_new(int num, int element_size)
{
    int buffer_size = num * element_size;
    struct array_s* ret = (struct array_s*)malloc(sizeof(struct array_s));
    if (NULL == ret) {
        return NULL;
    }

    memset(ret, 0, sizeof(*ret));
    ret->buffer = (char*)malloc(buffer_size);
    if (NULL != ret->buffer) {
        memset(ret->buffer, 0, buffer_size);
        ret->element_size = element_size;
        ret->element_num = num;
        ret->buffer_size = buffer_size;
    } else {
        ox_array_delete(ret);
        ret = NULL;
    }

    return ret;
}

void
ox_array_delete(struct array_s* self)
{
    if (NULL == self)
        return;

    if (NULL != self->buffer) {
        free(self->buffer);
        self->buffer = NULL;
    }

    self->buffer_size = 0;
    self->element_size = 0;
    self->element_num = 0;

    free(self);
    self = NULL;
}

char*
ox_array_at(struct array_s* self, int index)
{
    if (NULL == self)
        return NULL;
    if (NULL == self->buffer)
        return NULL;

    char* ret = NULL;

    if (0 <= index && index < self->element_num) {
        ret = self->buffer + (index * self->element_size);
    } else {
        assert(false);
    }

    return ret;
}

bool
ox_array_set(struct array_s* self, int index, const void* data)
{
    if (NULL == self)
        return false;
    if (NULL == self->buffer)
        return false;

    char* old_data = ox_array_at(self, index);

    if(NULL != old_data) {
        memcpy(old_data, data, self->element_size);
        return true;
    } else {
        return false;
    }
}

bool
ox_array_increase(struct array_s* self, int increase_num)
{
    if (NULL == self)
        return false;
    if (NULL == self->buffer)
        return false;

    int new_buffer_size = 0;
    char* new_buffer = NULL;

    if (0 >= increase_num)
        return false;

    new_buffer_size = self->buffer_size + increase_num * self->element_size;
    new_buffer = (char*)malloc(new_buffer_size);

    if (new_buffer != NULL) {
        memset(new_buffer, 0, new_buffer_size);
        memcpy(new_buffer, self->buffer, self->buffer_size);
        free(self->buffer);
        self->buffer = new_buffer;
        self->element_num += increase_num;
        self->buffer_size = new_buffer_size;

        return true;
    }
    else {
        assert(false);
        return false;
    }
}

int
ox_array_num(const struct array_s* self)
{
    if (NULL == self || NULL == self->buffer)
        return 0;

    return self->element_num;
}
