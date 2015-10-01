/*
 * libhttparser
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#include "libhttparser.h"

struct HttpBuffer
{
	long		bufsize ;
	long		len ;
	char		*base ;
	char		*ptr ;
} ;

struct HttpHeader
{
	long		header_total_len ;
	
	char		*line_begin ;
	char		*line_end ;
	
	char		*METHOD ;
	char		*URL ;
	char		*VERSION ;
	
	char		*CONTENT_LENGTH ;
	char		*COOKIE ;
	char		*USER_AGENT ;
	char		*HOST ;
	char		*REFERER ;
	char		*CONNECTION ;
	char		*ACCEPT ;
	char		*ACCEPT_LANGUAGE ;
	char		*ACCEPT_ENCODING ;
} ;

struct HttpBody
{
	long		body_total_len ;
	
	long		body_remain_len ;
} ;

struct HttpAppend
{
	char		*append_ptr ;
	long		append_len ;
	
	int		append_fd_flag ;
	int		append_fd ;
	int		select_flag ;
	int		close_flag ;
} ;

struct HttpEnv
{
	struct timeval		timeout ;
	struct timeval		*ptimeout ;
	
	struct HttpBuffer	*req_buf ;
	struct HttpHeader	req_header ;
	struct HttpBody		req_body ;
	
	struct HttpBuffer	*rsp_buf ;
	struct HttpAppend	rsp_append ;
} ;

static char *strpchr( char *begin , char *end , char c )
{
	char		*pc = begin ;
	
	for( ; pc <= end ; pc++ )
	{
		if( (*pc) == c )
			return pc;
	}
	
	return NULL;
}

static void AjustTimeval( struct timeval *ptv , struct timeval *t1 , struct timeval *t2 )
{
	ptv->tv_sec -= ( t2->tv_sec - t1->tv_sec ) ;
	ptv->tv_usec -= ( t2->tv_usec - t1->tv_usec ) ;
	
	while( ptv->tv_usec < 0 )
	{
		ptv->tv_usec += 1000000 ;
		ptv->tv_sec--;
	}
	
	return;
}

static void FreeHttpBuffer( struct HttpBuffer *b )
{
	if( b )
	{
		if( b->base )
			free( b->base );
		
		free( b );
	}
	
	return;
}

static struct HttpBuffer *AllocHttpBuffer( int bufsize )
{
	struct HttpBuffer	*b = NULL ;
	
	b = (struct HttpBuffer *)malloc( sizeof(struct HttpBuffer) ) ;
	if( b == NULL )
	{
		return NULL;
	}
	memset( b , 0x00 , sizeof(struct HttpBuffer) );
	
	b->base = (char*)malloc( bufsize ) ;
	if( b->base == NULL )
	{
		FreeHttpBuffer( b );
		return NULL;
	}
	memset( b->base , 0x00 , bufsize );
	b->bufsize = bufsize ;
	b->len = 0 ;
	b->ptr = b->base ;
	
	return b;
}

static int ReallocHttpBuffer( struct HttpBuffer *b , int new_bufsize )
{
	char	*new_base = NULL ;
	
	new_base = (char*)realloc( b->base , new_bufsize ) ;
	if( new_base == NULL )
		return HTTPARSER_ERROR_ALLOC;
	
	b->ptr = new_base + ( b->ptr - b->base ) ;
	b->base = new_base ;
	b->bufsize = new_bufsize ;
	memset( b->ptr , 0x00 , b->bufsize - b->len );
	
	return 0;
}

static void CleanHttpBuffer( struct HttpBuffer *b )
{
	memset( b->base , 0x00 , b->len );
	b->len = 0 ;
	b->ptr = b->base ;
	
	return;
}

int StrcpyHttpStatusCode( struct HttpEnv *e , char *status_code )
{
	long		len ;
	
	len = strlen(status_code) ;
	
	strcpy( e->rsp_buf->ptr , status_code );
	e->rsp_buf->ptr += len ;
	e->rsp_buf->len += len ;
	
	return 0;
}

int StrcatHttpResponseBuffer( struct HttpEnv *e , char *str )
{
	long		len ;
	int		nret = 0 ;
	
	len = strlen(str) ;
	
	while( e->rsp_buf->len + len > e->rsp_buf->bufsize - 1 )
	{
		nret = ReallocHttpBuffer( e->rsp_buf , e->rsp_buf->bufsize * 2 ) ;
		if( nret )
			return nret;
	}
	
	strncpy( e->rsp_buf->ptr , str , len ) ;
	e->rsp_buf->ptr += len ;
	e->rsp_buf->len += len ;
	
	return 0;
}

int StrcatfHttpResponseBuffer( struct HttpEnv *e , char *format , ... )
{
	va_list		valist ;
	long		size ;
	int		len ;
	int		nret = 0 ;
	
	size = e->rsp_buf->bufsize - e->rsp_buf->len - 1 ;
	
	while(1)
	{
		va_start( valist , format );
		len = VSNPRINTF( e->rsp_buf->ptr , size , format , valist ) ;
		va_end( valist );
		
		if( len == -1 || len == size )
		{
			nret = ReallocHttpBuffer( e->rsp_buf , e->rsp_buf->bufsize * 2 ) ;
			if( nret )
				return nret;
		}
		else
		{
			break;
		}
	}
	
	e->rsp_buf->ptr += len ;
	e->rsp_buf->len += len ;
	
	return 0;
}

void CleanHttpResponseBuffer( struct HttpEnv *e )
{
	CleanHttpBuffer( e->rsp_buf );
	return;
}

void SetHttpResponseAppendPtr( struct HttpEnv *e , char *ptr , long len )
{
	e->rsp_append.append_ptr = ptr ;
	e->rsp_append.append_len = len ;
	return;
}

void SetHttpResponseAppendFd( struct HttpEnv *e , int append_fd , int select_flag , int close_flag )
{
	e->rsp_append.append_fd_flag = 1 ;
	e->rsp_append.append_fd = append_fd ;
	e->rsp_append.select_flag = select_flag ;
	e->rsp_append.close_flag = close_flag ;
	return;
}

char *GetRequestHeader_METHOD( struct HttpEnv *e )
{
	return e->req_header.METHOD;
}

char *GetRequestHeader_URL( struct HttpEnv *e )
{
	return e->req_header.URL;
}

char *GetRequestHeader_VERSION( struct HttpEnv *e )
{
	return e->req_header.VERSION;
}

char *GetRequestHeader_CONTENT_LENGTH( struct HttpEnv *e )
{
	return e->req_header.CONTENT_LENGTH;
}

char *GetRequestHeader_COOKIE( struct HttpEnv *e )
{
	return e->req_header.COOKIE;
}

char *GetRequestHeader_USER_AGENT( struct HttpEnv *e )
{
	return e->req_header.USER_AGENT;
}

char *GetRequestHeader_HOST( struct HttpEnv *e )
{
	return e->req_header.HOST;
}

char *GetRequestHeader_REFERER( struct HttpEnv *e )
{
	return e->req_header.REFERER;
}

char *GetRequestHeader_CONNECTION( struct HttpEnv *e )
{
	return e->req_header.CONNECTION;
}

char *GetRequestHeader_ACCEPT( struct HttpEnv *e )
{
	return e->req_header.ACCEPT;
}

char *GetRequestHeader_ACCEPT_LANGUAGE( struct HttpEnv *e )
{
	return e->req_header.ACCEPT_LANGUAGE;
}

char *GetRequestHeader_ACCEPT_ENCODING( struct HttpEnv *e )
{
	return e->req_header.ACCEPT_ENCODING;
}

char *GetRequestBodyBase( struct HttpEnv *e )
{
	return e->req_buf->base+e->req_header.header_total_len;
}

long GetRequestBodyLength( struct HttpEnv *e )
{
	return e->req_body.body_total_len;
}

int SetRequestBufferSize( struct HttpEnv *e , long bufsize )
{
	return ReallocHttpBuffer( e->req_buf , bufsize );
}

int SetResponseBufferSize( struct HttpEnv *e , long bufsize )
{
	return ReallocHttpBuffer( e->rsp_buf , bufsize );
}

static int ParseHttpHeaderLine(struct HttpEnv *e )
{
	if( e->req_header.METHOD == NULL )
	{
		char	*pc = NULL ;
		
		for( pc = e->req_header.line_begin ; *(pc) == ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		e->req_header.METHOD = pc ;
		for( pc++ ; *(pc) != ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		*(pc) = '\0' ;
		
		for( pc++ ; *(pc) == ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		e->req_header.URL = pc ;
		for( pc++ ; *(pc) != ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		*(pc) = '\0' ;
		
		for( pc++ ; *(pc) == ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		e->req_header.VERSION = pc ;
		for( pc++ ; *(pc) != ' ' ; pc++ )
		{
			if( pc > e->req_header.line_end )
				break;
		}
	}
	else
	{
		char	*line_middle = NULL ;
		
		line_middle = strpchr( e->req_header.line_begin , e->req_header.line_end , ':' ) ;
		if( line_middle == NULL )
		{
			return HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID;
		}
		
		*(line_middle) = '\0' ;
		
		for( line_middle++ ; line_middle < e->req_header.line_end ; line_middle++ )
		{
			if( *(line_middle) != ' ' )
				break;
		}
		
		if( STRICMP( e->req_header.line_begin , == , "Content-Length" ) )
		{
			e->req_header.CONTENT_LENGTH = line_middle ;
			e->req_body.body_total_len = atol(line_middle) ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Cookie" ) )
		{
			e->req_header.COOKIE = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "User-Agent" ) )
		{
			e->req_header.USER_AGENT = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Host" ) )
		{
			e->req_header.HOST = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Referer" ) )
		{
			e->req_header.REFERER = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Connection" ) )
		{
			e->req_header.CONNECTION = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Accept" ) )
		{
			e->req_header.ACCEPT = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Accept-Language" ) )
		{
			e->req_header.ACCEPT_LANGUAGE = line_middle ;
		}
		else if( STRICMP( e->req_header.line_begin , == , "Accept-Encoding" ) )
		{
			e->req_header.ACCEPT_ENCODING = line_middle ;
		}
	}
	
	return 0;
}

static int ReadHttpHeader( int sock , struct HttpEnv *e )
{
	struct timeval	t1 , t2 ;
	fd_set		fds ;
	char		*block_end = NULL ;
	long		len ;
	int		nret = 0 ;
	
	e->req_header.line_begin = e->req_buf->base ;
	
	while(1)
	{
		while( e->req_buf->len + HTTPARSER_READBLOCK_SIZE_DEFAULT > e->req_buf->bufsize )
		{
			nret = ReallocHttpBuffer( e->req_buf , e->req_buf->bufsize * 2 ) ;
			if( nret )
				return nret;
		}
		
		gettimeofday( & t1 , NULL );
		
		FD_ZERO( & fds );
		FD_SET( sock , & fds );
		
		nret = select( sock + 1 , & fds , NULL , NULL , e->ptimeout ) ;
		if( nret == 0 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_RECEIVE_TIMEOUT;
		}
		else if( nret != 1 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_RECEIVE;
		}
		
		len = (long)read( sock , e->req_buf->ptr , HTTPARSER_READBLOCK_SIZE_DEFAULT ) ;
		if( len == -1 )
			return HTTPARSER_ERROR_TCP_RECEIVE;
		else if( len == 0 )
			return HTTPARSER_ERROR_HTTP_TRUNCATION;
		
		gettimeofday( & t2 , NULL );
		AjustTimeval( & (e->timeout) , & t1 , & t2 );
		
		block_end = e->req_buf->ptr + len - 1 ;
		
		e->req_header.line_end = strpchr( e->req_buf->ptr , block_end , '\n' ) ;
		while( e->req_header.line_end )
		{
			*(e->req_header.line_end) = '\0' ;
			if( *(e->req_header.line_end-1) == '\r' )
				*(e->req_header.line_end-1) = '\0' ;
			
			if( *(e->req_header.line_begin) == '\0' )
			{
				e->req_header.header_total_len = e->req_header.line_end - e->req_buf->base + 1 ;
				e->req_body.body_remain_len = e->req_header.header_total_len + e->req_body.body_total_len - e->req_buf->len - len ;
				e->req_header.line_begin = e->req_header.line_end + 1 ;
				e->req_buf->len += len ;
				e->req_buf->ptr += len ;
				return 0;
			}
			
			nret = ParseHttpHeaderLine( e ) ;
			if( nret )
				return nret;
			
			e->req_header.line_begin = e->req_header.line_end + 1 ;
			
			e->req_header.line_end = strpchr( e->req_header.line_begin , block_end , '\n' ) ;
		}
		
		e->req_buf->len += len ;
		e->req_buf->ptr += len ;
	}
	
	return 0;
}

static int ReadHttpBody( int sock , struct HttpEnv *e )
{
	struct timeval	t1 , t2 ;
	fd_set		fds ;
	long		len ;
	int		nret = 0 ;
	
	if( e->req_buf->len + HTTPARSER_READBLOCK_SIZE_DEFAULT > e->req_buf->bufsize )
	{
		nret = ReallocHttpBuffer( e->req_buf , e->req_buf->bufsize * 2 ) ;
		if( nret )
			return nret;
	}
	
	while( e->req_body.body_remain_len > 0 )
	{
		gettimeofday( & t1 , NULL );
		
		FD_ZERO( & fds );
		FD_SET( sock , & fds );
		
		nret = select( sock + 1 , & fds , NULL , NULL , e->ptimeout ) ;
		if( nret == 0 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_RECEIVE_TIMEOUT;
		}
		else if( nret != 1 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_RECEIVE;
		}
		
		len = (long)read( sock , e->req_buf->ptr , e->req_body.body_remain_len ) ;
		if( len == -1 )
			return HTTPARSER_ERROR_TCP_RECEIVE;
		else if( len == 0 )
			return HTTPARSER_ERROR_HTTP_TRUNCATION;
		
		gettimeofday( & t2 , NULL );
		AjustTimeval( & (e->timeout) , & t1 , & t2 );
		
		e->req_buf->len += len ;
		e->req_buf->ptr += len ;
		
		e->req_body.body_remain_len -= len ;
	}
	
	return 0;
}

static int WriteHttpBuffer( int sock , struct HttpEnv *e , char *ptr , long total_len )
{
	struct timeval	t1 , t2 ;
	fd_set		fds ;
	long		len ;
	int		nret = 0 ;
	
	while( total_len > 0 )
	{
		gettimeofday( & t1 , NULL );
		
		FD_ZERO( & fds );
		FD_SET( sock , & fds );
		
		nret = select( sock + 1 , NULL , & fds , NULL , e->ptimeout ) ;
		if( nret == 0 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_SEND_TIMEOUT;
		}
		else if( nret != 1 )
		{
			return HTTPARSER_ERROR_TCP_SELECT_SEND;
		}
		
		len = (long)write( sock , ptr , total_len ) ;
		if( len == -1 )
			return HTTPARSER_ERROR_TCP_SEND;
		
		gettimeofday( & t2 , NULL );
		AjustTimeval( & (e->timeout) , & t1 , & t2 );
		
		ptr += len ;
		
		total_len -= len ;
	}
	
	return 0;
}

static int WriteHttpFd( int sock , struct HttpEnv *e , int fd )
{
	char		fd_buf[ HTTPARSER_READBLOCK_SIZE_DEFAULT + 1 ] ;
	struct timeval	t1 , t2 ;
	fd_set		fds ;
	long		len ;
	int		nret = 0 ;
	
	while(1)
	{
		gettimeofday( & t1 , NULL );
		
		if( e->rsp_append.select_flag )
		{
			FD_ZERO( & fds );
			FD_SET( fd , & fds );
			
			nret = select( sock + 1 , & fds , NULL , NULL , e->ptimeout ) ;
			if( nret == 0 )
			{
				return HTTPARSER_ERROR_FD_SELECT_READ_TIMEOUT;
			}
			else if( nret != 1 )
			{
				return HTTPARSER_ERROR_FD_SELECT_READ;
			}
		}
		
		len = (long)read( fd , fd_buf , HTTPARSER_READBLOCK_SIZE_DEFAULT ) ;
		if( len == -1 )
			return HTTPARSER_ERROR_FD_READ;
		else if( len == 0 )
			break;
		
		gettimeofday( & t2 , NULL );
		AjustTimeval( & (e->timeout) , & t1 , & t2 );
		
		nret = WriteHttpBuffer( sock , e , fd_buf , len ) ;
		if( nret )
			return nret;
	}
	
	return 0;
}

static int WriteHttpData( int sock , struct HttpEnv *e )
{
	int		nret = 0 ;
	
	nret = WriteHttpBuffer( sock , e , e->rsp_buf->base , e->rsp_buf->len ) ;
	if( nret )
		return nret;
	
	if( e->rsp_append.append_ptr )
	{
		nret = WriteHttpBuffer( sock , e , e->rsp_append.append_ptr , e->rsp_append.append_len ) ;
		if( nret )
			return nret;
	}
	
	if( e->rsp_append.append_fd_flag )
	{
		nret = WriteHttpFd( sock , e , e->rsp_append.append_fd ) ;
		if( nret )
			return nret;
	}
	
	return 0;
}

int InitHttpEnv( struct HttpEnv **e )
{
	(*e) = (struct HttpEnv *)malloc( sizeof(struct HttpEnv) );
	if( (*e) == NULL )
		return HTTPARSER_ERROR_ALLOC;
	memset( (*e) , 0x00 , sizeof(struct HttpEnv) );
	
	(*e)->req_buf = AllocHttpBuffer( HTTPARSER_REQUEST_BUFSIZE_DEFAULT ) ;
	if( (*e)->req_buf == NULL )
		return HTTPARSER_ERROR_ALLOC;
	
	(*e)->rsp_buf = AllocHttpBuffer( HTTPARSER_RESPONSE_BUFSIZE_DEFAULT ) ;
	if( (*e)->rsp_buf == NULL )
		return HTTPARSER_ERROR_ALLOC;
	
	return 0;
}

int PerformHttpParser( int sock , struct HttpEnv *e , long timeout , funcProcessHttpData *pfuncProcessHttpData , void *p )
{
	int	nret = 0 ;
	
	if( timeout > 0 )
	{
		e->timeout.tv_sec = timeout ;
		e->timeout.tv_usec = 0 ;
		e->ptimeout = & (e->timeout) ;
	}
	else
	{
		e->ptimeout = NULL ;
	}
	CleanHttpBuffer( e->req_buf );
	memset( & (e->req_header) , 0x00 , sizeof(struct HttpHeader) );
	memset( & (e->req_body) , 0x00 , sizeof(struct HttpBody) );
	CleanHttpBuffer( e->rsp_buf );
	memset( & (e->rsp_append) , 0x00 , sizeof(struct HttpAppend) );
	
	nret = ReadHttpHeader( sock , e ) ;
	if( nret )
		return nret;
	
	if( e->req_header.CONTENT_LENGTH )
	{
		nret = ReadHttpBody( sock , e ) ;
		if( nret )
			return nret;
	}
	
	nret = pfuncProcessHttpData( e , p ) ;
	if( nret )
		return nret;
	
	nret = WriteHttpData( sock , e ) ;
	if( nret )
		return nret;
	
	if( e->rsp_append.append_fd >= 0 && e->rsp_append.close_flag )
	{
		close( e->rsp_append.append_fd );
	}
	
	return 0;
}

int CleanHttpEnv( struct HttpEnv **e )
{
	if( (*e) )
	{
		FreeHttpBuffer( (*e)->req_buf );
		FreeHttpBuffer( (*e)->rsp_buf );
		free( (*e) );
		(*e) = NULL ;
	}
	
	return 0;
}

