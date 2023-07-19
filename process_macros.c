#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "code.h"
#include "utils.h"
#include "instructions.h"
#include "table.h"
#include "process_macros.h"


bool process_line_macros(line_info line, macro_table *mcro_table, macro_table *processed_mcro_lines_table, bool *found_macro, char *macro_name){
	char word[MAX_LINE_LENGTH];
	char symbol[MAX_LINE_LENGTH];
	macro_table current_macro_lines_table = NULL;
	opcode curr_opcode;
	int i;
	i=0;
	MOVE_TO_NOT_WHITE(line.content, i);
	/* found macro on prev steps which means this line is part of macro block. need to add this line to mcro_table with macro_name for future use*/
	if(*found_macro)
	{
		/* find first word of the line, store on word */
		find_next_word(line,word,&i);
		/* if word is not mcro closing word, add the line to mcro_table*/
		if(!is_macro_closing(word))
		{
			add_table_macro_item(mcro_table, macro_name, line.content);
			printf("printing table mcro_table\n");
			print_table(*mcro_table);
			return TRUE;
		}
		/*is macro closing.. reset found_macro, macro_name */
		*found_macro=FALSE;
		macro_name[0]='\0';
		return TRUE;
	}
	/* empty/comment line, add line to processed_mcro_lines_table */
	if (!line.content[i] || line.content[i] == '\n' || line.content[i] == EOF || line.content[i] == ';')
	{
		/* macro_name is \0. from now on lines that are not in macro block will be added to table with empty key. key is not relevant for anything */
		add_table_macro_item(processed_mcro_lines_table, macro_name, line.content);
		return TRUE;
	}
	/*line starting with label, no chance for macro line*/
	if (find_label(line, symbol) && symbol[0] != '\0') {
		/* macro_name is \0. from now on lines that are not in macro block will be added to table with empty key. key is not relevant for anything */
		add_table_macro_item(processed_mcro_lines_table, macro_name, line.content);
		return TRUE;
	}
	/* if line is not empty\comment, not label and line is not part of macro block. find first word and figure if macro opening\macro name\opcode\instruction */
	find_next_word(line,word,&i);
	
	if(is_macro_opening(word)){
		word[0]='\0';
		/* found macro opening word, need to find macro name */
		find_next_word(line,word,&i);
		if(word[0]=='\0'){
			 printf("Error: no macro name defined!\n");
			 return FALSE;
		}
		*found_macro=TRUE;
		strcpy(macro_name,word);
		return TRUE;
	}

	if(find_instruction_by_name(word+1) && !is_int(word+1)){
	 	/* we found instruction. */
		add_table_macro_item(*processed_mcro_lines_table, macro_name, line.content);
		return TRUE;
	}

	get_opcode_func(word, &curr_opcode);
	/* Check if we found opcode. */
	if (curr_opcode != NONE_OP) {
		add_table_macro_item(*processed_mcro_lines_table, macro_name, line.content);
		return TRUE; 
	}
	
	/* If we got here, we found line with macro name! */
	/* need to filter mcro_table for relevant macro block lines and store on tmp current_macro_lines table*/
	current_macro_lines_table= filter_table_by_macro_name(mcro_table, word);

	/* copy lines of current_macro_lines table to processed_mcro_lines table */
	do{
		add_table_macro_item(processed_mcro_lines_table, macro_name,  current_macro_lines_table->value); 
	}while((current_macro_lines_table = current_macro_lines_table->next) != NULL);
	  
	return TRUE;
}
