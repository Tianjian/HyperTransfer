#ifndef _SESSION_THREAD_H_
#define _SESSION_THREAD_H_
#include "session_task.h"
#include "session_pending_pool.h"
namespace ftp_server
{
	class session_thread
	{
	public:
		int t_id ;
		pthread_t session_id;
		bool b_run;
		session_task *fsm_task;
		session_pending_pool p_s_pending_pool;
	public:
		session_thread()
		{
			b_run = false;
			fsm_task = NULL;
//			p_s_pending_pool = new session_pending_pool();
		};
		~session_thread()
		{
//			delete p_s_pending_pool;
		};
	};
}
#endif /* _SESSION_THREAD_H_ */
