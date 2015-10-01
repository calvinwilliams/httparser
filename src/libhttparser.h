/*
 * libhttparser
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_HTTPARSER_
#define _H_HTTPARSER_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/time.h>

#if ( defined _WIN32 )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC            _declspec(dllexport)
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC
#endif
#endif

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef STRICMP
#if ( defined _WIN32 )
#define STRICMP(_a_,_C_,_b_) ( stricmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strnicmp(_a_,_b_,_n_) _C_ 0 )
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )
#endif
#endif

#ifndef MEMCMP
#define MEMCMP(_a_,_C_,_b_,_n_) ( memcmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef SNPRINTF
#if ( defined _WIN32 )
#define SNPRINTF        _snprintf
#define VSNPRINTF       _vsnprintf
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ )
#define SNPRINTF        snprintf
#define VSNPRINTF       vsnprintf
#endif
#endif

#define HTTPARSER_ERROR_ALLOC				-11
#define HTTPARSER_ERROR_TCP_SELECT_RECEIVE		-31
#define HTTPARSER_ERROR_TCP_SELECT_RECEIVE_TIMEOUT	-32
#define HTTPARSER_ERROR_TCP_RECEIVE			-33
#define HTTPARSER_ERROR_TCP_SELECT_SEND			-41
#define HTTPARSER_ERROR_TCP_SELECT_SEND_TIMEOUT		-42
#define HTTPARSER_ERROR_TCP_SEND			-43
#define HTTPARSER_ERROR_FD_SELECT_READ			-51
#define HTTPARSER_ERROR_FD_SELECT_READ_TIMEOUT		-52
#define HTTPARSER_ERROR_FD_READ				-53
#define HTTPARSER_ERROR_HTTP_TRUNCATION			-101
#define HTTPARSER_ERROR_HTTP_HEADERLINE_INVALID		-102
#define HTTPARSER_ERROR_HTTP_PROCESSING			-104

#define HTTPARSER_READBLOCK_SIZE_DEFAULT		4*1024
#define HTTPARSER_REQUEST_BUFSIZE_DEFAULT		16*1024
#define HTTPARSER_RESPONSE_BUFSIZE_DEFAULT		16*1024

struct HttpBuffer ;
struct HttpEnv ;

typedef int funcProcessHttpData( struct HttpEnv *e , void *p );

int InitHttpEnv( struct HttpEnv **e );
int PerformHttpParser( int sock , struct HttpEnv *e , long timeout , funcProcessHttpData *pfuncProcessHttpData , void *p );
int CleanHttpEnv( struct HttpEnv **e );

#define HTTPARSER_SUPPORT_VERSION	"HTTP/1.1"
#define HTTPARSER_STATUSCODE_100	HTTPARSER_SUPPORT_VERSION " " "100 Continue\r\n"
#define HTTPARSER_STATUSCODE_101	HTTPARSER_SUPPORT_VERSION " " "101 Switching Protocol\r\n"
#define HTTPARSER_STATUSCODE_200	HTTPARSER_SUPPORT_VERSION " " "200 OK\r\n"
#define HTTPARSER_STATUSCODE_201	HTTPARSER_SUPPORT_VERSION " " "201 Created\r\n"
#define HTTPARSER_STATUSCODE_202	HTTPARSER_SUPPORT_VERSION " " "202 Accepted\r\n"
#define HTTPARSER_STATUSCODE_203	HTTPARSER_SUPPORT_VERSION " " "203 Non Authoritative Information\r\n"
#define HTTPARSER_STATUSCODE_204	HTTPARSER_SUPPORT_VERSION " " "204 No Content\r\n"
#define HTTPARSER_STATUSCODE_205	HTTPARSER_SUPPORT_VERSION " " "205 Reset Content\r\n"
#define HTTPARSER_STATUSCODE_206	HTTPARSER_SUPPORT_VERSION " " "206 Partial Content\r\n"
#define HTTPARSER_STATUSCODE_300	HTTPARSER_SUPPORT_VERSION " " "300 Multiple Choices\r\n"
#define HTTPARSER_STATUSCODE_301	HTTPARSER_SUPPORT_VERSION " " "301 Moved Permannetly\r\n"
#define HTTPARSER_STATUSCODE_302	HTTPARSER_SUPPORT_VERSION " " "302 Found\r\n"
#define HTTPARSER_STATUSCODE_303	HTTPARSER_SUPPORT_VERSION " " "303 See Other\r\n"
#define HTTPARSER_STATUSCODE_304	HTTPARSER_SUPPORT_VERSION " " "304 Not Modified\r\n"
#define HTTPARSER_STATUSCODE_305	HTTPARSER_SUPPORT_VERSION " " "305 Use Proxy\r\n"
#define HTTPARSER_STATUSCODE_307	HTTPARSER_SUPPORT_VERSION " " "307 Temporary Redirect\r\n"
#define HTTPARSER_STATUSCODE_400	HTTPARSER_SUPPORT_VERSION " " "400 Bad Request\r\n"
#define HTTPARSER_STATUSCODE_401	HTTPARSER_SUPPORT_VERSION " " "401 Unauthorized\r\n"
#define HTTPARSER_STATUSCODE_402	HTTPARSER_SUPPORT_VERSION " " "402 Payment Required\r\n"
#define HTTPARSER_STATUSCODE_403	HTTPARSER_SUPPORT_VERSION " " "403 Forbidden\r\n"
#define HTTPARSER_STATUSCODE_404	HTTPARSER_SUPPORT_VERSION " " "404 Not Found\r\n"
#define HTTPARSER_STATUSCODE_405	HTTPARSER_SUPPORT_VERSION " " "405 Method Not Allowed\r\n"
#define HTTPARSER_STATUSCODE_406	HTTPARSER_SUPPORT_VERSION " " "406 Not Acceptable\r\n"
#define HTTPARSER_STATUSCODE_407	HTTPARSER_SUPPORT_VERSION " " "407 Proxy Authentication Required\r\n"
#define HTTPARSER_STATUSCODE_408	HTTPARSER_SUPPORT_VERSION " " "408 Request Timeout\r\n"
#define HTTPARSER_STATUSCODE_409	HTTPARSER_SUPPORT_VERSION " " "409 Conflict\r\n"
#define HTTPARSER_STATUSCODE_410	HTTPARSER_SUPPORT_VERSION " " "410 Gone\r\n"
#define HTTPARSER_STATUSCODE_411	HTTPARSER_SUPPORT_VERSION " " "411 Length Request\r\n"
#define HTTPARSER_STATUSCODE_412	HTTPARSER_SUPPORT_VERSION " " "412 Precondition Failed\r\n"
#define HTTPARSER_STATUSCODE_413	HTTPARSER_SUPPORT_VERSION " " "413 Request Entity Too Large\r\n"
#define HTTPARSER_STATUSCODE_414	HTTPARSER_SUPPORT_VERSION " " "414 Request URI Too Long\r\n"
#define HTTPARSER_STATUSCODE_415	HTTPARSER_SUPPORT_VERSION " " "415 Unsupported Media Type\r\n"
#define HTTPARSER_STATUSCODE_416	HTTPARSER_SUPPORT_VERSION " " "416 Requested Range Not Satisfiable\r\n"
#define HTTPARSER_STATUSCODE_417	HTTPARSER_SUPPORT_VERSION " " "417 Expectation Failed\r\n"
#define HTTPARSER_STATUSCODE_500	HTTPARSER_SUPPORT_VERSION " " "500 Internal Server Error\r\n"
#define HTTPARSER_STATUSCODE_501	HTTPARSER_SUPPORT_VERSION " " "501 Not Implemented\r\n"
#define HTTPARSER_STATUSCODE_502	HTTPARSER_SUPPORT_VERSION " " "502 Bad Gateway\r\n"
#define HTTPARSER_STATUSCODE_503	HTTPARSER_SUPPORT_VERSION " " "503 Service Unavailable\r\n"
#define HTTPARSER_STATUSCODE_504	HTTPARSER_SUPPORT_VERSION " " "504 Gateway Timeout\r\n"
#define HTTPARSER_STATUSCODE_505	HTTPARSER_SUPPORT_VERSION " " "505 HTTP Version Not Supported\r\n"

int StrcpyHttpStatusCode( struct HttpEnv *e , char *status_code );
int StrcatHttpResponseBuffer( struct HttpEnv *e , char *str );
int StrcatfHttpResponseBuffer( struct HttpEnv *e , char *format , ... );
void CleanHttpResponseBuffer( struct HttpEnv *e );

void SetHttpResponseAppendPtr( struct HttpEnv *e , char *ptr , long len );
void SetHttpResponseAppendFd( struct HttpEnv *e , int append_fd , int select_flag , int close_flag );

char *GetRequestHeader_METHOD( struct HttpEnv *e );
char *GetRequestHeader_URL( struct HttpEnv *e );
char *GetRequestHeader_VERSION( struct HttpEnv *e );
char *GetRequestHeader_CONTENT_LENGTH( struct HttpEnv *e );
char *GetRequestHeader_COOKIE( struct HttpEnv *e );
char *GetRequestHeader_USER_AGENT( struct HttpEnv *e );
char *GetRequestHeader_HOST( struct HttpEnv *e );
char *GetRequestHeader_REFERER( struct HttpEnv *e );
char *GetRequestHeader_CONNECTION( struct HttpEnv *e );
char *GetRequestHeader_ACCEPT( struct HttpEnv *e );
char *GetRequestHeader_ACCEPT_LANGUAGE( struct HttpEnv *e );
char *GetRequestHeader_ACCEPT_ENCODING( struct HttpEnv *e );

char *GetRequestBodyBase( struct HttpEnv *e );
long GetRequestBodyLength( struct HttpEnv *e );

int SetRequestBufferSize( struct HttpEnv *e , long bufsize );
int SetResponseBufferSize( struct HttpEnv *e , long bufsize );

#endif

