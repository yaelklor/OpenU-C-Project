/* Output files related functions */
#ifndef _WRITE_FILES_H
#define _WRITE_FILES_H
#include "globals.h"
#include "table_operations.h"

/**
 * Writes the output files of a single assembled file
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename (without the extension)
 * @param ent_table The entries table
 * @param ext_table The external references table
 * @return Whether succeeded
 */
int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename,
                       table symbol_table);

/**
 * Writes a proceeded macro lines (after proceeded the macro lines) table to a file.
 * @param tab The macro table to write
 * @param filename The filename without the extension
 * @param file_extension The extension of the file, including dot before
 * @return Whether succeeded
 */
bool write_macro_table_to_file(macro_table tab, char *filename, char *file_extension);

#endif
