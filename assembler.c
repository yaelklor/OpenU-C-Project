#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "writefiles.h"
#include "utils.h"
#include "first_pass.h"
#include "second_pass.h"
#include "process_macros.h"

/**
 * Processes a single assembly source file, and returns the result status.
 * @param filename The filename, without it's extension
 * @return Whether succeeded
 */
static bool process_file(char *filename);

/**
 * Entry point - 24bit assembler. Assembly language specified in booklet.
 */
int main(int argc, char *argv[]) {
	int i;
	/* To break line if needed */
	bool succeeded = TRUE;
	/* Process each file by arguments */
	for (i = 1; i < argc; ++i) {
		/* If last process failed and there's another file, break line: */
		if (!succeeded) puts("");
		/* Foreach argument (file name), send it for full processing. */
		succeeded = process_file(argv[i]);
		/* Line break if failed */
	}
	return 0;
}

static bool process_file(char *filename) {
	/* Memory address counters */
	int temp_c;
	long ic = IC_INIT_VALUE, dc = 0, icf, dcf;
	bool is_success = TRUE; /* Is succeeded so far */
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH + 2]; /* Temporary string for storing line, read from file */
	FILE *file_des; /* Current assembly file descriptor to process */
	long data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
	machine_word *code_img[CODE_ARR_IMG_LENGTH];
	/* Our symbol table */
	table symbol_table = NULL;
	/* Our mcro lines table */
	macro_table mcro_table = NULL;
	/* Our lines after mcro processed table */
	macro_table processed_mcro_lines_table = NULL; 
	bool *found_macro = FALSE; /* If found macro: flag will turn to TRUE, flag will turn FALSE once mcro lines added to table */
	char *macro_name[MAX_LINE_LENGTH]; /* If found macro: macro_name=current macro name, macro_name=\0 once mcro lines added to table */
	line_info curr_line_info;

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
	curr_line_info.file_name = input_filename;
	curr_line_info.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
	/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
	for (curr_line_info.line_number = 1; fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; curr_line_info.line_number++) {
		/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
		if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
			/* Print message and prevent further line processing, as well as second pass.  */
			printf_line_error(curr_line_info, "Line too long to process. Maximum line length should be %d.", MAX_LINE_LENGTH);
			is_success = FALSE;
			/* skip leftovers */
			do {
				temp_c = fgetc(file_des);
			} while (temp_c != '\n' && temp_c != EOF);
		} else {
			if (!process_line_macros(curr_line_info, &mcro_table, &processed_mcro_lines_table, &found_macro, macro_name)) {
				if (is_success) {
					is_success = FALSE;
				}
			}
		}
	}

	/* write processed_mcro_lines_table to .am file */
	if (is_success) {
		/* Concat extensionless filename with .am extension */
		is_success= write_macro_table_to_file(processed_mcro_lines_table, filename, ".am");
	}

	/* if proccess mcro lines succeeded and .am file created successfully: start the first pass */
	if (is_success) {
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
		curr_line_info.file_name = input_filename;
		curr_line_info.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
		/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
		for (curr_line_info.line_number = 1; fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; curr_line_info.line_number++) {
			/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
			if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
				/* Print message and prevent further line processing, as well as second pass.  */
				printf_line_error(curr_line_info, "Line too long to process. Maximum line length should be %d.", MAX_LINE_LENGTH);
				is_success = FALSE;
				/* skip leftovers */
				do {
					temp_c = fgetc(file_des);
				} while (temp_c != '\n' && temp_c != EOF);
			} else {
				if (!process_line_fpass(curr_line_info, &ic, &dc, code_img, data_img, &symbol_table)) {
					if (is_success) {
						icf = -1;
						is_success = FALSE;
					}
				}
			}
		}

		/* Save ICF & DCF */
		icf = ic;
		dcf = dc;
	}

	/* if first pass didn't fail, start the second pass */
	if (is_success) {
		ic = IC_INIT_VALUE;

		/* Now let's add IC to each DC for each of the data symbols in table */
		add_value_to_type(symbol_table, icf, DATA_SYMBOL);

		rewind(file_des); /* Start from beginning of file again */

		for (curr_line_info.line_number = 1; !feof(file_des); curr_line_info.line_number++) {
			int i = 0;
			fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */
			MOVE_TO_NOT_WHITE(temp_line, i)
			if (code_img[ic - IC_INIT_VALUE] != NULL || temp_line[i] == '.')
				is_success &= process_line_spass(curr_line_info, &ic, code_img, &symbol_table);
		}
			printf("is_success after second pass %d\n", is_success);
			/* Write files if second pass succeeded */
			if (is_success) {
				/* Everything was done. Write to *filename.ob/.ext/.ent */
				is_success = write_output_files(code_img, data_img, icf, dcf, filename, symbol_table);
			}
	}

	/* Now let's free some pointer: */
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

	/* return whether every assembling succeeded */
	return is_success;
}
