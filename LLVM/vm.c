#include "chunk.h"
#include "common.h"
#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "compiler.h"
#include "debug.h"
#include <stdarg.h>
#include <string.h>
#include "object.h"
VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code -1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line); 
  resetStack();
}

void initVM() {
  resetStack();
  vm.objects = NULL;
  initTable(&vm.strings);
}

void freeVM() {
  freeTable(&vm.strings);
  freeObjects();

} 

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value) {
  return CECILE_IS_NIL(value) || (CECILE_IS_BOOL(value) && !CECILE_AS_BOOL(value));
}

static void concatenate() {
  ObjString* b = CECILE_AS_STRING(pop());
  ObjString* a = CECILE_AS_STRING(pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = takeString(chars, length);
  push(CECILE_OBJ_VAL(result));
}

static InterpretResult run() {
  #define READ_BYTE() (*vm.ip++)
  #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
  #define BINARY_OP(valueType, op) \
    do { \
      if (!CECILE_IS_NUMBER(peek(0)) || !CECILE_IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = CECILE_AS_NUMBER(pop()); \
      double a = CECILE_AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

  for (;;) {
    #ifdef DEBUG_TRACE_EXECUTION
      printf("          ");
      for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
      disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
    #endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NIL: push(CECILE_NIL_VAL); break;
      case OP_TRUE: push(CECILE_BOOL_VAL(true)); break;
      case OP_FALSE: push(CECILE_BOOL_VAL(false)); break;
      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(CECILE_BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_GREATER: BINARY_OP(CECILE_BOOL_VAL, >); break;
      case OP_LESS: BINARY_OP(CECILE_BOOL_VAL, <); break;
      case OP_ADD: {
        if (CECILE_IS_STRING(peek(0)) && CECILE_IS_STRING(peek(1))) {
          concatenate();
        } else if (CECILE_IS_NUMBER(peek(0)) && CECILE_IS_NUMBER(peek(1))) {
          double b = CECILE_AS_NUMBER(pop());
          double a = CECILE_AS_NUMBER(pop());
          push(CECILE_NUMBER_VAL(a + b)); 
        } else {
          runtimeError("Operands must be two numbers or two string");
          return INTERPRET_RUNTIME_ERROR;
        }
      }
      case OP_SUBTRACT: BINARY_OP(CECILE_NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(CECILE_NUMBER_VAL, *); break;
      case OP_DIVIDE: BINARY_OP(CECILE_NUMBER_VAL, /); break;
      case OP_NOT:
        push(CECILE_BOOL_VAL(isFalsey(pop())));
        break;
      case OP_NEGATE: 
        if (!CECILE_IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number."); 
          return INTERPRET_RUNTIME_ERROR;
        }
        push(CECILE_NUMBER_VAL(-CECILE_AS_NUMBER(pop())));
        break;
      case OP_RETURN: {
        printValue(pop());
        printf("\n");
         return INTERPRET_OK;
      }
    }
  }
  #undef READ_BYTE
  #undef READ_CONSTANT 
  #undef BINARY_OP
}

InterpretResult interpret(const char* source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source,&chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result  = run();

  freeChunk(&chunk);
  return result;

}


