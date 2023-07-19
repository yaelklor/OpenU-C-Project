/* Implements a dynamically-allocated symbol/macro table */
#ifndef _TABLE_OPERATIONS_H
#define _TABLE_OPERATIONS_H
#include "globals.h"

/** A symbol type */
typedef enum symbol_type {
	CODE_SYMBOL,
	DATA_SYMBOL,
	EXTERNAL_SYMBOL,
	/** Address that contains a reference to (a usage of) external symbol */
	EXTERNAL_REFERENCE,
	ENTRY_SYMBOL
} symbol_type;

/** pointer to table entry is just a table. */
typedef struct entry* table;

/** pointer to table entry is a macro_table. */
typedef struct macro_entry* macro_table;

/** A single macro table entry */
typedef struct macro_entry {
	/** Next entry in table */
	macro_table next;
	/** Line info value */
	char *value;
	/** Key (macro name) is a string (aka char*) */
	char *key;
} table_macro_entry;

/** A single table entry */
typedef struct entry {
	/** Next entry in table */
	table next;
	/** Address of the symbol */
	long value;
	/** Key (symbol name) is a string (aka char*) */
	char *key;
	/** Symbol type */
	symbol_type type;
} table_entry;

/**
 * Adds an macro item to the table.
 * @param tab A pointer to the macro table
 * @param key The key of the entry to insert
 * @param value The value of the entry to insert
 */
void add_table_macro_item(macro_table *tab, char *key, char *value);

/**
 * Adds an item to the table, keeping it sorted.
 * @param tab A pointer to the table
 * @param key The key of the entry to insert
 * @param value The value of the entry to insert
 * @param type The type of the entry to insert
 */
void add_table_item(table *tab, char *key, long value, symbol_type type);

/**
 * Deallocates all the memory required by the macro table.
 * @param tab The macro table to deallocate
 */
void free_macro_table(macro_table tab);

/**
 * Deallocates all the memory required by the table.
 * @param tab The table to deallocate
 */
void free_table(table tab);

/**
 * Adds the value to add into the value of each entry
 * @param tab The table, containing the entries
 * @param to_add The value to add
 * @param type The type of symbols to add the value to
 */
void add_value_to_type(table tab, long to_add, symbol_type type);

/**
 * Returns all the entries by their macro name in a new table
 * @param tab The table
 * @param macro_name The macro_name to look for
 * @return A new table, which contains the entries
 */
macro_table filter_table_by_macro_name(macro_table *tab, char* macro_name);

/**
 * Returns all the entries by their type in a new table
 * @param tab The table
 * @param type The type to look for
 * @return A new table, which contains the entries
 */
table filter_table_by_type(table tab, symbol_type type);

/**
 * Printing the table entries, function for debugging
 */
void print_table(macro_table tab);

/**
 * Find entry from the only specified types
 * @param tab The table
 * @param symbol_count The count of given types
 * @param ... The types to filter
 * @return The entry if found, NULL if not found
 */
table_entry *find_by_types(table tab,char *key, int symbol_count, ...);

#endif
