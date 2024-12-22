
#ifndef MiSHeader
#define MiSHeader

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <unistd.h>

#define MQTT_ADDRESS    "localhost:1883"	// MQTT broker address
#define TOPIC           "MiS"                 	// Topic to publish
#define LCD_TOPIC       "/LCD"                 	// Topic to publish
#define MSG_TOPIC       "/MSG"                 	// Topic to publish
#define RST_TOPIC       "/RST"                 	// Topic to publish
#define QOS         1                           // Quality of Service level
#define TIMEOUT     10000L                      // Timeout in milliseconds
#define TRUE        1
#define CLIENTID    "MiSRFID"

void fn_usage( void );

#endif
