#ifndef _DISK_THREAD_POOL_H_
#define _DISK_THREAD_POOL_H_

namespace ftp_server
{
	class disk_thread_pool
	{
	public:
		int max_num;
		disk_thread *p_threads;

		pthread_cond_t *dt_cond;
		pthread_mutex_t *dt_mutex;

		_event_notify *fsm_notify;
	public:
		disk_thread_pool(int thread_num)
		{
			max_num = thread_num;
			p_threads = new disk_thread[ max_num ];
			fsm_notify = new _event_notify[max_num];

			dt_cond = new pthread_cond_t[ max_num ];
			dt_mutex = new pthread_mutex_t[ max_num ];
			for(int i = 0; i < max_num; i++ )
			{
				pthread_cond_init(&dt_cond[i], NULL);
				pthread_mutex_init(&dt_mutex[i], NULL);
			}

		};

		~disk_thread_pool()
		{
			stop_running( false);

			for(int i = 0; i < max_num; i++ )
			{
				pthread_cond_destroy(&dt_cond[i]);
				pthread_mutex_destroy(&dt_mutex[i]);
			}
			delete[] dt_cond;
			delete[] dt_mutex;
			delete[] p_threads;

			delete[] fsm_notify;
		};

	public:
		bool start_running();
		bool stop_running( bool b_force );
	};

};


#endif /* _DISK_THREAD_POOL_H_ */
