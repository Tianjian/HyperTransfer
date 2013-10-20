#ifndef _TCP_MANAGER_H_
#define _TCP_MANAGER_H_

namespace ftp_server
{
	class TCP_manager
	{
	public:
		int ctrl_port;		// 控制连接监听端口号
		int listen_socket;
		int notify_fd;
		int post_fd;

		struct event event_accept; 
		struct event event_notify;
		struct event ev_timeout;
	private:
		map<int , TCP_client *>  m_client_map;

		int _socket_span ;
	    int _socket_life;

	private:
		pthread_mutex_t _lock;
	public:
		int init_listen_socket();
		TCP_manager()
		{
		    _socket_span = 10;
			_socket_life = 120;
			pthread_mutex_init(&_lock, NULL);
		};
		void set_ctrl_port(int port)
		{
			ctrl_port = port;
		}

		int get_ctrl_port()
		{
			return ctrl_port;
		}
		int get_socket_span()
		{
			return _socket_span;
		};

		int get_socket_life()
		{
			return _socket_life;
		};

		void get_timeout_client(vector<int>& fd_list )   
		{       
			fd_list.clear();        
	        time_t now = time(NULL);        
		    map< int , TCP_client *>::iterator iter;     
			for( iter = m_client_map.begin(); iter != m_client_map.end() ; iter++ )     
	        {           
		        time_t last_access = (iter->second) -> get_last_access();
			    if( _socket_life + last_access < now )            
				{
					fd_list.push_back( (iter->second) -> ctrl_socket );           
	            }   
		    }
	    };

		void set_timeout( int chk_span, int ctx_life )
	    {
		    _socket_span = chk_span ;
			_socket_life = ctx_life ;
	    };

		bool add_client(TCP_client *client, int fd);
		TCP_client *seek_client(int fd);
		bool del_client( int fd );

		void set_socketpair(int n_fd, int p_fd)
		{
			notify_fd = n_fd;
			post_fd = p_fd;
		}

		void notify( _event_notify& _notify )
		{
			pthread_mutex_lock( &_lock ); 
			write( post_fd , (void *)&_notify , sizeof(_notify) );
			pthread_mutex_unlock( &_lock ); 
		};
	};
}
#endif /* _TCP_MANAGER_H_ */
