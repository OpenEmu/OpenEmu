#include <sys/types.h>
#include <network.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "net_print.h"

/* It opens a connection to the host name rhost that is listening
   on the specified port. It returns the socket descriptor, or -1
   in case of failure.
 */
static int clientsocket(const char *rhost, unsigned short port)
{
  struct sockaddr_in sad;/* structure to hold server's address*/
  int    fd;             /* socket descriptor                 */

  memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
  sad.sin_family = AF_INET;  /* set family to Internet */
  sad.sin_port = htons((u_short)port); 
  sad.sin_addr.s_addr = inet_addr( rhost );
  
  /* Create a socket */
  fd = net_socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    printf( "socket creation failed\n");
    return (-1);;
  }
  
  /* Connect the socket to the specified server */
  if (net_connect(fd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
    printf( "connect failed\n");
    return (-1);
  }

  return fd;
}

static int _net_print_socket = -1;

int net_print_init(const char *rhost, unsigned short port)
{
	int	sk = -1;
	int	wd = 0x12345678;

	if ( _net_print_socket < 0 ) {
		if (rhost==NULL){
			rhost = DEFAULT_NET_PRINT_HOST_IP;
		}
		if (port <= 0 ){
			port = DEFAULT_NET_PRINT_PORT;
		}

		sk = clientsocket( rhost, port);
		if ( sk >= 0 ) {
			_net_print_socket = sk;
			net_print_string( __FILE__, __LINE__, "net_print_init() successful, socket=%d, testing for hi-low-byte using int 0x12345678:\n", _net_print_socket);

 	  		net_print_binary( 'X', &wd, sizeof(wd));
		}
	}


	return _net_print_socket;
}

void net_print_close()
{  
  if( _net_print_socket >= 0 ) 
  { 
    net_print_string( __FILE__, __LINE__, "net_print_init() closing socket, socket=%d\n", _net_print_socket );
    net_close( _net_print_socket );
  }
}

int net_print_string( const char* file, int line, const char* format, ...)
{
	va_list	ap;
	int len;
	int ret;
	char buffer[512];

	va_start(ap, format);

	if ( _net_print_socket < 0 ) {
		return	-1;
	}

	len = 0;
	if ( file != NULL) {
		len = sprintf( buffer, "%s:%d, ", file, line);
	}

	len += vsprintf( buffer+len, format, ap);
	va_end(ap);

	ret = net_send( _net_print_socket, buffer, len, 0);
	return ret;
}

int net_print_binary( int format, const void* data, int len)
{
	int col, k, ret;
	char line[80], *out;
	const unsigned char *in;
	const char* binary = data;

	if ( _net_print_socket < 0 ) {
		return	-1;
	}

	in = (unsigned char*)binary;
	for( k=0; k<len ; ) {
		out = line;
		switch( format ) {
		case 'x':
			for( col=0; col<8 && k<len && ((len-k)/2>0); col++, k+=2) {
				ret = sprintf( out, "%02X%02X ", *in, *(in+1));
				if ( col==3) {
					strcat( out++, " ");
				}
				out += ret;
				in+=2;
			}
			break;
		case 'X':
			for( col=0; col<4 && k<len && ((len-k)/4>0); col++, k+=4) {
				ret = sprintf( out, "%02X%02X%02X%02X ", *in, *(in+1), *(in+2), *(in+3));
				if ( col==1) {
					strcat( out++, " ");
				}
				out += ret;
				in+=4;
			}
			break;
				
		case 'c':
			for( col=0; col<16 && k<len; col++, k++) {
				if ( isprint( *in) && !isspace(*in)) {
					ret = sprintf( out, "%c  ", *in);
				} else {
					ret = sprintf( out, "%02X ", *in);
				}
				if ( col==7) {
					strcat( out++, " ");
				}
				out += ret;
				in++;
			}
			break;

		default:
			for( col=0; col<16 && k<len; col++, k++) {
				ret = sprintf( out, "%02X ", *in);
				if ( col==7) {
					strcat( out++, " ");
				}
				out += ret;
				in++;
			}
		}
		if ( out != line ) {
			strcat( out, "\n");
			net_print_string( NULL, 0, "%s", line);
		} else {
			break;
		}
	}
/***
	if ( out!=line) {
		strcat( out, "\n");
		net_print_string( NULL, 0, "%s", line);
	}
***/
	return len;
}


