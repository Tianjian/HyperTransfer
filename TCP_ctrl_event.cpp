#include "common_include.h"
#include "common_def.h"

#include "TCP_client.h"
#include "TCP_manager.h"
#include "TCP_interface.h"
#include "TCP_tool.h"

#include "session_manager.h"


#include "file_index.h"
#include "disk_task.h"
#include "disk_info.h"
#include "disk_pending_pool.h"
#include "disk_fsm.h"
#include "disk_thread.h"
#include "disk_thread_pool.h"
#include "disk_manager.h"


using namespace std;

namespace ftp_server
{
	extern session_manager *g_session_man;
	extern disk_manager *g_disk_man;
	extern _ftpserver_config g_config;
	extern TCP_interface *g_tcp;
	extern TCP_tool *g_tcp_tool;

	int TCP_interface::init_TCP_event(TCP_interface *p_TCP_i)
	{
        TCP_manager *p_TCP_m = p_TCP_i -> get_TCP_manager();
		
		int pair_fd[2];
//		cout<<"base:"<<p_TCP_i -> base<<endl;	
		p_TCP_i->base = event_init();	//initial libevent

		event_set(&p_TCP_m -> event_accept, p_TCP_m -> listen_socket,
			EV_READ|EV_PERSIST,	on_accept, (void *)p_TCP_m);

		event_add(&p_TCP_m -> event_accept, NULL);

		socketpair(AF_UNIX, SOCK_STREAM, 0, pair_fd);
//cout<<"create:"<<pair_fd[0]<<" "<<pair_fd[1]<<endl;
		// 套接字的域  套接字类型 使用的协议 指向文件描述符的指针
		if (g_tcp_tool -> set_nonblock(pair_fd[0]) < 0)
		{
			LOG(WARN, "Failed to set notify socket to non-blocking.");
		}

		p_TCP_m -> set_socketpair(pair_fd[0], pair_fd[1]);

		event_set(&p_TCP_m -> event_notify, pair_fd[0], EV_READ|EV_PERSIST,
			on_notify, (void *)p_TCP_m);

		event_add(&p_TCP_m -> event_notify, NULL);
		LOG(DEBUG, "Init listen events success.");

		//here add timeout callback event
	    struct timeval tv;

		evtimer_set(&(p_TCP_m -> ev_timeout), on_timeout, (void*)p_TCP_m);
	    evutil_timerclear(&tv);

		tv.tv_sec = p_TCP_m -> get_socket_span();
	    event_add(&(p_TCP_m -> ev_timeout), &tv);

		event_dispatch();

		return 0;
	}

	void TCP_interface::on_accept(int fd, short ev, void *arg)
	{
        TCP_manager *p_TCP_m = (TCP_manager *)arg;

		/** 接受客户端并获取相关信息 */
		TCP_client *client;
		int client_fd;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
//		cout<<"on_accept():"<<client_fd<<endl;
		
		if(client_fd == -1)
		{
			LOG(WARN, "Accept failed");
			return ;
		}
		else
		{
			;
		}

		/** 设置客户端socket为非阻塞 */
		if(g_tcp_tool -> set_nonblock(client_fd) < 0)
		{
			LOG(WARN, "Failed to set client socket non-blocking");
		}
		else
		{
			;
		}

        int flag = 1;
        int result = setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, 
			(char *)&flag, sizeof(int));
		/* socket affected | set option at TCP level | name of option | 
		the cast is historical cruft */
		if( result < 0)
		{	
			LOG(WARN, "Canot set fd "<<client_fd<<" to nodelay "<<strerror(errno));
		}
		else
		{
			;
		}
 
		/** 构造一个客户端并添加到管理器中 */	
		client = new TCP_client(client_fd);
		if(client == NULL)
		{
			LOG(WARN, "Construct client failed");
		}

		p_TCP_m -> add_client(client, client_fd);
		
		/** 初始化on_read事件 */
		event_set(&client->ev_read, client_fd, EV_READ | EV_PERSIST, on_read, (void*)p_TCP_m);

		/** 激活on_read事件 */
		event_add(&client->ev_read, NULL);

		/** 初始化on_write事件 */
		event_set(&client->ev_write, client_fd, EV_WRITE, on_write, (void*)p_TCP_m);

		LOG(INFO, "Accepted connection from "<<inet_ntoa(client_addr.sin_addr));

		/** 设置一个事件，通知有新用户 */
		session_task *event_task = new session_task(client_fd, SESSION_TASK_CTRL_ACCEPT, inet_ntoa(client_addr.sin_addr), IPLEN + 1);

		g_session_man -> post_to_session( event_task );

        return ;
	}
	
	void TCP_interface::on_notify(int fd, short ev, void *arg)
    {
		TCP_manager *p_TCP_m = (TCP_manager *)arg;
		TCP_client *client = NULL;

		_event_notify tcp_notify;
		tcp_notify.buf = NULL;
		tcp_notify.len = 0;

		int len = 0;

		len = read(fd, (char*)&tcp_notify, sizeof(tcp_notify));
	    if (len != sizeof(tcp_notify)) 
		{
			LOG(WARN , "Socket Pair Error in Reading, Len: "<<len );
			return;
		}
    	client = p_TCP_m -> seek_client( tcp_notify.fd );
		client -> update_last_access();

    	if ( client == NULL )
	    {
    	   	LOG(WARN , "Cannot Seek Client By FD: "<< tcp_notify.fd);
			//cout<<"Cannot Seek Client By FD: "<<tcp_notify.fd<<endl;
			if(tcp_notify.buf != NULL)
			{
				delete []tcp_notify.buf;
				tcp_notify.buf=NULL;
			}
        	return;
		}

		if ( EVENT_NOTIFY_CTRL_WRITE == tcp_notify.action)
		{
			struct _buffer_q *buf_q = new struct _buffer_q;
 			if (buf_q == NULL)
			{
				LOG(WARN, "Construct bufferq faild");
				return;
			}
			buf_q->buf = tcp_notify.buf;
			buf_q->len = tcp_notify.len;
			buf_q->offset = 0;

			TAILQ_INSERT_TAIL(&client->write_q, buf_q, entries);
			event_add(&client->ev_write, NULL);
			return;
		}
		else
		if (EVENT_NOTIFY_PASV == tcp_notify.action)
		{
			//存储该事件的信息：客户端句柄、事件动作、缓冲区内容及长度

			int port_num;
      
            do{ 
				port_num = g_tcp_tool -> get_free_port(client);  //  获取一个空闲端口
				if(port_num == -1)
				{
					LOG(WARN, "get_free_port 出错，要处理");
					return;
				}
            }while(port_num < g_config.start_port || port_num > g_config.end_port);

			g_tcp -> init_TCP_data_event(client);

			session_task *event_task = new session_task(tcp_notify.fd);
			event_task -> set_action(SESSION_TASK_PASV);
			event_task -> set_buf_len(port_num);

			g_session_man -> post_to_session( event_task );
			return;
		}
		else
		if (EVENT_NOTIFY_TRANS_OK == tcp_notify.action)
		{
//			cout<<"EVENT_NOTIFY_TRANS_OK == tcp_notify.action"<<endl;
			struct _buffer_q *buf_q = new struct _buffer_q;
 			if (buf_q == NULL)
			{
				LOG(WARN, "Construct bufferq faild");
				return;
			}
			buf_q->buf = tcp_notify.buf;
			buf_q->len = tcp_notify.len;
			buf_q->offset = 0;

			TAILQ_INSERT_TAIL(&client->write_q, buf_q, entries);
			event_add(&client->ev_write, NULL);

			client -> clear_datafd();

			return;
		}
		else
		if(EVENT_NOTIFY_DATA_WRITE == tcp_notify.action)
		{
			struct _data_buffer_q *buf_q = new struct _data_buffer_q;
 			if (buf_q == NULL)
			{
				LOG(WARN, "Construct bufferq faild");
				return;
			}
			buf_q->buf = tcp_notify.buf;
			buf_q->len = tcp_notify.len;
			buf_q->offset = 0;

			TAILQ_INSERT_TAIL(&client->data_write_q, buf_q, data_entries);
			event_add(&client->event_data_write, NULL);

			return;
		}
		else
		{
			LOG(WARN, "Unknown Action : "<< tcp_notify.action );
			return;
		}
		return ;
    }


	void TCP_interface::on_read(int fd, short ev, void *arg)
	{
		TCP_manager *p_TCP_m = (TCP_manager *)arg;

		//存储该事件的信息：客户端句柄、事件动作、缓冲区内容及长度
		session_task *event_task = new session_task(fd, COMMANDSIZE);

		int len = read(fd, event_task -> get_buf(), COMMANDSIZE);
    	event_task -> set_buf_len(len);

		if (len == 0)
		{
//			printf("read len = 0");
//			cout<<"Client disconnected.fd="<<fd<<endl;
			LOG(WARN, "Client disconnected. fd="<<fd);
			p_TCP_m -> del_client(fd);
			
//			event_notify.fd = fd;
			event_task -> set_action(SESSION_TASK_DISCONN);
			g_session_man -> post_to_session( event_task );
			return;
		}
		else
		if (len < 0)
		{
//			printf("read len < 0");
	
			LOG(WARN, "Socket failure, disconnecting client: "<<strerror(errno));
			p_TCP_m->del_client(fd);

//			event_notify.fd = fd;
			event_task -> set_action(SESSION_TASK_CONN_ERR);
			g_session_man -> post_to_session( event_task );
			return;
		}
		else
		{
    	    TCP_client *client = NULL;
        	client = p_TCP_m -> seek_client( fd );
	        if(client == NULL)
    	    {
        	    LOG(FATAL, "Update client : "<<fd <<" is not exist");
	        }
    	    else
        	{
            	client -> update_last_access();
//				printf("on_read:%d %s\n",event_notify.fd, event_notify.buf);
    	    }
	        event_task -> set_action(SESSION_TASK_READ);
//			printf(" on_read : %s\n", event_task -> get_buf());
			g_session_man -> post_to_session( event_task );
			return ;
		}
	}

    void TCP_interface::on_write(int fd, short ev, void *arg)
    {
    	TCP_manager* p_TCP_m = (TCP_manager *)arg;
    	TCP_client *client = p_TCP_m -> seek_client(fd);
  		//存储该事件的信息：客户端句柄、事件动作、缓冲区内容及长度
		session_task *event_task = new session_task(fd, COMMANDSIZE);
	  	if( client == NULL )
    	{
    		LOG(WARN, "Client FD "<<fd<<" is no longer available");
    		return;
    	}

     	struct _buffer_q *buf_q;
		int len;

		buf_q = TAILQ_FIRST(&client -> write_q);

    	if (buf_q == NULL)
    	{
   			return;
    	}

    	len = write(fd, buf_q -> buf + buf_q ->offset, buf_q->len - buf_q->offset);
		
//printf("on_write:fd:%d.len:%d.buf:%s.\n",fd,buf_q->len,buf_q->buf);
		
		if (len < 0 )
    	{
 /*   		if (errno == EINTR || errno == EAGAIN)
    		{
    			event_add(&client->ev_write, NULL);
    			return;
    		}
    		else
    		{*/
			LOG(WARN, "Error in socket write, close FD: "<<fd);
			p_TCP_m -> del_client(fd);

			event_task -> set_action(SESSION_TASK_CONN_ERR);
			g_session_man -> post_to_session( event_task );
			return;
 //   		}
    	}
    	else
    	if ((buf_q->offset + len) < buf_q->len)
    	{
    		buf_q->offset += len;
    		event_add(&client->ev_write, NULL);

            if(client==NULL)
            {
				LOG(FATAL,"Update client : "<<fd <<" whiel don't exist");
            }
            else
            {
    	       	client -> update_last_access();
            }
    		return;
    	}
    	else
    	{
    		TAILQ_REMOVE(&client->write_q, buf_q, entries);
        	if( buf_q->buf != NULL )
            {
				delete[] buf_q->buf;
				buf_q -> buf = NULL;

			}

	   		delete buf_q;

			buf_q = NULL;
    	    if(client == NULL)
            {
    	        LOG(WARN, "Update client : "<<fd <<" whiel don't exist");
            }
    	    else
            {
    	        client -> update_last_access();
            }
    		return;
    	}
    }

    void TCP_interface::on_timeout(int fd, short ev, void *arg)
    {
	    TCP_manager* p_TCP_m = (TCP_manager *)arg;
		//存储该事件的信息：客户端句柄、事件动作、缓冲区内容及长度

		/** 获取超时客户端 */
	    vector<int> fds;
		p_TCP_m -> get_timeout_client( fds );
		// 输入客户端寿命 和 存储客户端描述符的容器

		LOG(WARN , "Eliminate "<< fds.size() <<" fds from TCP_interface.");
		cout<<fds.size()<<" client is timeout."<<endl;
	    // 记录超时客户端数量

		/** 逐一删除客户端及会话 */
		if(!fds.empty())
		{
			for( unsigned int i = 0 ; i < fds.size() ; i++ )        
			{   
		        LOG(INFO, "Delete Client : "<< fds[i] );
	
			    p_TCP_m -> del_client( fds[i] );	// 根据客户端描述符删除客户端
			
	//			event_notify.fd = fd;
				session_task *event_task = new session_task(fds[i], SESSION_TASK_DISCONN, 0);
				g_session_man -> post_to_session( event_task );
			}
		}
		/** 间隔一段时间后，重新运行该函数，删除超时客户端 */
		struct timeval tv;
	    evutil_timerclear(&tv);
		tv.tv_sec = p_TCP_m -> get_socket_span();
	    event_add(&(p_TCP_m -> ev_timeout), &tv);
    }
}
