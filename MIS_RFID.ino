/* Men in Sheds RFID ID Verification  */
/*
  Version:    	1.0
  Date:      	1/11/2024
  Latest Edit:  21/12/2024 15:00
*/

/* Library Includes */

#include <SPI.h>                  // needed for RFID device       v1.0
#include <MFRC522.h>              // needed for RFID device       v1.4.11
#include <ESP8266WiFi.h>          // needed for WIFi              v2.0
#include <PubSubClient.h>         // needed for MQTT              v2.0
#include <Wire.h>                 // needed for i2c               v1.0
#include <LiquidCrystal_I2C.h>    // needed for LCD 2004          v1.1.2

/*  Definition Section  */

#define VERSION         1.0
#define RST_PIN         D3
#define SS_PIN          D8

#define GREEN           D0          // Green LED
#define RED             D4          // Red LED

#define SDA             D2          // i2c bus
#define SCL             D1

#define LCD             0x27        // address of LCD2004
#define LCD_chars       20          // number of characters per line
#define LCD_lines        4          // number of lines

#define MSG_BUFFER_SIZE (50)

/*  variable instantiation  */

/*  Network Credentials */
const char * Home_WiFi = "GOULD_TP";
char Home_Pass[ 20 ]   = "pr`rxgvx";
const char * Home_MQTT = "192.168.1.180";

const char * MiS_WiFi  = "BTB-NTCHT6";
char MiS_Pass[ 20 ]    = "RHCagINtyI}`k>";
const char * MiS_MQTT  = "192.168.1.249";

const char * Jim_WiFi  = "BT-S7AT5Q";
char Jim_Pass[ 20 ]    = "vHqbS8{:YqKX@q";
const char * Jim_MQTT  = "192.168.1.165";

const char * Rich_WiFi = "GOULDWAN24";
char Rich_Pass[ 20 ]   = "Tgkf2;47cd";
const char * Rich_MQTT = "192.168.1.180";

//const char * LCD_Title = "MiS Tag Identity";
const char * MiS_TOPIC  = "MiS";
const char * MiS_DEVICE = "RFID_3";
const char * PUB_TOPIC  = "MiS/RFID_3/IN";
const char * SUB_TOPIC  = "MiS/RFID_3/OUT";
const char * MiS_MY_MSG = "MiS/RFID_3/MSG";
const char * MiS_MSG    = "MiS/MSG";
const char * MiS_RESET  = "MiS/RFID_3/RST";
const char * MiS_LCD    = "MiS/LCD";

char MQTT_server[ 20 ], my_salut[ 20 ], my_member[ 20 ];
int  MQTT_port = 1883;

char WiFi_Pass[ 20 ];
char PUB_message[ MSG_BUFFER_SIZE ], SUB_message[ MSG_BUFFER_SIZE ], MQTT_in_buffer[ MSG_BUFFER_SIZE ];
char MQTT_in_topic[ 20 ];
char Old_message[ MSG_BUFFER_SIZE ];
char my_IP_Address[ 20 ];         //  xxx.xxx.xxx.xxx
char LCD_Title[ 20 ], LCD_Ready[ 20 ];
int MQTT_in_flag, MQTT_in_length;
unsigned long epoch, pulse, payload_time;
int rtn, first, line[ LCD_lines ], count;

  int network, networks, my_network;

/*  Instance creation */

MFRC522 mfrc522( SS_PIN, RST_PIN );           	// Create MFRC522 instance

WiFiClient espClient;                 			// WiFi

LiquidCrystal_I2C lcd( LCD, LCD_chars, LCD_lines );   // LCD2004

PubSubClient client( espClient );           	// MQTT

/*	------------------------------------------------------------------------------ */

/*  Scan WiFi for networks  */
int fn_WiFiScan() {

//  int network, networks, my_network;
	WiFi.disconnect();
	delay( 2000 );
	my_network = -1;
	networks = WiFi.scanNetworks();
  delay( 1000 );
	Serial.print( "Networks seen : " );
	Serial.println( networks );
  delay( 1000 );
	if ( networks == 0 )
	{
			fn_Display( 1, 3, 0, "Restarting" );
      delay( 60000 );
		ESP.restart();
	}
	for ( network = 0; network < networks; network++ )
	{
    if ( strcmp( WiFi.SSID( network ).c_str(), Rich_WiFi ) == 0 )
		{
      strcpy( WiFi_Pass, Rich_Pass );
			strcpy( MQTT_server, Rich_MQTT );
			return ( network );
		}
		if ( strcmp( WiFi.SSID( network ).c_str(), Home_WiFi ) == 0 )
		{
      strcpy( WiFi_Pass, Home_Pass );
			strcpy( MQTT_server, Home_MQTT );
			return ( network );
		}
		if ( strcmp( WiFi.SSID( network ).c_str(), MiS_WiFi ) == 0 )
		{
      strcpy( WiFi_Pass, MiS_Pass );
			strcpy( MQTT_server, MiS_MQTT );
			return ( network );
		}
		if ( strcmp( WiFi.SSID( network ).c_str(), Jim_WiFi ) == 0 )
		{
      strcpy( WiFi_Pass, Jim_Pass );
			strcpy( MQTT_server, Jim_MQTT );
			return ( network );
		}
	}
  fn_Display( 1, 3, 0, "Restarting" );
	delay( 60000 );
	ESP.restart();
	return ( -1 );
}

/*	Connect to Wifi */
int fn_WiFi_Connect( int network )
{
	int i = 0, rtn = 0;
	fn_Display( 1, 3, 0, "Connecting" );
	delay( 2000 );
	WiFi.begin( WiFi.SSID( network ), WiFi_Pass );
  Serial.print( "Network : " );
  Serial.println( WiFi.SSID( network ) );

	while ( i < 10 )
	{
	    if ( WiFi.status() != WL_CONNECTED )
		{
			delay( 3000 );
			if ( i > 10 ) return ( -1 );
			Serial.print( "." );
		}
    i++;
	}
	Serial.println();
	fn_Clear( 2 );
	fn_Clear( 3 );
	return ( 0 );
}

/*  change LED state  */
void fn_LED( int LED, int state )
{
	if ( state == 1 ) digitalWrite( LED, LOW );
	if ( state == 0 ) digitalWrite( LED, HIGH );
	if ( state == 2 ) digitalWrite( LED, !digitalRead( LED ) );
}

/*	display title on LCD */
void fn_title( void )
{
	int i;
	for ( i = 0; i < LCD_lines; i++ )
	{
		if ( line[ i ] == 1 ) fn_Clear( i );
	}
	fn_Display( 1, 0, 0, LCD_Title );     // display title
}

/*  display message on LCD2004  */
void fn_Display( int my_centre, int my_line, int my_pos, const char * my_message )
{
	if ( my_centre == 1 ) my_pos = ( LCD_chars - strlen( my_message ) ) / 2;
	if ( line[ my_line ] == 1 )
	{
		lcd.setCursor( 0, my_line );
		lcd.print( "                   " );
	}
	lcd.setCursor( my_pos, my_line );
	lcd.print( my_message );
	line[ my_line ] = 1;            //  set line in use flag
}

/*	clear LCD line */
void fn_Clear( int cl_line )
{
	lcd.setCursor( 0, cl_line );
	lcd.print( "                     " );
	line[ cl_line ] = 0;
}

/*  MQTT callback */
void MQTT_CB( char * topic, byte * payload, uint8_t length )
{
	int i;
	memset( MQTT_in_topic,  0, sizeof( MQTT_in_topic  ) );
	memset( MQTT_in_buffer, 0, sizeof( MQTT_in_buffer ) );
  Serial.println( "Message Received" );

	MQTT_in_flag = 1;
	strcpy( MQTT_in_topic, topic );
	MQTT_in_length = length;
	for ( i = 0; i < length; i++ ) MQTT_in_buffer[ i ] = (char) payload[ i ];
	MQTT_in_buffer[ i ] = 0;

  if ( payload_time != 0 )
  {
    if ( millis() > payload_time + 10000 )
    {
      memset( Old_message, 0, sizeof( Old_message) );
      payload_time = 0;
    }
  }
	if ( strcmp( MQTT_in_buffer, Old_message ) == 0 )
	{
		MQTT_in_flag = 0;
		return;
	}
  if ( MQTT_in_buffer[ 0 ] == 0 )
	{
		MQTT_in_flag = 0;
		return;
	}
	Serial.print( "Call Back - Topic = " );
	Serial.print( MQTT_in_topic );
	Serial.print( " Payload = " );
	Serial.println( MQTT_in_buffer );

	strcpy( Old_message, MQTT_in_buffer );
	payload_time = millis();
	return;
}

/*	connect to MQTT broker */
int fn_MQTT_Connect()
{
	int i = 0, rtn = 0;
	Serial.println( "MQTT connection ..." );
	while ( i < 10 )
	{
		rtn = client.connect( MiS_TOPIC );
		if ( rtn == 1 )
		{
			fn_Display( 1, 3, 0, "MQTT Connected" );
			Serial.println( "" );
			delay( 1000 );
			fn_Clear( 3 );
			return ( 0 );
		}
		Serial.print( "." );
		i++;
	}
	Serial.println( "" );
	return ( rtn );
}

/*  ------------------------------------------------------------------------------- */

void setup() {
	int i;
	epoch = millis();                 //  start the clock
	Serial.begin( 115200 );           //  Initialize serial communications with the PC

	pinMode( GREEN, OUTPUT );         //  Green LED
	pinMode( RED, OUTPUT );           //  Red LED
	digitalWrite( GREEN, HIGH );      //  Off
	digitalWrite( RED, HIGH );        //  Off
	SPI.begin();
	lcd.init();                       //  Initialize LCD2004
	lcd.backlight();                  //  LCD2004 backlight on

	// set LCD defaults
	strcpy( LCD_Title, "MiS Tag Identity" );
	strcpy( LCD_Ready, "Ready" );

	fn_title();                       //  Title line on LCD2004

  Serial.println( "" );

	fn_Display( 1, 2, 0, "Network Search" );
	//  decrypt passwords
	
	for ( i = 0; i < strlen( Home_Pass ); i++ ) Home_Pass[ i ] = Home_Pass[ i ] + 3 - i;
	for ( i = 0; i < strlen( MiS_Pass  ); i++ ) MiS_Pass[ i ]  = MiS_Pass[ i ]  + 3 - i;
	for ( i = 0; i < strlen( Jim_Pass  ); i++ ) Jim_Pass[ i ]  = Jim_Pass[ i ]  + 3 - i;
  for ( i = 0; i < strlen( Rich_Pass  ); i++ ) Rich_Pass[ i ]  = Rich_Pass[ i ]  + 3 - i;

/*	Scan the WiFi */
	int net = -1, res = -1;
	count = 0;
	while ( net == -1 )
	{
		net = fn_WiFiScan();          //  returns network or -1
		if ( net == -1 ) Serial.println( "No WiFi" );
		delay( 5000 );
		if ( count++ > 10 ) ESP.restart();     // restart wemo
	}
	
/*	Connect to recognised WiFi  */
	count = 0;
	while ( res != 0 )
	{
		Serial.print( "Network selected : " );
		Serial.println( net );
    
		delay( 1000 );
		res = fn_WiFi_Connect( net );
		if ( res != 0 )
		{
			delay( 1000 );
			if ( count++ > 10 ) 
      {
        			fn_Display( 1, 3, 0, "Restarting" );
              ESP.restart();   //  restart Wemo
      }
		}
	}
	char my_IP_Address[ 20 ];
	strcpy(  my_IP_Address, WiFi.localIP().toString().c_str() );
	fn_Display( 1, 2, 0, "Network Found" );
	fn_Display( 1, 3, 0, my_IP_Address );
  Serial.print( "IP Address ~: " );
  Serial.println( my_IP_Address );
	delay( 3000 );
	fn_Clear( 2 );
	fn_Clear( 3 );

/*  MQTT setup */
	client.setServer( MQTT_server, MQTT_port );
	client.setCallback( MQTT_CB );
  delay( 1000 );
  client.loop();
/*  RFID setup  */
	mfrc522.PCD_Init();
	epoch = millis();
	fn_Display( 1, 2, 0, LCD_Ready );
  delay( 5000 );
	Serial.println( "Ready" );
}

/*	Determine if tag has been presented  */
int fn_RFID()
{
	int rtn = 0, i;
	char my_UID[ 10 ];

	if ( ! mfrc522.PICC_IsNewCardPresent() ) return ( 1 );
	if ( ! mfrc522.PICC_ReadCardSerial()   ) return ( 2 );

	fn_Display( 1, 2, 0, "** Tag detected **" );

	memset( my_UID, 0, sizeof( my_UID ) );
  
	sprintf( my_UID, "%02x%02x%02x%02x",
			mfrc522.uid.uidByte[0],
			mfrc522.uid.uidByte[1],
			mfrc522.uid.uidByte[2],
			mfrc522.uid.uidByte[3] );

	memset( PUB_message, 0, sizeof( PUB_message ) );
	memset( SUB_message, 0, sizeof( SUB_message ) );
  
	strcpy( PUB_message, my_UID );

	fn_Display( 1, 3, 0, my_UID );

	if ( !client.connected() ) fn_MQTT_Connect();

/*	publish <UID> to MQTTT  */
	client.publish( PUB_TOPIC, ( const uint8_t *  ) PUB_message, strlen( PUB_message ), false );

	Serial.print( "Published Topic : " );
	Serial.print( PUB_TOPIC );
	Serial.print( " Payload : " );
	Serial.println( PUB_message );

//	delay( 1000 );

	fn_LED( RED, 1 );         //  turn RED led on

	Serial.print( "Subscription : " );
	Serial.println( SUB_TOPIC );
  
	client.subscribe( SUB_TOPIC, 1 );

	fn_Display( 1, 2, 0, "** Await Resp **" );

	//epoch = millis();
	count = 0;
	while ( MQTT_in_flag == 0 )
	{
		client.loop();
		delay( 1000 );
//		unsigned now = millis();
		if ( count++ > 20 )			// wait for 20 seconds
		{
			fn_Display( 1, 2, 0, "No Response" );
			fn_LED( RED, 0 );
			delay( 5000 );
			return ( -1 );
		}
	}
	MQTT_in_flag = 0;

	if ( strcmp( MQTT_in_topic, SUB_TOPIC ) != 0 )
	{
		fn_Display( 1, 2, 0, "Bad Response" );
		fn_LED( RED, 0 );
		delay( 5000 );
		return( -1 );
	}

//		MQTT broker response 12131415:Y:W:Richard or <UID>:Y:B:Richard or <UID>:N

	if ( MQTT_in_buffer[ 9 ] == 'N' )
	{
		fn_Display( 1, 2, 0, "*** Tag Unknown ***" );
		fn_LED( RED, 0 );
		delay( 5000 );
		return ( -1 );
	}

	if ( MQTT_in_buffer[ 11 ] == 'I' or MQTT_in_buffer[ 11 ] == 'W' )
	{
		sprintf( my_salut, "%s ", "Hi : " );
	} else {
		sprintf( my_salut, "%s ", "Bye: " );
	}
	memset( my_member, 0, sizeof( my_member ) );
	
	for ( i = 13; i < MQTT_in_length; i++ ) my_member[ ( i - 13 ) ] = MQTT_in_buffer[ i ];

	strcat( my_salut, my_member );
	fn_Clear( 3 );
	fn_Display( 1, 2, 0, my_salut );

	fn_LED( RED, 0 );       // turn RED led off

	fn_LED( GREEN, 1 );     // turn GREEN led on

	client.unsubscribe( SUB_TOPIC );
	delay( 10000 );         // wait for 10 seconds
	fn_LED( GREEN, 0 );     // turn GREEN led off
	return ( -1 );
}

void loop() {
	pulse = millis();
	if ( line[ 2 ] == 0 ) fn_Display( 1, 2, 0, "** Ready **" );

	if ( first == 0 || pulse > ( epoch ) + ( 10 * 60 * 1000 )	)		//  10 minutes
	{
	  first = 1;
    Serial.println( "Subscribing to other topics" );
    if ( !client.connected() ) fn_MQTT_Connect();
		client.subscribe( MiS_MY_MSG, 1 );      // message from broker for me
//    delay( 2000 );
		client.subscribe( MiS_MSG, 1 );         // global message
//    delay( 2000 );
    client.subscribe( MiS_RESET, 1 );       // reset wemo
//    delay( 2000 );
    client.subscribe( MiS_LCD, 1 );		 // LCD Commands
//    delay( 2000 );
    epoch = millis();
		//fn_title();									//  refresh screen every 10 minutes
	}

	if ( ! client.connected() ) fn_MQTT_Connect();

	rtn = fn_RFID();				// test for tag
	if ( rtn < 0 ) fn_title();

	if ( MQTT_in_flag == 1 )
	{
		MQTT_in_flag = 0;
		if ( strcmp( MQTT_in_topic, MiS_MY_MSG ) == 0 )
		{
			Serial.print( "Incoming message for me : " );
			Serial.println( MQTT_in_buffer );
			MQTT_in_flag = 0;
			fn_Clear( 2 );
			fn_Display( 1, 2, 0, MQTT_in_buffer );
			strcpy( LCD_Ready, MQTT_in_buffer );
		}
		if ( strcmp( MQTT_in_topic, MiS_MSG ) == 0 )
		{
			Serial.print( "Incoming message for all : " );
			Serial.println( MQTT_in_buffer );
			MQTT_in_flag = 0;
			fn_Clear( 0 );
			fn_Display( 1, 0, 0, MQTT_in_buffer );
			strcpy( LCD_Title, MQTT_in_buffer );
		}
		if ( strcmp( MQTT_in_topic, MiS_RESET ) == 0 )
		{
			if ( strcmp( MQTT_in_buffer, "Yes" ) == 0 )
			{
				Serial.println( "Resetting Wemo ");
        fn_Display( 1, 3, 0, "Restarting" );
				ESP.restart();
			}
		}
		if ( strcmp( MQTT_in_topic, MiS_LCD ) == 0 )
		{
			if ( strcmp( MQTT_in_buffer, "ON" ) == 0 )
			{
				lcd.backlight();                  //  LCD2004 backlight on
			} else {
				lcd.noBacklight();
			}
		}
	}
	client.loop();
}
