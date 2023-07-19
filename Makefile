# Basic compilation macros
CC = gcc # GCC Compiler
CFLAGS = -ansi -Wall -pedantic # Flags
GLOBAL_DEPS = globals.h # Dependencies for everything
EXE_DEPS = assembler.o utils_code_functions.o process_first_pass.o process_second_pass.o process_macros.o instructions.o table_operations.o utils_functions.o writefiles.o # Deps for exe

## Executable
assembler: $(EXE_DEPS) $(GLOBAL_DEPS)
	$(CC) -g $(EXE_DEPS) $(CFLAGS) -o $@

## Main:
assembler.o: assembler.c $(GLOBAL_DEPS)
	$(CC) -c assembler.c $(CFLAGS) -o $@

## Code helper functions:
utils_code_functions.o: utils_code_functions.c utils_code_functions.h $(GLOBAL_DEPS)
	$(CC) -c utils_code_functions.c $(CFLAGS) -o $@

## Process macros:
process_macros.o: process_macros.c process_macros.h $(GLOBAL_DEPS)
	$(CC) -c process_macros.c $(CFLAGS) -o $@

## Process first pass:
process_first_pass.o: process_first_pass.c process_first_pass.h $(GLOBAL_DEPS)
	$(CC) -c process_first_pass.c $(CFLAGS) -o $@

## Process second pass:
process_second_pass.o: process_second_pass.c process_second_pass.h $(GLOBAL_DEPS)
	$(CC) -c process_second_pass.c $(CFLAGS) -o $@

## Instructions helper functions:
instructions.o: instructions.c instructions.h $(GLOBAL_DEPS)
	$(CC) -c instructions.c $(CFLAGS) -o $@

## Table operations:
table_operations.o: table_operations.c table_operations.h $(GLOBAL_DEPS)
	$(CC) -c table_operations.c $(CFLAGS) -o $@

## Useful functions:
utils_functions.o: utils_functions.c instructions.h $(GLOBAL_DEPS)
	$(CC) -c utils_functions.c $(CFLAGS) -o $@

## Output Files:
writefiles.o: writefiles.c writefiles.h $(GLOBAL_DEPS)
	$(CC) -c writefiles.c $(CFLAGS) -o $@

# Clean Target (remove leftovers)
clean:
	rm -rf *.o
