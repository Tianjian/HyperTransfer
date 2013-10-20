#include "common_include.h"
#include "common_def.h"

#include "TCP_client.h"
#include "TCP_manager.h"
#include "TCP_interface.h"
#include "TCP_tool.h"

#include "session_manager.h"



namespace ftp_server
{
	extern session_manager *g_session_man;

    bool session_manager::start_proc()
    {
		/** 启动线程池 */	
		return p_s_thread_pool -> start_running();
    }

	/** 将从TCP接口的获取事件通知到会话管理器 */
	bool session_manager::post_to_session( session_task *deal_task )
	{
		_session_pending_item *temp_item = new struct _session_pending_item;
		// 新建一个会话队列项

		int i;
		i = deal_task -> get_fd() % g_config.session_thread_num;
		//	根据控制连接的socket号把会话交给指定的线程处理

		temp_item -> deal_task = deal_task;
		TAILQ_INSERT_TAIL(&(g_session_man -> p_s_thread_pool -> p_threads[i].p_s_pending_pool.session_pending_head),
			temp_item, queue_entry);
		pthread_cond_signal(&g_session_man -> p_s_thread_pool -> st_cond[i]);
		//	赋值完毕，发送信号给线程
		return true;
	}

	void session_manager::add_session(session *_session)
	{
		pair<map< int , session* >::iterator, bool> insert_pair;
		insert_pair = g_session_man -> m_session_map.insert(pair< int, session* >( _session -> get_fd() , _session ));
//		cout<<"insert_pair"<<insert_pair<<endl;
		return ;
	}

	session* session_manager::seek_session( int fd )
	{
		map<int , session *>::iterator iter;
		iter = m_session_map.find( fd );
		//session *temp=(session *)iter->second;
		if( iter != m_session_map.end() )
		{
		//	printf("%d\n",temp->fd);
		//	printf("%d\n",temp->command_length);
//			printf("return not NULL");
			return (session *)iter->second;
		}
		else
		{
//			printf("return NULL");
			return NULL;
		}
	}
	bool session_manager::del_session( int fd )
	{
		map< int , session * >::iterator iter;
		iter = m_session_map.find( fd );	
		if( iter != m_session_map.end() )
		{
			session *pclt = (session *)iter->second;
			m_session_map.erase( iter );
			delete pclt;
			pclt = NULL;
		}
		return true;
	}

}
