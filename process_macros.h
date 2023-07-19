#ifndef _PROCESS_MACROS_H
#define _PROCESS_MACROS_H
/* Processes a code line in first pass */
#include "globals.h"

/**
 * Processes a single line in the first pass
 * @param line The line text
 * @param datas The data symbol table
 * @param codes The code symbol table
 * @param externals The externals symbol table
 * @param IC A pointer to the current instruction counter
 * @param DC A pointer to the current data counter
 * @param code_img The code image array
 * @param data_img The data image array
 * @return Whether succeeded.
 */
bool process_line_macros(line_info line, macro_table *mcro_table, macro_table *processed_mcro_lines_table, bool *found_macro, char *macro_name);

#endif
