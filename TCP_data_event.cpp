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

	/** 初始化数据连接的事件 */
	int TCP_interface::init_TCP_data_event(TCP_client *data_client)
	{
		event_set(&data_client -> event_data_accept, data_client -> listen_fd,
			EV_READ|EV_PERSIST,	data_accept, (void *)data_client);
		event_add(&data_client -> event_data_accept, NULL);

		return 0;
	}	

	void TCP_interface::data_accept(int fd, short ev, void *arg)
    {
        TCP_client *p_client = (TCP_client *)arg;
		/** 接受客户端并获取相关信息 */
		int client_fd;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);

		if(client_fd == -1)
		{
			printf("%d no accpet\n", fd);
			LOG(WARN, "accept failed");
			return ;
		}
		else
		{
			printf("%d accept %d\n",fd,client_fd);
		}

		/** 设置客户端socket为非阻塞 */
		if(g_tcp_tool -> set_nonblock(client_fd) < 0)
		{
			LOG(WARN, "failed to set client socket non-blocking");
		}
		p_client -> set_data_fd(client_fd);
		{
			/** 初始化on_read事件 */
			event_set(&p_client -> event_data_read, client_fd,
 			EV_READ | EV_PERSIST, data_read, (void*)p_client);

			/** 激活on_read事件 */
			event_add(&p_client -> event_data_read, NULL);
		}
		/** 初始化on_write事件 */
		event_set(&p_client -> event_data_write, client_fd, 
			EV_WRITE, data_write, (void*)p_client);

		LOG(INFO,"Accepted connection from "<<inet_ntoa(client_addr.sin_addr));

		session_task *event_task = new session_task(p_client -> get_ctrl_socket(), SESSION_TASK_DATA_ACCEPT, 0);

		g_session_man -> post_to_session( event_task );


        return ;
    }
	
	void TCP_interface::data_read(int fd, short ev, void *arg)
	{
        TCP_client *p_client = (TCP_client *)arg;
		p_client -> update_last_access();
		int data_len;
		if(fd == -1)
		{
			if(fd == -1)
			{
				cout<<"on_read:数据连接句柄已删除"<<endl;
			}
			return ;
		}
		else
		{
			data_len = p_client -> read_data();

			/** 客户端关闭数据连接，读出数据为空，上传完毕（也可能是网络问题）上传中断 */
			if (data_len == 0)
			{
				if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) 
				{
cout<<errno<<strerror(errno)<<endl;
return ;
				}
				disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_STOR_OK);
LOG(INFO , "disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_STOR_OK)");
				event_task -> set_new_file_buffer(p_client -> get_file_buffer(), p_client -> get_buf_len());
//cout<<"g_disk_man -> post_to_disk( event_task ):"<<p_client -> get_buf_len()<<endl;
				p_client -> clear_data();
				g_disk_man -> post_to_disk( event_task );
				p_client -> clear_datafd();

			}
			else
			/** 数据连接错误 */
			if (data_len < 0)
			{
				if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) 
				{
cout<<errno<<strerror(errno)<<endl;
				}
				else
				{
					LOG(INFO , "Socket failure, disconnecting client: "<<fd<<" "<<strerror(errno));
					p_client -> clear_datafd();
				}
				return ;
			}
			/** 数据上传中 */
			else
			{
				/** 将数据从临时区复制到文件数据缓存区 */
				if(p_client -> get_buf_len() > FILEBUFWTIRESIZE)
				{
					LOG(WARN, "p_client -> get_buf_len() > FILEBUFWTIRESIZE");
					exit(1);
				}
				p_client -> copy_data();

				if(p_client -> get_buf_len() <= FILEBUFWTIRESIZE)
				{
//cout<<"p_client -> get_buf_len():"<<p_client -> get_buf_len()<<endl;
					return ;
				}
//				if(TCP_STOR_LOADING == p_client -> get_data_action())
//				{
					disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_STORING);
					LOG(INFO, "disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_STORING);");
					event_task -> set_new_file_buffer(p_client -> get_file_buffer(), p_client -> get_buf_len());
					g_disk_man -> post_to_disk( event_task );
					p_client -> clear_data();
//				}
//				else
//				{
//					cout<<"p_client -> get_data_action()："<<p_client -> get_data_action()<<endl;
//				}
			}
		}
		return ;
	}

	void TCP_interface::data_write(int fd, short ev, void *arg)
    {
//		cout<<"data_write"<<endl;
        TCP_client *p_client = (TCP_client *)arg;
		p_client -> update_last_access();

		int len;
		struct _data_buffer_q *buf_q;

		buf_q = TAILQ_FIRST(&p_client -> data_write_q);
		// 从会话的写队列中获取要写的数据

		if (buf_q == NULL)
		{
//			cout<<"data_write: buf_q == NULL"<<endl;
			return;
		}

		len = write(fd, buf_q -> buf + buf_q -> offset, buf_q -> len - buf_q->offset );
//cout<<"len:"<<len<<endl;
		if (len == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
cout<<"errno == EINTR || errno == EAGAIN"<<endl;
				event_add(&p_client->event_data_write, NULL);
				return;
			}
			else
			{
				LOG(WARN, "error in socket write, close FD: "<<fd);
				return;
			}
		}
		else
		if( len < 0)
		{
			cout<<"len"<<len<<"bigerror:"<<strerror(errno)<<endl;
		}
		else
/*		if( 0 == len )
		{
			TAILQ_REMOVE(&p_client->data_write_q, buf_q, data_entries);
			if( buf_q->buf != NULL )
			{
				delete[] buf_q->buf;
				buf_q->buf = NULL;
			}
			delete buf_q;
			buf_q = NULL;
			disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_WRITE_DONE);
			g_disk_man -> post_to_disk( event_task );
		}
		else
*/		if ((buf_q->offset + len) < buf_q->len)
		{
			buf_q->offset += len;
			event_add(&p_client->event_data_write, NULL);
//cout<<"**************************"<<len<<endl;
			return;
		}
		else
		{
			TAILQ_REMOVE(&p_client->data_write_q, buf_q, data_entries);
			if( buf_q->buf != NULL )
			{
				delete[] buf_q->buf;
				buf_q->buf = NULL;
			}
			delete buf_q;
			buf_q = NULL;

			disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_WRITE_DONE);
LOG(INFO, "disk_task *event_task = new disk_task(p_client -> get_ctrl_socket(), DATA_TASK_WRITE_DONE);");
			g_disk_man -> post_to_disk( event_task );
			return;
		}
	}
}
