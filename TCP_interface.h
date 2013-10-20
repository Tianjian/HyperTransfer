#ifndef _TCP_interface_H_
#define _TCP_interface_H_

namespace ftp_server
{
	class TCP_interface
	{

	public:
//		int listen_socket;	// FTP监听句柄

		struct event_base *base;

	private:
		TCP_manager *p_TCP_manager;

/*	public:
		const static int RET_CTRL_WRITE = 0x0001;
		const static int DATA_WRITE = 0x0004;
*/
	public:
		TCP_interface()
		{
			p_TCP_manager = new TCP_manager();
			p_TCP_manager -> set_ctrl_port(SERVERPORT);
			// 若配置项无设置，则使用默认端口
		};
		~TCP_interface()
		{
			delete p_TCP_manager;
			p_TCP_manager = NULL;
		};

	public:
		void set_ctrl_port(int port)
		{
			p_TCP_manager -> set_ctrl_port(port);
		}

		int get_ctrl_port()
		{
			return p_TCP_manager -> get_ctrl_port();
		}

		TCP_manager *get_TCP_manager()
		{
			return p_TCP_manager;
		};

		bool start_proc();

	public:
		int init_TCP_event(TCP_interface *p);
		int init_TCP_data_event(TCP_client *data_client);
	public:
		/** 设置检查超时的间隔时间和会话寿命 */
		void set_timeout( int chk_span , int session_life )  
		{ 
			if(p_TCP_manager == NULL)
	        {
		        LOG(FATAL,"set socket time out fail for TCP_Manger is not initial");
//			    cout<<"set socket time out fail for TCP_Manger is not initial"<<endl;
	        }
		    else
	        {
	            p_TCP_manager -> set_timeout(chk_span, session_life);
		    }
	    };

		void update_last_access(int fd)
		{
		    TCP_client *client = NULL;
        	client = p_TCP_manager -> seek_client( fd );
	        if(client == NULL)
    	    {
        	    LOG(FATAL,"Update client : "<<fd <<" is not exist");
	        }
    	    else
        	{
            	client -> update_last_access();
    	    }
			return ;
		}
	private:
		/** 控制连接事件：接受、通知、读、写、超时 */
		static void on_read(int fd, short ev, void *arg);
		static void on_write(int fd, short ev, void *arg);
		static void on_accept(int fd, short ev, void *arg);
		static void on_notify(int fd, short ev, void *arg);
    	static void on_timeout(int fd, short ev, void *arg);
		/** 数据连接事件：接受、读、写 */
		static void data_accept(int fd, short ev, void *arg);
		static void data_read(int fd, short ev, void *arg);
		static void data_write(int fd, short ev, void *arg);
		static void data_timeout(int fd, short ev, void *arg);


	public:
		void post(int fd, char *buf, int len )
		{
			_event_notify _notify;
	        _notify.fd = fd;
    	   	_notify.action = EVENT_NOTIFY_CTRL_WRITE;
			_notify.buf = new char[COMMANDSIZE];
        	strncpy(_notify.buf, buf, COMMANDSIZE);
			_notify.len = len;
			p_TCP_manager -> notify( _notify );
		};

		void post_transfer_ok(int fd, int action, char *buf, int len )
		{
			_event_notify _notify;
	        _notify.fd = fd;
    	   	_notify.action = action;
			_notify.buf = new char[COMMANDSIZE];
        	strncpy(_notify.buf, buf, COMMANDSIZE);
			_notify.len = len;
			p_TCP_manager -> notify( _notify );
		};

		void post_action(int fd, int action)
		{
			_event_notify _notify;
	        _notify.fd = fd;
    	   	_notify.action = action;
			p_TCP_manager -> notify( _notify );
		};

		void data_post(int cmd_fd, char *buf, int len )
		{
			_event_notify _notify;
	        _notify.fd = cmd_fd;
    	   	_notify.action = EVENT_NOTIFY_DATA_WRITE;
			_notify.len = len;
			_notify.buf = new char[len];
			memcpy(_notify.buf, buf, len);
			p_TCP_manager -> notify( _notify );
		};
/*		void close_fd(int fd)
		{
			_event_notify _notify;
			_notify.fd = fd;
			_notify.action = TCP_ACTION_DELETE;
			_notify.buf = NULL;
			_notify.len = 0;
        	p_TCP_manager -> notify( _notify );
		};
*/	};
}
#endif
