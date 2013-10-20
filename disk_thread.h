#ifndef _DISK_THREAD_H_
#define _DISK_THREAD_H_

namespace ftp_server
{
	class disk_thread
	{
	public:
		int t_id ;
		pthread_t disk_id;
		bool b_run;
		disk_task *fsm_task;
		disk_pending_pool p_d_pending_pool;
		disk_fsm *g_disk_fsm;

	public:
		disk_thread()
		{
			b_run = false;
			fsm_task = NULL;
			g_disk_fsm = new disk_fsm();
//			p_d_pending_pool = new disk_pending_pool();
		};
		~disk_thread()
		{
//			delete p_d_pending_pool;
		};
		void tailq_insert(_disk_pending_item *temp_item)
		{
			p_d_pending_pool.tailq_insert(temp_item);
		};
	};
}
#endif /* _DISK_THREAD_H_ */
