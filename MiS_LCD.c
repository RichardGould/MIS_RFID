/********************************************************************************
 *                                                                              *
 *   MiS Suite                                                                  *
 *                                                                              *
 *   MiS_LCD.c      -    Control Contrast of LCD                                *
 *                                                                              *
 *   Description    -    Command line instruction to dim/brighten LCD           *
 *                       The LCD is the visual display of an identity           *
 *                       tag system commissioned by MiS Bedford                 *
 *                       The tag reader communicates via a MQTT broker          *
 *                       on a Raspberry Pi                                      *
 *                                                                              *
 *   Command Line   -    MiS_LCD -0                Display dims                 *
 *                  -    MiS_LCD -1                Display brightens            *
 *                                                                              *
 *   Author         -    Richard Gould                                          *
 *   Creation       -    December 2024                                          *
 *   Version        -    1.00                                                   *
 *   Last Edit      -    22/12/2024 12:00                                       *
 *   Change History                                                             *
 *        1.00      -    Creation                                               *
 *                                                                              *
 *                                                                              *
 ********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/MQTTClient.h"
#include <unistd.h>
#include "MiS.h"

#define RETAINED 0

char PUB_TOPIC[ 40 ], PAYLOAD[ 40 ];

void fn_usage( void )
{
	printf( "\nMiS RFID project - LCD control\n\n" );
	printf( "Usage : MiS_RFID_LCD [-0] [-1] -v\n" );
	exit( -1 );
}

int main( int argc, char *argv[] )
{
        int c, my_mode, my_verb;
/*
 *      process command line
 */
        while ( ( c = getopt ( argc, argv, "01v" ) ) != -1 )
        switch ( c )
        {
                case '0':       my_mode = 0; break;
                case '1':       my_mode = 1; break;
                case 'v':       my_verb = 1; break;
                default:        fn_usage(); break;
        }
	int rc;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	// Create the MQTT client
	if ( my_verb == 1 ) printf( "Creating MQTT client\n" );
	rc = MQTTClient_create( &client, MQTT_ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL );
	if ( rc != MQTTCLIENT_SUCCESS )
	{
       		printf( "Failed to create client, return code %d\n", rc );
		exit( EXIT_FAILURE );
	}

	// Configure connection options
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	// Connect to the MQTT broker
	if ( my_verb == 1 ) printf( "Connecting to MQTT broker\n" );
	rc = MQTTClient_connect( client, &conn_opts );
	if ( rc != MQTTCLIENT_SUCCESS )
	 {
		printf( "Failed to connect, return code %d\n", rc );
		MQTTClient_destroy( &client );
		exit( EXIT_FAILURE );
	}

	memset( PUB_TOPIC, 0, sizeof( PUB_TOPIC ) );
	strcat( PUB_TOPIC, TOPIC );
	strcat( PUB_TOPIC, "/" );
	strcat( PUB_TOPIC, LCD_TOPIC );
	if ( my_mode == 0 ) strcpy( PAYLOAD, "OFF" );
	if ( my_mode == 1 ) strcpy( PAYLOAD, "ON" );
	if ( my_verb == 1 ) printf( "Initialising MQTT Payload\n" );
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = PAYLOAD;
	pubmsg.payloadlen = strlen( PAYLOAD );
	pubmsg.qos = QOS;
	pubmsg.retained = RETAINED;

	MQTTClient_deliveryToken token;

	if ( my_verb == 1 ) printf( "Publish Topic = %s , Payload = %s\n", PUB_TOPIC, pubmsg.payload );	
	
	rc = MQTTClient_publishMessage( client, PUB_TOPIC, &pubmsg, &token );
	MQTTClient_waitForCompletion(client, token, TIMEOUT);

	if ( rc != MQTTCLIENT_SUCCESS )
	{
		printf( "Failed to publish message, return code %d\n", rc );
		MQTTClient_disconnect( client, TIMEOUT );
		MQTTClient_destroy( &client );
		exit( EXIT_FAILURE );
    	}
	if ( my_verb == 1 ) printf( "Payload published.\n" );
}
