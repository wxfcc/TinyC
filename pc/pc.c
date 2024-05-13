#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

typedef struct _pc_object {
    int refCount;
}_pc_object, *pc_object;

typedef struct _pc_string {
    _pc_object object;
    int length;
    char data[0];
}_pc_string, *pc_string;
//
void pc_object_add_ref(pc_object object);
//
pc_object pc_object_new(int object_size) {
    pc_object object = malloc(object_size);
    if (object) {
        memset(object, 0, object_size);
        pc_object_add_ref(object);
    }
    return object;
}
void pc_object_delete(pc_object object) {
    free(object);
}
void pc_object_add_ref(pc_object object) {
    object->refCount ++;
}
void pc_object_dec_ref(pc_object object) {
    object->refCount--;
    if (object->refCount <= 0)
        pc_object_delete(object);
}
//
pc_string pc_String_new(const char*s) {
    pc_string string;
    int len = (int)strlen(s) + 1;
    int object_len = len + sizeof(_pc_string);
    string = (pc_string)pc_object_new(object_len);
    string->length = len;
    memcpy(string->data, s, len);
    return string;
}
void pc_String_finalize(pc_string string) {
    pc_object_dec_ref(&string->object);
}
int pc_String_len(pc_string string) {
    return string->length;
}
char* pc_String_getData(pc_string string) {
    return string->data;
}
//
void pc_System_out_println(pc_string string) {
    char* s = pc_String_getData(string);
    printf("%s\n", s);
}
//
int main()
{
    pc_string s = pc_String_new("Hello Pack C");
    pc_System_out_println(s);
    pc_String_finalize(s);
    return 0;
}

