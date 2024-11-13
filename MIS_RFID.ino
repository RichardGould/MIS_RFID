/* Men in Sheds RFID ID Verification	*/
/*
	Version:		   1.0
	Date:			     1/11/2024
	Latest Edit:	12/11/2024
	
 */
 
 /*	Library Includes */
 
#include <SPI.h>					// needed for RFID device
#include <MFRC522.h>				// needed for RFID device
#include <ESP8266WiFi.h>			// needed for WIFi
#include <PubSubClient.h>			// needed for MQTT
#include <Wire.h>					// needed for i2c
#include <LiquidCrystal_I2C.h>		// needed for LCD 2004

/*	Definition Section	*/

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

#define MSG_BUFFER_SIZE	(50)

/*	variable instantiation	*/

/*	Network Credentials	*/
const char * Home_WiFi = "GOULDWAN24";
char Home_Pass[ 20 ] = "Teg`*1()SR";
const char * Home_MQTT = "192.168.1.178";
const char * MIS_WiFi  = "BTB-NTCHT6";
char MIS_Pass[ 20 ]  = "RF?[_?Bfi7iJS$";
const char * MIS_MQTT  = "192.168.1.249";
const char * LCD_Title = "MIS Tag Identity";
const char * MIS_TOPIC = "MIS";
const char * MIS_DEVICE = "RFID_1";
const char * MIS_MSG    = "MIS/MESSAGE";

char MQTT_server[ 20 ], my_salut[ 20 ], my_member[ 20 ];
int  MQTT_port = 1883;

char WiFi_Pass[ 20 ];
char PUB_message[ MSG_BUFFER_SIZE ], SUB_message[ MSG_BUFFER_SIZE ], MQTT_in_buffer[ MSG_BUFFER_SIZE ];
char MQTT_in_topic[ 20 ];
char my_IP_Address[ 20 ];					//	xxx.xxx.xxx.xxx
int MQTT_in_flag, MQTT_in_length;
unsigned long epoch;
uint8_t line[ LCD_lines ];

/*	Instance creation	*/

MFRC522 mfrc522( SS_PIN, RST_PIN );						// Create MFRC522 instance

WiFiClient espClient;									// WiFi

LiquidCrystal_I2C lcd( LCD, LCD_chars, LCD_lines );		// LCD2004

PubSubClient client( espClient );						// MQTT

/*	Scan WiFi for networks	*/
int fn_WiFiScan() {
	int network, networks, my_network;
	WiFi.disconnect();
	delay( 2000 );
	my_network = -1;
	networks = WiFi.scanNetworks();
	Serial.print( "Networks seen : " );
	Serial.println( networks );
	if ( networks == 0 ) return( -1 );
	for ( network = 0; network < networks; network++ )
	{
 		if ( strcmp( WiFi.SSID( network ).c_str(), Home_WiFi ) == 0 ) 
		{
			strcpy( WiFi_Pass, Home_Pass );
			strcpy( MQTT_server, Home_MQTT );
			return( network );
		}
		if ( strcmp( WiFi.SSID( network ).c_str(), MIS_WiFi ) == 0 ) 
		{
			strcpy( WiFi_Pass, MIS_Pass );
			strcpy( MQTT_server, MIS_MQTT );
			return( network );
		}      
	}
	return( -1 );
}

/*	Connect to Wifi */
int fn_WiFi_Connect( int network )
{
	int i = 0, rtn = 0;
	fn_Display( 1, 3, 0, "Connecting" );
	delay( 2000 );
	WiFi.begin( WiFi.SSID( network ), WiFi_Pass );
	while ( i < 10 )
  {
    if ( WiFi.status() != WL_CONNECTED )
    delay( 1000 );
    i++;
		if ( i > 10 ) return( -1 );
		Serial.print(".");
	}
	Serial.println();
	fn_Clear( 2 );
  fn_Clear( 3 );
	return( 0 );
}

/*	change LED state	*/
void fn_LED( int LED, int state )
{
  if ( state == 1 ) digitalWrite( LED, LOW );
  if ( state == 0 ) digitalWrite( LED, HIGH );
  if ( state == 2 ) digitalWrite( LED, !digitalRead( LED ) );
}

void fn_title( void )
{
	int i;
	for ( i=0; i<LCD_lines; i++ )
	{
		if ( line[ i ] == 1 ) fn_Clear( i );
	} 
	fn_Display( 1, 0, 0, LCD_Title );			// display title
}

/*	display message on LCD2004	*/
void fn_Display( int my_centre, int my_line, int my_pos, const char * my_message )
{
	if ( my_centre == 1 ) my_pos = ( LCD_chars - strlen( my_message ) ) /2;
  if ( line[ my_line ] == 1 )
  {
    lcd.setCursor( 0, my_line );
    lcd.print( "                   " );
  }
	lcd.setCursor( my_pos, my_line );
	lcd.print( my_message );
	line[ my_line ] = 1;						//	set line in use flag
}

void fn_Clear( int cl_line )
{
  lcd.setCursor( 0, cl_line );
	lcd.print( "                     " );
  line[ cl_line ] = 0;
}

/*	MQTT callback	*/
void MQTT_CB( char * topic, byte * payload, uint8_t length )
{
	int i;

	if ( payload[ 0 ] != 0 )
  {
//   	Serial.print( "Message Received" );
    MQTT_in_flag = 1;
	  strcpy( MQTT_in_topic, topic );
	  MQTT_in_length = length;
	  for ( i = 0; i < length; i++ ) MQTT_in_buffer[ i ] = (char) payload[ i ];
	  MQTT_in_buffer[ i ] = 0;
//	  Serial.println( MQTT_in_buffer );
  }
}

int fn_MQTT_Connect()
{
	int i = 0, rtn = 0;
	Serial.println( "MQTT connection ..." );
//	fn_Display( 0, 3, 0, "MQTT Connect" );
  while ( i<10 )
  {
    rtn = client.connect( MIS_TOPIC );
    if ( rtn == 0 )
    {
      fn_Display( 0, 3, 0, "MQTT Connected" );
      return( 0 );
    }
    Serial.print( "." );
    i++;
  }
	return( rtn );
}

int fn_MQTT_Publish( void )
{
	int i = 0, rtn = 0;

  if ( !client.connected() )
	{
		rtn = fn_MQTT_Connect();
		if ( rtn > 0 ) return( rtn );
	}
  Serial.println( "MQTT publish ..." );
  fn_Display( 0, 3, 0, "MQTT Publish" );
  while( i < 10 )
  {
    rtn = client.publish( MIS_TOPIC, PUB_message, false );
    if ( rtn == 0 )
    {
      fn_Display( 0, 3, 0, "MQTT Published" );
      return( 0 );
    }
    i++;
  }
	return( rtn );
}

/*	------------------------------------------------------------------------------- */


void setup() {
	int i;
	epoch = millis(); 
	Serial.begin( 115200 );						// 	Initialize serial communications with the PC
	pinMode( GREEN, OUTPUT );					//	Green LED
	pinMode( RED, OUTPUT );						//	Red LED
	digitalWrite( GREEN, HIGH );			//	Off
	digitalWrite( RED, HIGH );				//	Off
	SPI.begin();
	lcd.init();									      //	Initialize LCD2004
	lcd.backlight();							    //	LCD2004backlight on

	fn_title();									      //	Title line on LCD2004

  fn_Display( 1, 2, 0, "Network Search" );
  //  decrypt passwords
	for ( i=0; i<strlen( Home_Pass ); i++ )	Home_Pass[ i ] = Home_Pass[ i ] +3 +i;
	for ( i=0; i<strlen( MIS_Pass  ); i++ )	MIS_Pass[ i ]  = MIS_Pass[ i ]  +3 +i;

	int net = -1, res = -1, count = 0;
	while ( net == -1 )
	{
		net = fn_WiFiScan();					//	returns network or -1
		if ( net == -1 ) Serial.println( "No WiFi" );
		delay( 5000 );
    if ( count++ > 10 ) return;     // restart wemo
	}
	count = 0;
  while ( res != 0 )
  {
    res = fn_WiFi_Connect( net );
	  if ( res != 0 )
    {
      delay( 1000 );
      if ( count++ > 10 ) return;						//	restart Wemo
    }
  }
	char my_IP_Address[ 20 ];
	strcpy(  my_IP_Address, WiFi.localIP().toString().c_str() );
	fn_Display( 1, 2, 0, "Network Found" );
	fn_Display( 1, 3, 0, my_IP_Address );
	delay( 3000 );
	fn_Clear( 2 );
  fn_Clear( 3 );
 
//	MQTT setup
	client.setServer( MQTT_server, MQTT_port );
	client.setCallback( MQTT_CB );

	client.subscribe( MIS_MSG );			// message from broker
	
	mfrc522.PCD_Init();
	fn_Display( 1, 2, 0, "** Ready **" );
}

int fn_RFID()
{
	int rtn = 0, i;
	char my_UID[ 10 ];

	if ( ! mfrc522.PICC_IsNewCardPresent() ) return( 1 );
	if ( ! mfrc522.PICC_ReadCardSerial()   ) return( 1 );
 
	fn_Display( 1, 2, 0, "** Tag detected **" );

	sprintf( my_UID, "%02x%02x%02x%02x",
			mfrc522.uid.uidByte[0],
			mfrc522.uid.uidByte[1],
			mfrc522.uid.uidByte[2],
			mfrc522.uid.uidByte[3] );

	memset( PUB_message, 0, sizeof( PUB_message ) );
	strcat( PUB_message, MIS_DEVICE );
	strcat( PUB_message, "/" );
	strcat( PUB_message, my_UID );

	fn_Display( 1, 3, 0, my_UID );
	Serial.println( PUB_message );

	if ( !client.connected() ) fn_MQTT_Connect();

	client.publish( MIS_TOPIC, PUB_message, false );

	memset( SUB_message, 0, sizeof( SUB_message ) );
	strcat( SUB_message, MIS_TOPIC );
	strcat( SUB_message, "/" );
	strcat( SUB_message, MIS_DEVICE );
	strcat( SUB_message, "/" );
	strcat( SUB_message, my_UID );

	Serial.print( "Subscription : " );
	Serial.println( SUB_message );

	fn_LED( RED, 1 );					//	turn RED led on

	client.subscribe( SUB_message );

	unsigned long epoch = millis();
	while ( MQTT_in_flag == 0 )
	{
		client.loop();
		delay( 100 );
    unsigned now = millis();
		if ( ( now - epoch ) > ( 20 * 1000 ) )
		{
	    fn_Display( 1, 2, 0, "No Response" );
			fn_LED( RED, 0 );
			delay( 5000 );
			return( 0 );
		}
	}

	if ( strcmp( MQTT_in_topic, SUB_message ) )
	{
		MQTT_in_flag = 0;
		fn_Display( 1, 2, 0, "Bad Response" );
		fn_LED( RED, 0 );
		delay( 5000 );
	}

//		MQTT broker response Y/I/Richard or Y/O/Richard or N

	if ( MQTT_in_buffer[ 0 ] == 'N' )
	{
		fn_Display( 1, 2, 0, "*** Tag Unknown ***" );
		fn_LED( RED, 0 );
		delay( 10000 );
		return( 0 );
	}

	if ( MQTT_in_buffer[ 2 ] == 'I' )
	{
		sprintf( my_salut, "%s ", "Hi : " );
	} else {
		sprintf( my_salut, "%s", "Bye: " );
	}

	for ( i=4; i<strlen( MQTT_in_buffer ); i++ ) my_member[ ( i - 4 ) ] = MQTT_in_buffer[ i ];
	
  my_member[ ( i - 4 ) ] = 0;

	strcat( my_salut, my_member );
	fn_Clear( 3 );
	fn_Display( 1, 2, 0, my_salut );
	client.publish( SUB_message, "", false );				//	delete published topic
	fn_LED( RED, 0 );				// turn RED led off

	fn_LED( GREEN, 1 );     // turn GREEN led on
	delay( 10000 );         // wait for a bit
	fn_LED( GREEN, 0 );     // turn GREEN led off
	return( 0 );
}

void loop() {
  if ( line[ 2 ] == 0 ) fn_Display( 1, 2, 0, "** Ready **" );
  client.subscribe( MIS_MSG );			// message from broker
  unsigned long now = millis();
	if ( ( now - epoch )  > 10000 )
	{
		epoch = millis();
		fn_title();      //  refresh screen every 1 minutes
	}

	if ( ! client.connected() ) fn_MQTT_Connect();

	fn_RFID();

	if ( MQTT_in_flag == 1 )
	{
		if ( ! strcmp( MQTT_in_topic, MIS_MSG ) )
		{
			Serial.print( "Incoming message : " );
			Serial.println( MQTT_in_buffer );
			MQTT_in_flag = 0;
			fn_Display( 1, 0, 0, MQTT_in_buffer );
		} else {
			MQTT_in_flag = 0;
		}
	}
  client.loop();
}