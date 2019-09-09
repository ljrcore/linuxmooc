//----------------------------------------------------------------
//	fileview.cpp
//
//	This program displays the contents of a specified file 
//	in hexadecimal and ascii formats (including any device
//	special files representing storage media).  A user may
//	navigate the file's contents using arrow-key commands,
//	or may adjust the format of the hexadecimal display to
//	select from among five data-sizes: byte (B), word (W), 
//	doubleword (D), quadword (Q) or octaword (O).  It also
//	is possible to seek to a specified position within the
//	file by hitting the <ENTER>-key and then typing in the
//	desired (hexadecimal) address.  Type <ESCAPE> to quit.  
//
//	       compile-and-link using: $ make fileview
//
//	programmer: ALLAN CRUSE
//	written on: 26 OCT 2002
//	revised on: 07 JUN 2006 -- removed reliance on 'ncurses' 
//----------------------------------------------------------------

#include <stdio.h>	// for printf(), perror(), fflush() 
#include <fcntl.h>	// for open()
#include <string.h>	// for strncpy()
#include <unistd.h>	// for read(), lseek64()
#include <stdlib.h>	// for exit()
#include <termios.h>	// for tcgetattr(), tcsetattr()

#define MAXNAME	80
#define BUFHIGH 16
#define BUFWIDE 16
#define BUFSIZE 256
#define ROW	6
#define COL	2

#define KB_SEEK 0x0000000A
#define KB_QUIT	0x0000001B
#define KB_BACK 0x0000007F
#define KB_HOME	0x00315B1B
#define KB_LNUP 0x00415B1B
#define KB_PGUP	0x00355B1B
#define KB_LEFT 0x00445B1B
#define KB_RGHT 0x00435B1B
#define KB_LNDN 0x00425B1B
#define KB_PGDN 0x00365B1B
#define KB_END  0x00345B1B
#define KB_DEL  0x00335B1B


char progname[] = "FILEVIEW";
char filename[ MAXNAME + 1 ];
char buffer[ BUFSIZE ];
char outline[ 80 ];

int main( int argc, char *argv[] )
{
	// setup the filename (if supplied), else terminate
	if ( argc > 1 ) strncpy( filename, argv[1], MAXNAME );
 	else { fprintf( stderr, "argument needed\n" ); exit(1); }

	// open the file for reading
	int	fd = open( filename, O_RDONLY );
	if ( fd < 0 ) { perror( filename ); exit(1); }

	// obtain the filesize (if possible)
	long long	filesize = lseek64( fd, 0LL, SEEK_END );
	if ( filesize < 0LL ) 
		{ 
		fprintf( stderr, "cannot locate \'end-of-file\' \n" ); 
		exit(1); 
		}

	long long	incmin = ( 1LL <<  8 );
	long long	incmax = ( 1LL << 36 );		
	long long	posmin = 0LL;
	long long	posmax = (filesize - 241LL)&~0xF;
	if ( posmax < posmin ) posmax = posmin;

	// initiate noncanonical terminal input
	struct termios	tty_orig;
	tcgetattr( STDIN_FILENO, &tty_orig );
	struct termios	tty_work = tty_orig;
	tty_work.c_lflag &= ~( ECHO | ICANON );  // | ISIG );
	tty_work.c_cc[ VMIN ]  = 1;
	tty_work.c_cc[ VTIME ] = 0;
	tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_work );	
	printf( "\e[H\e[J" );

	// display the legend
	int	i, j, k;
	k = (77 - strlen( progname ))/2;
	printf( "\e[%d;%dH %s ", 1, k, progname );
	k = (77 - strlen( filename ))/2;
	printf( "\e[%d;%dH\'%s\'", 3, k, filename );
	char	infomsg[ 80 ];
	sprintf( infomsg, "filesize: %llu (=0x%013llX)", filesize, filesize );
	k = (78 - strlen( infomsg ));
	printf( "\e[%d;%dH%s", 24, k, infomsg );
	fflush( stdout );

	// main loop to navigate the file
	long long	pageincr = incmin;
	long long	lineincr = 16LL;
	long long	position = 0LL;
	long long	location = 0LL;
	int		format = 1;
	int		done = 0;
	while ( !done )
		{
		// erase prior buffer contents
		for (j = 0; j < BUFSIZE; j++) buffer[ j ] = ~0;

		// restore 'pageincr' to prescribed bounds
		if ( pageincr == 0LL ) pageincr = incmax;
		else if ( pageincr < incmin ) pageincr = incmin;
		else if ( pageincr > incmax ) pageincr = incmax;
		
		// get current location of file-pointer position
		location = lseek64( fd, position, SEEK_SET );
		
		// try to fill 'buffer[]' with data from the file
		char	*where = buffer;
		int	to_read = BUFSIZE;
		while ( to_read > 0 )
			{
			int	nbytes = read( fd, where, to_read );
			if ( nbytes <= 0 ) break; 
			to_read -= nbytes;
			where += nbytes;
			}
		int	datalen = BUFSIZE - to_read; 

		// display the data just read into the 'buffer[]' array
		unsigned char		*bp;
		unsigned short		*wp;
		unsigned int		*dp;
		unsigned long long	*qp;
		for (i = 0; i < BUFHIGH; i++)
			{
			int	linelen;

			// draw the line-location (13-digit hexadecimal)
			linelen = sprintf( outline, "%013llX ", location );
			
			// draw the line in the selected hexadecimal format
			switch ( format )
				{
				case 1:	// 'byte' format
				bp = (unsigned char*)&buffer[ i*BUFWIDE ];
				for (j = 0; j < BUFWIDE; j++)
					linelen += sprintf( outline+linelen, 
						"%02X ", bp[j] );
				break;

				case 2:	// 'word' format
				wp = (unsigned short*)&buffer[ i*BUFWIDE ];
				for (j = 0; j < BUFWIDE/2; j++)
					linelen += sprintf( outline+linelen,
						" %04X ", wp[j] );
				break;

				case 4:	// 'dword' format
				dp = (unsigned int*)&buffer[ i*BUFWIDE ];
				for (j = 0; j < BUFWIDE/4; j++)
					linelen += sprintf( outline+linelen,
						"  %08X  ", dp[j] );
				break;

				case 8:	// 'qword' format
				qp = (unsigned long long*)&buffer[ i*BUFWIDE ];
				for (j = 0; j < BUFWIDE/8; j++)
					linelen += sprintf( outline+linelen,
						"    %016llX    ", qp[j] );
				break;

				case 16: // 'octaword'
				qp = (unsigned long long*)&buffer[ i*BUFWIDE ];
				linelen += sprintf( outline+linelen, "     " );
				linelen += sprintf( outline+linelen, 
					"   %016llX%016llX   ", qp[1], qp[0] );
				linelen += sprintf( outline+linelen, "     " ); 
				break;
				}

			// draw the line in ascii format
			for (j = 0; j < BUFWIDE; j++)
				{
				char	ch = buffer[ i*BUFWIDE + j ];
				if (( ch < 0x20 )||( ch > 0x7E )) ch = '.';
				linelen += sprintf( outline+linelen, "%c", ch);
				}

			// transfer this output-line to the screen 
			printf( "\e[%d;%dH%s", ROW+i, COL, outline );

			// advance 'location' for the next output-line
			location += BUFWIDE;
			} 	
		printf( "\e[%d;%dH", 23, COL );
		fflush( stdout );	
	
		// await keypress 	
		long long	inch = 0LL;
		read( STDIN_FILENO, &inch, sizeof( inch ) );
		printf( "\e[%d;%dH%60s", 23, COL, " " );	

		// interpret navigation or formatting command
		inch &= 0x00FFFFFFLL;
		switch ( inch )
			{
			// move to the file's beginning/ending
			case 'H': case 'h':
			case KB_HOME:	position = posmin; break;
			case 'E': case 'e':
			case KB_END:	position = posmax; break;

			// move forward/backward by one line
			case KB_LNDN:	position += BUFWIDE; break;
			case KB_LNUP:	position -= BUFWIDE; break;

			// move forward/packward by one page
			case KB_PGDN:	position += pageincr; break;
			case KB_PGUP:	position -= pageincr; break;

			// increase/decrease the page-size increment
			case KB_RGHT:	pageincr >>= 4; break;
			case KB_LEFT:	pageincr <<= 4; break;

			// reset the hexadecimal output-format
			case 'B': case 'b':	format = 1; break;
			case 'W': case 'w':	format = 2; break;
			case 'D': case 'd':	format = 4; break;
			case 'Q': case 'q':	format = 8; break;
			case 'O': case 'o':	format = 16; break;

			// seek to a user-specified file-position
			case KB_SEEK:
			printf( "\e[%d;%dHAddress: ", 23, COL );
			fflush( stdout );
			{
			char	inbuf[ 16 ] = {0};
				//tcsetattr( STDIN_FILENO, TCSANOW, &tty_orig );
			int	i = 0;
			while ( i < 15 )
				{
				long long ch = 0;
				read( STDIN_FILENO, &ch, sizeof( ch ) );
				ch &= 0xFFFFFF;
				if ( ch == '\n' ) break;
				if ( ch == KB_QUIT ) { inbuf[0] = 0; break; }
				if ( ch == KB_LEFT ) ch = KB_BACK;
				if ( ch == KB_DEL ) ch = KB_BACK;
				if (( ch == KB_BACK )&&( i > 0 ))
					{ 
					inbuf[--i] = 0; 
					printf( "\b \b" ); 
					fflush( stdout );
					}
				if (( ch < 0x20 )||( ch > 0x7E )) continue;
				inbuf[ i++ ] = ch;
				printf( "%c", ch );
				fflush( stdout );
				}		
			printf( "\e[%d;%dH%70s", 23, COL, " " );
			fflush( stdout );
			position = strtoull( inbuf, NULL, 16 );
			position &= ~0xFLL;	// paragraph align
			}
			break;			

			// program termination 
			case KB_QUIT:	done = 1; break;

			default:	
			printf( "\e[%d;%dHHit <ESC> to quit", 23, 2 ); 
			}
		fflush( stdout );

		// insure that 'position' remains within bounds
		if ( position < posmin ) position = posmin;
		if ( position > posmax ) position = posmax;
		}	

	// restore canonical terminal behavior
	tcsetattr( STDIN_FILENO, TCSAFLUSH, &tty_orig );	
	printf( "\e[%d;%dH\e[0J\n", 23, 0 );
}

