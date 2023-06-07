#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef enum {
  CECILE_VAL_BOOL,
  CECILE_VAL_NIL,
  CECILE_VAL_NUMBER,
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as; 
} Value;

#define CECILE_IS_BOOL(value)    ((value).type == CECILE_VAL_BOOL)
#define CECILE_IS_NIL(value)     ((value).type == CECILE_VAL_NIL)
#define CECILE_IS_NUMBER(value)  ((value).type == CECILE_VAL_NUMBER)

#define CECILE_AS_BOOL(value)   ((value).as.boolean)
#define CECILE_AS_NUMBER(value)   ((value).as.number)

#define CECILE_BOOL_VAL(value)  ((Value){CECILE_VAL_BOOL, {.boolean = value}})
#define CECILE_NIL_VAL          ((Value){CECILE_VAL_NIL, {.number = 0}})
#define CECILE_NUMBER_VAL(value)      ((Value){CECILE_VAL_NUMBER, {.number = value}})

typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif

