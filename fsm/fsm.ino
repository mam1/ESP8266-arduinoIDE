#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include <printf.h>

#define MAX_COMMAND_SIZE 	100
#define CMD_TYPES			9

// #define CMD_TYPES  	10
// #define MAX_COMMAND_SIZE	15

// /* command parser fsm */
// #define _CMD_TOKENS     		41
// #define _CMD_STATES     		35

// /* key codes */
// #define _ESC        27
// #define _SPACE      32
// #define _COLON      58
// #define _SLASH      47
// #define _COMMA      44
// #define _BS         8
// #define _DEL		127
// #define _QUOTE      34
// #define _CR         13
// #define _NL 		10
// #define _FF 		12
// #define _EOF		0
// #define _NO_CHAR    255
// #define _DELIMITER	28
// #define _UPA		124


char 				*message;

int command_type(char* cmd) {



}
/* convert tex to float */
float t_to_f(char *ptr) {
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


int get_command_type(char *sptr) {
	char  			*index;
	int 			i;

	/* command list */
	char 	command[MAX_COMMAND_SIZE + 1], *cmd_ptr;
	char    *keyword[CMD_TYPES] = {
		/*  0 */    "temperature",
		/*  1 */    "humidity",
		/*  2 */    "on",
		/*  3 */    "off",
		/*  4 */    "h_low",
		/*  5 */    "h_high",
		/*  6 */    "t_low",
		/*  7 */    "t_high",
		/*  8 */	"auto"
	};

	for(i=0;i<CMD_TYPES;i++){
		Serial.printf("testing for <%s>\n",keyword[i]);
		index = strstr(sptr, keyword[i]);

		if(index != NULL){
			Serial.printf("index-> <%c>\n", *index);
			return i;
		}
	}

	return -1;
}

// /***************************************/
// /**  character input parser fsm start **/
// /***************************************/

// /* fsm support functions */
// int char_type(char);
// TQ *process_buffer(void);

// /* fsm fuctions */
// int nop(char *);	//do nothing
// int del(char *);	//remove character from input buffer
// int add(char *);	//add character to input buffer
// int aqd(char *);	//add quote + delim to input buffer
// int adq(char *);	//delim  + quote to input buffer
// //int dlm(char *);	//add delimiter to buffer
// int cr(char *);		//process input buffer, reset char_fsm
// int cr2(char *);	//remove trailing delimiter, process buffer, reset char_fsm
// int snul(char *);

// /* message processor action table */

// CHAR_ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {
// 	/* DELIM */{nop,  add,  add,  nop},
// 	/* QUOTE */{add,  aqd,  adq,  add},
// 	/*   DEL */{del,  del,  del,  del},
// 	/*    CR */{cr,  cr,   cr,   cr2},
// 	/* OTHER */{add,  add,  add,  add}
// };

// /* character processor state transition table */
// int char_new_state[_CHAR_TOKENS][_CHAR_STATES] = {

// 	/* DELIM */{ 0, 1, 3, 3},
// 	/* QUOTE */{ 1, 3, 1, 1},
// 	/*   DEL */{ 0, 1, 2, 3},
// 	/*    CR */{ 0, 0, 0, 0},
// 	/* OTHER */{ 2, 1, 2, 2}
// };

/*****************************************************/
/****  character input parser state machine end  *****/
/*****************************************************/


// int parce(char *payload) {
// 	int 		i;

// 	/* command list */
// 	char 	*start_command = "::";
// 	char 	command[MAX_COMMAND_SIZE + 1], *cmd_ptr;
// 	char    *keyword[CMD_TYPES] = {
// 		/*  0 */    "temperature",
// 		/*  1 */    "humidity",
// 		/*  2 */    "on",
// 		/*  3 */    "off",
// 		/*  4 */    "h_low",
// 		/*  5 */    "h_high",
// 		/*  6 */    "t_low",
// 		/*  7 */    "t_high"
// 	};

// 	cmd_ptr = command;
// 	ptr = strstr((char*)payload, start_command);
// 	if (ptr != NULL) {
// 		ptr += strlen(start_command);
// 		while (*ptr = ' ') ptr++;  	// eat white space
// 		while ((*ptr != \0) | (*ptr != ' ')) *cmd_ptr++ = ptr++; // extract command
// 		if (cmd_ptr > command){
// 			*cmd_ptr = '\0';
// 			Serial.print("command >")
// 			Serial.print(command);
// 			switch (command_type(command)) {
// 			  case 0:	// temperature
// 			  	Serial.println("invalid command");
// 			    break;
// 			  case 1:	// humidity
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 2:	// on
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 3:	// off
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 4:	// h_low
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 5:	// h_high
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 6:	// t_low
// 			    Serial.println("invalid command");
// 			    break;
// 			  case 7:	// t_ high
// 			    Serial.println("invalid command");
// 			    break;
// 			  default:
// 			  	Serial.println("invalid command");
// 			    break;
// 			}



// 		}



// 		for (int i = 0; i < 6; i++)
// 			convert += *ptr++;
// 		convert.toCharArray(char1, convert.length() + 1);
// 		humid = atof(char1);
// 		Serial.println(humid);
// 		display.printf("humidity %2.1f\n", humid);
// 		display.display();
// 	}





// 	for (i = 0; i < CMD_TYPES; i++) {
// 		if (strlen(cmd) == strlen(keyword[i])) {
// 			p = cmd;
// 			while (*p != '\0') {
// 				*p = tolower(*p);
// 				p++;
// 			};
// 			if (strncmp(cmd, keyword[i], strlen(cmd)) == 0)
// 				return i;
// 		}
// 		return i;
// 	}

// find ::

// extract Command

void setup() {



	Serial.begin(115200);                 // start the serial interface
	delay(1000);
	Serial.println(" ");
	Serial.println(" ");
	Serial.println("starting");
	

}

void loop() {
	char    command[MAX_COMMAND_SIZE];
	int 	command_type;

	strcpy(command, "h_high 45.23");
	Serial.printf("input string <%s>\n",command);

	
	command_type = get_command_type(command);
	Serial.printf("command type %i found\n",command_type);
	switch (command_type(command)) {
	  // case 0:	// temperature
	  // 	Serial.println("invalid command");
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
	  // case 6:	// t_low
	  //   Serial.println("invalid command");
	  //   break;
	  // case 7:	// t_ high
	  //   Serial.println("invalid command");
	    // break;
	  case 9:	// auto
	    Serial.println("invalid command");
	    break;
	  default:
	  	Serial.println("invalid command");
	    break;
	}

	delay(20000);

	Serial.println("looping");
}


