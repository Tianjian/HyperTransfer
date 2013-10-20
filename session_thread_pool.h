#ifndef _SESSION_THREAD_POOL_H_
#define _SESSION_THREAD_POOL_H_
#include "common_include.h"
#include "session_thread.h"
#include "session.h"

namespace ftp_server
{
	class session_thread_pool
	{
	private:
		int max_num;

	public:
		session_thread *p_threads;
		pthread_cond_t *st_cond;
		pthread_mutex_t *st_mutex;

		_event_notify *fsm_notify;
		
	public:
		session_thread_pool(int thread_num)
		{
			max_num = thread_num;	// 线程池的线程数
			p_threads = new session_thread[ max_num ];

			st_cond = new pthread_cond_t[ max_num ];
			st_mutex = new pthread_mutex_t[ max_num ];

			fsm_notify = new _event_notify[ max_num ];
			for(int i = 0; i < max_num; i++ )
			{
				pthread_cond_init(&st_cond[i], NULL);
				pthread_mutex_init(&st_mutex[i], NULL);
			}
		};

		~session_thread_pool()
		{
			stop_running();
			for(int i = 0; i < max_num; i++ )
			{
				pthread_mutex_destroy(&st_mutex[i]);
				pthread_cond_destroy(&st_cond[i]);
			}
			delete[] st_cond;
			delete[] st_mutex;
			delete[] p_threads;
			delete[] fsm_notify;
		};

	public:
		bool start_running();

		bool stop_running( bool b_force = false );

	};

};


#endif 
