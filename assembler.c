#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "writefiles.h"
#include "utils.h"
#include "first_pass.h"
#include "second_pass.h"
#include "process_macros.h"

/**
 * This function processes a single assembly source file and returns whether succeeded or not.
 * @param filename The filename
 * @return Whether succeeded
 */
static bool process_source_file(char *filename);

/**
 * Entry point.
 */
int main(int argc, char *argv[]) {
	int i;
	/* Flag to break line if needed */
	bool succeeded = TRUE;
	/* Loop to process each file of arguments */
	for (i = 1; i < argc; ++i) {
		/* If last process failed and there's another file, break line: */
		if (!succeeded) puts("");
		/* Send each argument (file name) to full processing. */
		succeeded = process_source_file(argv[i]);
	}
	return 0;
}

static bool process_source_file(char *filename) {
	/* Memory address counters */
	int temp_c;
	long ic = IC_INIT_VALUE, dc = 0, icf, dcf;
	bool succeeded = TRUE; /* Is succeeded so far */
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH + 2]; /* Temp string for storing line, read from file */
	FILE *file_des; /* Current assembly file descriptor to process */
	long data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
	machine_word *code_img[CODE_ARR_IMG_LENGTH];
	/* The symbol table */
	table symbol_table = NULL;
	/* The mcro lines table */
	macro_table mcro_table = NULL;
	/* The lines after mcro processed table */
	macro_table processed_mcro_lines_table = NULL; 
	bool *found_macro = FALSE; /* If found macro: flag will turn to TRUE, flag will turn FALSE once mcro lines added to table */
	char *macro_name[MAX_LINE_LENGTH]; /* If found macro: macro_name=current macro name, macro_name=\0 once mcro lines added to table */
	line_info current_line;

	/* Concat extensionless filename with .as extension */
	input_filename = strallocat(filename, ".as");

	/* Open file, skip on failure */
	file_des = fopen(input_filename, "r");
	if (file_des == NULL) {
		/* if file couldn't be opened, write to stderr */
		printf("Error: file \"%s.as\" is inaccessible for reading. skipping it.\n", filename);
		free(input_filename); /* The only allocated space is for the full file name */
		return FALSE;
	}

	/*Start parsing macros: */
	current_line.file_name = input_filename;
	current_line.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
	/* Read line - stop if read failed (when NULL returned) - usually when EOF. 
 	increase line counter for error printing. */
	for (current_line.line_number = 1; fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; current_line.line_number++) {
		/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
		if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
			/* Print message and prevent further line processing, as well as second pass.  */
			printf_line_error(current_line, "Line too long to process. Maximum line length should be %d.", MAX_LINE_LENGTH);
			succeeded = FALSE;
			/* skip leftovers */
			do {
				temp_c = fgetc(file_des);
			} while (temp_c != '\n' && temp_c != EOF);
		} else {
			if (!process_line_macros(current_line, &mcro_table, &processed_mcro_lines_table, &found_macro, macro_name)) {
				if (succeeded) {
					succeeded = FALSE;
				}
			}
		}
	}

	/* write processed_mcro_lines_table to .am file */
	if (succeeded) {
		/* Concat extensionless filename with .am extension */
		succeeded= write_macro_table_to_file(processed_mcro_lines_table, filename, ".am");
	}

	/* if proccess mcro lines succeeded and .am file created successfully: start the first pass */
	if (succeeded) {
		input_filename = strallocat(filename, ".am");
		/* Open file, skip on failure */
		file_des = fopen(input_filename, "r");
		if (file_des == NULL) {
			/* if file couldn't be opened, write to stderr. */
			printf("Error: file \"%s.am\" was not created as expected\n", filename);
			free(input_filename); /* The only allocated space is for the full file name */
			return FALSE;
		}

		/* start first pass: */
		current_line.file_name = input_filename;
		current_line.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
		/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
		for (current_line.line_number = 1; fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; current_line.line_number++) {
			/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
			if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
				/* Print message and prevent further line processing, as well as second pass.  */
				printf_line_error(current_line, "Line too long to process. Maximum line length should be %d.", MAX_LINE_LENGTH);
				succeeded = FALSE;
				/* skip leftovers */
				do {
					temp_c = fgetc(file_des);
				} while (temp_c != '\n' && temp_c != EOF);
			} else {
				if (!process_line_fpass(current_line, &ic, &dc, code_img, data_img, &symbol_table)) {
					if (succeeded) {
						icf = -1;
						succeeded = FALSE;
					}
				}
			}
		}

		/* Save ICF & DCF */
		icf = ic;
		dcf = dc;
	}

	/* if the first pass didn't fail it's time to start the second pass */
	if (succeeded) {
		ic = IC_INIT_VALUE;

		/* Now is time to add IC to each DC for each of the data symbols in table */
		add_value_to_type(symbol_table, icf, DATA_SYMBOL);

		rewind(file_des); /* Start from beginning of file again */

		for (current_line.line_number = 1; !feof(file_des); current_line.line_number++) {
			int i = 0;
			fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */
			MOVE_TO_NOT_WHITE(temp_line, i)
			if (code_img[ic - IC_INIT_VALUE] != NULL || temp_line[i] == '.')
				succeeded &= process_line_spass(current_line, &ic, code_img, &symbol_table);
		}
			printf("succeeded after second pass %d\n", succeeded);
			/* Write files if second pass succeeded */
			if (succeeded) {
				/* Everything was done. Write to *filename.ob/.ext/.ent */
				succeeded = write_output_files(code_img, data_img, icf, dcf, filename, symbol_table);
			}
	}

	/* Now is time to free some pointers: */
	/* current file name */
	free(input_filename);
	/* Free symbol table */
	free_table(symbol_table);
	/* Free mcro table */
	free_macro_table(mcro_table);
	/* Free processed_mcro_lines table */
	free_macro_table(processed_mcro_lines_table);
	/* Free code & data buffer contents */
	free_code_image(code_img, icf);

	/* return whether processing succeeded */
	return succeeded;
}
