#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils_functions.h"
#include "table_operations.h"

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char base_64_code_data_img[2]="";

/**
 * Writes the code and data image into an .ob file, with lengths on top
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename, without the extension
 * @return Whether succeeded
 */
static bool write_ob_file(machine_word **code_img, long *data_img, long icf, long dcf, char *filename);

/**
 * Writes a symbol table to a file. Each symbol and it's address in line, separated by a single space.
 * @param tab The symbol table to write
 * @param filename The filename without the extension
 * @param file_extension The extension of the file, including dot before
 * @return Whether succeeded
 */
static bool write_table_to_file(table tab, char *filename, char *file_extension);

/**
 * Creates unsigned mask from requested range of bits
 * @param a low bit
 * @param b high bit
 * @return Whether succeeded
 */
static unsigned create_mask(unsigned a, unsigned b);


int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename, table symbol_table) {
	table externals=NULL, entries=NULL;
	bool result;
	printf("in write_output_files func \n");
	if(symbol_table){
		externals = filter_table_by_type(symbol_table, EXTERNAL_REFERENCE);
		entries = filter_table_by_type(symbol_table, ENTRY_SYMBOL);}
		/* Write .ob file */
		result = write_ob(code_img, data_img, icf, dcf, filename);
		/* Write *.ent and *.ext files: call with symbols from external references type or entry type only */
		if(externals){result=write_table_to_file(externals, filename, ".ext");
		free_table(externals);}
		if(entries){write_table_to_file(entries, filename, ".ent");
		free_table(entries);
	}

	return result;
}

static unsigned create_mask(unsigned a, unsigned b)
{
   unsigned r = 0, i = 0;
   for (i=a; i<=b; i++)
       r |= 1 << i;

   return r;
}

static bool write_ob_file(machine_word **code_img, long *data_img, long icf, long dcf, char *filename) {
	int i;
	long val;
	FILE *file_desc;
	unsigned first_6_bits, second_6_bits, first_12_bits;
	/* add extension of file to open */
	char *output_filename = strallocat(filename, ".ob");
	/* Try to open the file for writing */
	file_desc = fopen(output_filename, "w");
	free(output_filename);
	if (file_desc == NULL) {
		printf("Error: Can't create or rewrite to file %s.", output_filename);
		return FALSE;
	}

	/* print data/code word count on top */
	fprintf(file_desc, "%ld %ld\n", icf - IC_INIT_VALUE, dcf);

	/* starting from index 0, not IC_INIT_VALUE as icf, so we have to subtract it. */
		for (i = 0; i < icf - IC_INIT_VALUE; i++) {
		if (code_img[i]->length > 0) {
			val = (code_img[i]->word.code->opcode << 5) | (code_img[i]->word.code->src_addressing << 9) |
			      (code_img[i]->word.code->dest_addressing << 2) | (code_img[i]->word.code->ARE);
				printf("code_img[%d] is \n", i);
				printBinaryValue(val);
				printf("\n");
				first_6_bits = create_mask(0,5);
				second_6_bits = create_mask(6,11);
				first_6_bits = first_6_bits & val;
				second_6_bits = (second_6_bits & val)>>6;
				printf("first_6_bits in int %d\n", (int)first_6_bits);
				printf("second_6_bits in int %d\n", (int)second_6_bits);
				printf("base64_table[(int)first_6_bits] is:%c\n", base64_table[(int)first_6_bits]);
				printf("base64_table[(int)second_6_bits] is:%c\n", base64_table[(int)second_6_bits]);
		} else {
			/* We need to cut the value, keeping only it's 12 lsb, and include the ARE in the whole party as well: */
			first_12_bits=create_mask(0,12);
			val = ((first_12_bits & code_img[i]->word.data->data) << 2) | (code_img[i]->word.data->ARE);
			printf("code_img[%d] is \n", i);
				printBinaryValue(val);
				printf("\n");
				first_6_bits = create_mask(0,5);
				second_6_bits = create_mask(6,11);
				first_6_bits = first_6_bits & val;
				second_6_bits = (second_6_bits & val)>>6;
				printf("first_6_bits in int %d\n", (int)first_6_bits);
				printf("second_6_bits in int %d\n", (int)second_6_bits);
				printf("base64_table[(int)first_6_bits] is:%c\n", base64_table[(int)first_6_bits]);
				printf("base64_table[(int)second_6_bits] is:%c\n", base64_table[(int)second_6_bits]);
		}
		/* Write the value to the file - first */
		base_64_code_data_img[0]=base64_table[(int)second_6_bits];
		base_64_code_data_img[1]=base64_table[(int)first_6_bits];
		fprintf(file_desc, "%s\n", base_64_code_data_img);
	}

	/* Write data image. dcf starts at 0 so it's fine */
	for (i = 0; i < dcf; i++) {
		/* print only lower 12 bytes */
		first_12_bits=create_mask(0,11);
		val = first_12_bits & data_img[i];
		printf("data_img[%d] is \n", i);
		printBinaryValue(data_img[i]);
		printf("\n");
		first_6_bits = create_mask(0,5);
		second_6_bits = create_mask(6,12);
		first_6_bits = first_6_bits & val;
		second_6_bits = (second_6_bits & val)>>6;
		printf("first_6_bits in int %d\n", (int)first_6_bits);
		printf("second_6_bits in int %d\n", (int)second_6_bits);
		base_64_code_data_img[0]=base64_table[(int)second_6_bits];
		base_64_code_data_img[1]=base64_table[(int)first_6_bits];
		fprintf(file_desc, "%s\n", base_64_code_data_img);
	}

	/* Close the file */
	fclose(file_desc);
	return TRUE;
}

/* function for debugging  */
void printBinaryValue(unsigned int num)
{
    if(!num) return;

    printBinaryValue(num>>1);
    putchar(((num&1 == 1) ? '1' : '0'));
}

static bool write_table_to_file(table tab, char *filename, char *file_extension) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, file_extension);
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	/* Write first line without \n to avoid extraneous line breaks */
	fprintf(file_desc, "%s %.7ld", tab->key, tab->value);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n%s %.7ld", tab->key, tab->value);
	}
	fclose(file_desc);
	return TRUE;
}

bool write_macro_table_to_file(macro_table tab, char *filename, char *file_extension){
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, file_extension);
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	while (tab != NULL) {
		fprintf(file_desc, "%s",tab->value);
		tab = tab->next;
	}
	fclose(file_desc);
	return TRUE;

}
