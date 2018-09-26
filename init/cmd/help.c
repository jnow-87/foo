#include <stdio.h>
#include <cmd/cmd.hash.h>


/* macros */
#define MAX_LINE_LEN	80


/* global functions */
int help(int argc, char **argv){
	size_t i,
		   line_len,
		   len;
	cmd_t const *cmd;



	line_len = 0;

	for(i=MIN_HASH_VALUE; i<=MAX_HASH_VALUE; ++i){
		cmd = wordlist + i;

		if(cmd->name != 0 && cmd->name[0] != '\0' && cmd->exec != 0x0){
			len = strlen(cmd->name) + 1;

			if(line_len + len > MAX_LINE_LEN){
				printf("\n");
				line_len = 0;
			}

			line_len += len;

			printf("%s\t", cmd->name);
		}
	}

	printf("\n");

	return 0;
}
