/********************************************************************************
 *                                                                              *
 *   MiS Suite                                                                  *
 *                                                                              *
 *   MiS_PSW.c      -    Create encrypted password for WiFi                     *
 *                                                                              *
 *   Description    -    Command line instruction to create an encrypted        *
 *                       password for use within tag system commissioned        *
 *                                                                              *
 *   Command Line   -    MiS_PSW <True Password>                                *
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

/*
 *      System libraries
 */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
/*
 *      Local libraries
 */
#define _POSIX_SOURCE 1 /* POSIX compliant source */

/*
 *      Global Variables
 */
volatile int i, j, k, rc, count;

int red_led = 0, green_led = 0;

char serial_buf[ 255 ];

void fn_exit( int code )
{
        exit( code );
}

void fn_sighandler( int sig )
{
        printf( "\nsignal %i received\n", sig );
        exit( 1 );
}

int fn_usage( void )
{
        printf( "usage : pass <password>\n" );
        exit( 0 );
}

int main( int argc, char *argv[] )
{
        int c;
        char ch;
        char my_password[ 20 ];
/*
 *      process command line
 */
        signal( SIGINT, fn_sighandler );
        signal( SIGPWR, fn_sighandler );

        strcpy( my_password, argv[ 1 ] );

        printf( "Password : %s : ", my_password );
        for ( i=0; i<strlen( my_password ); i++ )
        {
                ch = ( my_password[ i ] -3 +i );
                if ( ch == '\\' )
                {
                         printf( "%c", '\\' );
                } else {
                        printf( "%c", ch );
                }
        }
        printf( "\n" );
}

/*
 *      EOC     -       End of Code
 */

