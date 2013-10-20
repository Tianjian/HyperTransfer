#ifndef _SESSION_MANAGER_H_
#define _SESSION_MANAGER_H_
#include "session_thread_pool.h"
#include "session_pending_pool.h"
#include "user_manager.h"
#include "session_task.h"

namespace ftp_server
{
	class session_manager
	{
	public:
		int session_thread_num;
		session_thread_pool *p_s_thread_pool;
//		session_pending_pool *p_s_pending_pool;
		user_manager *p_user_manager;
	public:
		map<int , session *>  m_session_map;


	public:
		session_manager()
		{
			session_thread_num = g_config.session_thread_num;
			p_s_thread_pool = new session_thread_pool( session_thread_num );
//			p_s_pending_pool = new session_pending_pool();
			p_user_manager = new user_manager();
		};

		~session_manager()
		{
			delete p_s_thread_pool;
			p_s_thread_pool	= NULL;
			delete p_user_manager;
			p_user_manager = NULL;
		};

	public:
		bool start_proc();

	public:
		bool post_to_session( session_task *event_task );
		void add_session(session *_session);
		session *seek_session( int fd );
		bool del_session( int fd );
	};
};
#endif
