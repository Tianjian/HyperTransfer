#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_
#include "common_include.h"

using namespace std;

#define COMMANDSIZE 1461	//max length is 1460 Byte plus 1 byte string terminator
#define SERVERPORT	8899

#define SMALL_FILE		134217728	//      small file size in bytes
#define FILE_BLOCK		33554432	//      slice size of large file ji cd
#define READ_BLOCK		1024	//	file reading buffer size
#define FILESIZE_CHARLEN	16	//	maxim text length of file length numbers
#define MD5SUM_CHARLEN	34		//	maxim text length of md5 checksum
#define FILE_SIGNATURE	106		//	large file signature buffer length

#define PORT_PART_LEN	4	//	PASV mode return code length
#define IPLEN		16	// IP string length
#define PASV_RET_IPLEN	17	// IP string length for PASV mode
#define USERLEN		16	// username string length
#define PASSLEN		128	// password string length
#define PATHSIZE 	1024	// maxim file path length
#define FILENAMESIZE	256	// maxim file name length

#define USERLISTPATH	"conf/userlist"	// user authenfication file pathname

typedef struct 
{
    string  server_home;
    string  log_config_file; // Log4cpp File
	int session_thread_num;
	int disk_thread_num;
	int	disk_num;
	int client_life;
	int time_interval;
	int server_port;
	string server_LAN_ip;
	string server_WAN_ip;
	string init_path;
	string auth_ip;
	int auth_port;
	int start_port;
	int end_port;
}_ftpserver_config;

typedef struct 
{
    string  client_home;
    string  log_config_file; // Log4cpp File
}_ftpclient_config;

typedef struct
{
    char user[USERLEN];	//username
    char pass[PASSLEN];	//password
	bool upload_per;
	bool download_per;
	bool dele_per;

}_userlist;	//node of user info

struct _buffer_q
{               
    /* The buffer. */
    char *buf;
            
    /* The length of buf. */
    int len;

	/* The offset of buf  */
    int offset;

    /* The Next node of chain */
    TAILQ_ENTRY(_buffer_q) entries;
};

struct _data_buffer_q
{               
    /* The buffer. */
    char *buf;
            
    /* The length of buf. */
    int len;

	/* The offset of buf  */
    int offset;

    /* The Next node of chain */
    TAILQ_ENTRY(_data_buffer_q) data_entries;
};

typedef struct dir_lnk
{
        char d_name[FILENAMESIZE];    //
        struct dir_lnk * next;
}dirlnk;

typedef struct item_info
{
	char permission[11];   //
	time_t mod_time;       //
}info;

#define THREADSIZE 		8*1024*1024

#define STACKSIZE 		8*1024*1024
#define SESSION_THREAD_NUM 8
#define SPTHREADNUM 	5
#define DISKNUM 		2
#define FILEBUFSIZE		1048576	//	1024*1024
#define FILEBUFWTIRESIZE		1047116	//	1024*1024-1460

#define MAXCONN 		15000
#define INDEXSIZE		10240	//1024*10
#define FILEINFOLEN     320 //

#define DATASIZE		1460	//TCP networking read buffer size

/** session ctrl action */
#define CTRL_WAITING				0x00	// ctrl channel standby
#define CTRL_PASV				0x02	// ctrl channel in PASV mode

/** session data action */
#define DATA_WAITING				0x40	// data channel in PASV mode
#define DATA_PASV_OK				0x41	// data channel established
#define DATA_LIST					0x42	
#define DATA_STOR					0x43			
#define DATA_RETR					0x44		
#define DATA_INDEX_DOWNLOAD			0x45	// start downloading directory infomation
#define DATA_FILE_DOWNLOAD			0x46	// start downloading file



/** disk data action */
#define DISK_STOR_START				0x80	// disk thread standby
#define DISK_STORING				0x81	// busy in storage operation
#define DISK_STOR_DONE				0x82	// compelete a storage operation
#define DISK_INDEX_LOADING			0x83	// query directory information
#define DISK_INDEX_FINISH			0x84	// get directory information
#define DISK_RETR_LOADING			0x85	// query file data
#define DISK_RETR_FINISH			0x86	// get file data

/** session_task action */
#define SESSION_TASK_CTRL_ACCEPT	0xC0	// new client establishes a new ctrl channel
#define SESSION_TASK_READ			0xC1	//	new command received
#define SESSION_TASK_PASV			0xC2	//	complete PASV mode preparation
#define SESSION_TASK_DATA_ACCEPT	0xC4	//	new client establises a new data channel
#define SESSION_TASK_STOR_START		0xC5	//	complete data channel construction
#define SESSION_TASK_TRANS_OK		0xC6	//	complete data transfer, return 226
#define SESSION_TASK_DISCONN		0xC8	//	client exit normally, read = 0
#define SESSION_TASK_CONN_ERR		0xC9	//	client exit abnormally, read < 0

/** event_notify action */
#define EVENT_NOTIFY_CTRL_WRITE		0xD0	//	write event in ctrl channel
#define EVENT_NOTIFY_PASV			0xD1	//	PASV event
#define EVENT_NOTIFY_TRANS_OK		0xD2	//	data transfer compeleted
#define EVENT_NOTIFY_DOWN_START		0xD3	//	data downloading starts
#define EVENT_NOTIFY_DATA_DELETE	0xD4	//	data delete starts
#define EVENT_NOTIFY_DATA_WRITE		0xD5	//	data modify starts

/** data_task action */
#define DATA_TASK_STOR_START		0xE0	//	new storage task preparation
#define DATA_TASK_STORING			0xE1	//	file uploading
#define DATA_TASK_STOR_OK			0xE2	//	file uploading complete
#define DATA_TASK_WRITE_DONE		0xE3	//	data downloading complete
#define DATA_TASK_INDEX_START		0xE4	//	file directory query
#define DATA_TASK_RETR_START		0xE5	//	file downloading start
#define DATA_TASK_DELE				0xE6	//	file deletetion

#define CLIENTNUM 1000 			// maxim client num
#define N_BITS 3

/* event notification data structure*/
typedef struct
{
	int fd;     // client fd that this event binded to
	int action; // action type
	char* buf;  // data buffer
	int len;    // data buffer length
}_event_notify;

#endif
