/********************************************************************************
 *                                                                              *
 *   MiS Suite                                                                  *
 *                                                                              *
 *   MiS_MSG.c      -    Messaging system between base and tag reader           *
 *                                                                              *
 *   Description    -    Command line instruction to modify either the          *
 *                       title or the status line of an LCD within the          *
 *                       tag system commissioned by MiS Bedford                 *
 *                       The tag reader communicates via a MQTT broker          *
 *                       on a Raspberry Pi                                      *
 *                                                                              *
 *   Command Line   -    MiS_MSG -v -m <Message>              All Device Title  *
 *                  -    MiS_MSG -v -d <Device> -m <Message>  Individual Status *
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
#include "MQTTClient.h"
#include <unistd.h>
#include "MiS.h"

char PUB_TOPIC[ 40 ], PAYLOAD[ 40 ], DEVICE[ 40 ];
char * my_message;

void fn_usage( void )
{
	printf( "\nMiS RFID project - MSG control\n\n" );
	printf( "Usage : MiS_MSG -m <message> [-d] <device> -v\n" );
	exit( -1 );
}

int main( int argc, char *argv[] )
{
        int c, my_mode, my_verb;
/*
 *      process command line
 */
        while ( ( c = getopt ( argc, argv, "d:m:v" ) ) != -1 )
        switch ( c )
        {
                case 'm':       my_message = optarg; break;
                case 'd':       strcpy( DEVICE, optarg ); break;
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
	if ( DEVICE[ 0 ] != '\0' )
	{
		printf( "We get here\n" );
		strcat( PUB_TOPIC, "/" );
		strcat( PUB_TOPIC, DEVICE );
	}
	strcat( PUB_TOPIC, MSG_TOPIC );
	strcpy( PAYLOAD, my_message );
	if ( my_verb == 1 ) printf( "Initialising MQTT Payload\n" );
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = PAYLOAD;
	pubmsg.payloadlen = strlen( PAYLOAD );
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

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
