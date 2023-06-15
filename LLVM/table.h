#ifndef clox_table_h
#define clox_table_h

#include "object.h"
#include "common.h"
#include "value.h"

typedef struct {
  ObjString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;


void initTable(Table* table);
void freeTable(Table* table);
void tableSet(Table* table, ObjString* key, Value value);

#endif
