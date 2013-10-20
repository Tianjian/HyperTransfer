#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

using namespace std;
namespace ftp_server
{
	class TCP_client
	{
	public:
		int ctrl_socket;
//		int data_socket;

		int listen_fd;	//	数据连接监听描述符
		int data_fd;	//	数据连接描述符

		int data_action;

    	TAILQ_HEAD( , _buffer_q ) write_q;
		/** 存储写数据的队列 */
    	TAILQ_HEAD( , _data_buffer_q ) data_write_q;

		char temp_data[DATASIZE];	// 数据接收临时区
		int temp_data_len;				// 临时数据的长度
		char file_buffer[FILEBUFSIZE];	// 文件数据缓存区
		int buf_len;					// 缓存数据长度

	public:
		struct event ev_read;
		struct event ev_write;

		/** 数据连接事件：接受、通知、读、写 */
		struct event event_data_accept;	
		struct event event_data_notify;
		struct event event_data_read;
		struct event event_data_write;

	private:
		time_t last_access;

//		pthread_mutex_t s_mutex;		

	public:
		TCP_client( int ctrl_fd )
		{
			ctrl_socket = ctrl_fd;
			listen_fd = -1;
			data_fd = -1;
			last_access = time(NULL);
            TAILQ_INIT( &write_q );
			TAILQ_INIT( &data_write_q );

//			pthread_mutex_init(&s_mutex, NULL);


			temp_data_len = 0;	// 临时数据长度为0
			buf_len = 0;		// 缓冲区长度为0

		};

		~TCP_client()
		{
			clear_ctrlfd();
			clear_datafd();
	        struct _buffer_q *bufq;
    	    bufq = TAILQ_FIRST(&write_q);
			while( !TAILQ_EMPTY(&write_q) )
			{
				bufq = TAILQ_FIRST(&write_q);
				TAILQ_REMOVE( &write_q, bufq, entries);
				if( bufq->buf != NULL )
				{
					delete[] bufq->buf;
					bufq->buf = NULL;
				}
        		delete bufq;
				bufq = NULL;
	        }
			struct _data_buffer_q *data_bufq;
			while( !TAILQ_EMPTY(&data_write_q) )
			{
				data_bufq = TAILQ_FIRST(&data_write_q);
				TAILQ_REMOVE( &data_write_q, data_bufq, data_entries);
				if( data_bufq->buf != NULL )
				{
//					delete[] bufq->buf;
					data_bufq -> buf = NULL;
				}
        		delete data_bufq;
				data_bufq = NULL;
	        }

//			pthread_mutex_destroy(&s_mutex);

		};
	public:
		int get_listen_fd()
		{
			return listen_fd;
		};

		void set_listen_fd(int fd)
		{
			listen_fd = fd;
//			cout<<"set_listen_fd():"<<listen_fd<<endl;
		};
		
		int get_ctrl_socket()
		{
			return ctrl_socket;
		};

		int get_data_action()
		{
			return data_action;
		};

		void set_data_action(int new_action)
		{
			data_action = new_action;
		};

		int get_data_fd()
		{
			return data_fd;
		};

		void set_data_fd(int fd)
		{
			data_fd = fd;
		};

		int read_data()
		{
			temp_data_len = read(data_fd, temp_data, DATASIZE);
			
			return temp_data_len;
		};

		void copy_data()
		{
			memcpy(file_buffer + buf_len, temp_data, temp_data_len);
			buf_len += temp_data_len;
			memset(temp_data, 0, DATASIZE);
		};
				
		void clear_data()
		{
			buf_len = 0;
			memset(file_buffer, 0, FILEBUFSIZE);
		};

		char* get_file_buffer()
		{
			return file_buffer;
		};

		void set_buf_len(int size)
		{
			buf_len = size;
		};

		int get_buf_len()
		{
			return buf_len;
		};

		void update_last_access()
		{
			last_access=time(NULL);
	    };

		time_t get_last_access()
	    {
		    return last_access;
	    };

		void clear_ctrlfd()
		{
			if(ctrl_socket != -1)
			{
				LOG(DEBUG, "event_del: ctrl_read"<<event_del( &ev_read ));				
				LOG(DEBUG, "event_del: ctrl_write"<<event_del( &ev_write ));
				close(ctrl_socket);
				ctrl_socket = -1;
			}
		};

		void clear_datafd()
		{
			if(listen_fd != -1)
			{
				LOG(DEBUG, "event_del: data_accept"<<event_del(&event_data_accept));	
//				cout<<"close(listen_fd):"<<listen_fd<<endl;
				//int ret = 
				close(listen_fd);
//				cout<<"ret:"<<ret<<" close(listen_fd):"<<listen_fd<<endl;
				listen_fd = -1;
			}

			if(data_fd != -1)
			{
				LOG(DEBUG, "event_del: data_write"<<event_del(&event_data_write));
				LOG(DEBUG, "event_del: data_read"<<event_del(&event_data_read));
//				cout<<"close(data_fd):"<<data_fd<<endl;
				close(data_fd);
				data_fd = -1;				
			}
		};

	};
}

#endif /* _TCP_CLIENT_H_ */
