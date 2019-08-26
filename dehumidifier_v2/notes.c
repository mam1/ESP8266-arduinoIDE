// 258thomas/shop/dryer/controller
// 258thomas/shop/dryer/sensor

// 258thomas/shop/dryer/controller

#define SUB_TOPIC_1		"58thomas/shop/dryer/sensor"
#define SUB_TOPIC_2		"258thomas/shop/dryer/controller"
#define PUB_TOPIC		"258thomas/shop/dryer/controller"


oid callback(char* topic, byte* payload, unsigned int length)

switch(topic){
	case SUB_TOPIC_1:	// process a command
		process_command(payload, length);
		break;
	case SUB_TOPIC_2:	// process a humidity reading
		process_humidity_reading(payload, length);
		break;
	}

process_command(byte* payload, unsigned int length){
	char* 			ptr;

	ptr = (char*)payload;

	return;
}


process_humidity_reading(byte* payload, unsigned int length){
	char* 			ptr;

	ptr = (char*)payload;
	return;
}