#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CMD_TYPES  	10
#define MAX_COMMAND_SIZE	15


int command_type(char* cmd){



}

float t_to_f(char *ptr){
	int 			i;
	String 			convert = "";
  	char          	char1[8];
  	float 			converted;

	for (int i = 0; i < 6; i++)
		convert += *ptr++;
	convert.toCharArray(char1, convert.length() + 1);
	converted = atof(char1);
	return converted;

}
int parce(char *payload) {
	int 		i;

	/* command list */
	char 	*start_command = "::";
	char 	command[MAX_COMMAND_SIZE + 1], *cmd_ptr;
	char    *keyword[CMD_TYPES] = {
		/*  0 */    "temperature",
		/*  1 */    "humidity",
		/*  2 */    "on",
		/*  3 */    "off",
		/*  4 */    "h_low",
		/*  5 */    "h_high",
		/*  6 */    "t_low",
		/*  7 */    "t_high"
	};

	cmd_ptr = command;
	ptr = strstr((char*)payload, start_command);
	if (ptr != NULL) {
		ptr += strlen(start_command);
		while (*ptr = ' ') ptr++;  	// eat white space
		while ((*ptr != \0) | (*ptr != ' ')) *cmd_ptr++ = ptr++; // extract command
		if (cmd_ptr > command){
			*cmd_ptr = '\0';
			Serial.print("command >")
			Serial.print(command);
			switch (command_type(command)) {
			  case 0:	// temperature
			  	Serial.println("invalid command");
			    break;
			  case 1:	// humidity
			    Serial.println("invalid command");
			    break;
			  case 2:	// on 
			    Serial.println("invalid command");
			    break;
			  case 3:	// off
			    Serial.println("invalid command");
			    break;
			  case 4:	// h_low
			    Serial.println("invalid command");
			    break;
			  case 5:	// h_high
			    Serial.println("invalid command");
			    break;
			  case 6:	// t_low
			    Serial.println("invalid command");
			    break;
			  case 7:	// t_ high
			    Serial.println("invalid command");
			    break;			  
			  default:
			  	Serial.println("invalid command");
			    break;
			}



		}



		for (int i = 0; i < 6; i++)
			convert += *ptr++;
		convert.toCharArray(char1, convert.length() + 1);
		humid = atof(char1);
		Serial.println(humid);
		display.printf("humidity %2.1f\n", humid);
		display.display();
	}





	for (i = 0; i < CMD_TYPES; i++) {
		if (strlen(cmd) == strlen(keyword[i])) {
			p = cmd;
			while (*p != '\0') {
				*p = tolower(*p);
				p++;
			};
			if (strncmp(cmd, keyword[i], strlen(cmd)) == 0)
				return i;
		}
		return i;
	}

// find ::

// extract Command