#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <utmpx.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "libhttparser.h"

funcProcessHttpData ProcessHttpData ;
int ProcessHttpData( struct HttpEnv *e , void *p )
{
	char		rsp_header[] =	"Date: Wed, 24 Dec 2014 23:56:04 GMT\r\n"
					"Server: Apache\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: %d\r\n"
					"\r\n" ;
	char		rsp_body[] =	"<html>\r\n"
					"<head>\r\n"
					"<title>test</title>\r\n"
					"<meta name=\"renderer\" content=\"webkit\">\r\n"
					"<meta http-equiv=\"content-type\" content=\"text/html\" charset=\"utf-8\">\r\n"
					"</head>\r\n"
					"<body>\r\n"
					"hello\r\n"
					"</body>\r\n"
					"</html>\r\n" ;
	char		*rsp_ptr =	"123" ;
	int		fd ;
	long		append_len ;
	int		nret = 0 ;
	
	printf( "METHOD         [%s]\n" , GetRequestHeader_METHOD(e) );
	printf( "URL            [%s]\n" , GetRequestHeader_URL(e) );
	printf( "VERSION        [%s]\n" , GetRequestHeader_VERSION(e) );
	printf( "Method         [%s]\n" , GetRequestHeader_METHOD(e) );
	printf( "Content-Length [%s]\n" , GetRequestHeader_CONTENT_LENGTH(e) );
	printf( "Cookie         [%s]\n" , GetRequestHeader_COOKIE(e) );
	printf( "User-Agent     [%s]\n" , GetRequestHeader_USER_AGENT(e) );
	printf( "Host           [%s]\n" , GetRequestHeader_HOST(e) );
	printf( "Referer        [%s]\n" , GetRequestHeader_REFERER(e) );
	printf( "Connection     [%s]\n" , GetRequestHeader_CONNECTION(e));
	printf( "Accept         [%s]\n" , GetRequestHeader_ACCEPT(e) );
	printf( "Accept-Language[%s]\n" , GetRequestHeader_ACCEPT_LANGUAGE(e) );
	printf( "Accept-Encoding[%s]\n" , GetRequestHeader_ACCEPT_ENCODING(e) );
	
	printf( "REQ-BODY       [%.*s]\n" , (int)GetRequestBodyLength(e) , GetRequestBodyBase(e) );
	
	fd = open( "file.txt" , O_RDONLY ) ;
	if( fd == -1 )
	{
		return -1;
	}
	
	append_len = lseek( fd , 0 , SEEK_END );
	lseek( fd , 0 , SEEK_SET );
	
	nret = StrcpyHttpStatusCode( e , HTTPARSER_STATUSCODE_200 ) ;
	if( nret )
		return -1;
	
	nret = StrcatfHttpResponseBuffer( e , rsp_header , strlen(rsp_body) + strlen(rsp_ptr) + append_len ) ;
	if( nret )
		return -1;
	
	nret = StrcatHttpResponseBuffer( e , rsp_body ) ;
	if( nret )
		return -1;
	
	SetHttpResponseAppendPtr( e , rsp_ptr , strlen(rsp_ptr) );
	SetHttpResponseAppendFd( e , fd , 0 , 1 );
	
	return 0;
}

int main()
{
	int			listen_sock ;
	struct sockaddr_in	listen_addr ;
	int			accept_sock ;
	struct sockaddr_in	accept_addr ;
	socklen_t		accept_addr_len ;
	int			onoff ;
	
	struct HttpEnv		*e = NULL ;
	
	int			nret = 0 ;
	
	InitHttpEnv( & e );
	
	listen_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( listen_sock == -1 )
	{
		printf( "socket failed , errno[%d]\n" , errno );
		return 1;
	}
	
	onoff = 1 ;
	setsockopt( listen_sock , SOL_SOCKET , SO_REUSEADDR , (void *) & onoff , sizeof(onoff) );
	
	memset( & listen_addr , 0x00 , sizeof(struct sockaddr_in) );
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr( "0.0.0.0" );
	listen_addr.sin_port = htons( (unsigned short)8081 );
	
	nret = bind( listen_sock , (struct sockaddr *) & listen_addr , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "bind failed , errno[%d]\n" , errno );
		return 1;
	}
	
	nret = listen( listen_sock , 1024 ) ;
	if( nret == -1 )
	{
		printf( "listen failed , errno[%d]\n" , errno );
		return 1;
	}
	
	while(1)
	{
		accept_addr_len = sizeof(struct sockaddr) ;
		accept_sock = accept( listen_sock , (struct sockaddr *) & accept_addr, & accept_addr_len );
		if( accept_sock == - 1 )
		{
			printf( "accept failed , errno[%d]\n" , errno );
			break;
		}
		
		nret = PerformHttpParser( accept_sock , e , 10 , & ProcessHttpData , NULL ) ;
		if( nret )
		{
			printf( "PerformHttpParser[%d]\n" , nret );
			break;
		}
		
		close( accept_sock );
	}
	
	close( listen_sock );
	
	CleanHttpEnv( & e );
	
	return 0;
}

