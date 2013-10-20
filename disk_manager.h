#ifndef _DISK_MANAGER_H_
#define _DISK_MANAGER_H_

namespace ftp_server
{
	class disk_manager
	{
	public:
		int disk_thread_num;
		disk_thread_pool *p_d_thread_pool;
//		disk_pending_pool *p_d_pending_pool;
	public:
		map<int , disk_info *>  m_disk_info_map;

	public:
		disk_manager()
		{
			disk_thread_num = g_config.disk_thread_num;
			p_d_thread_pool = new disk_thread_pool(disk_thread_num);
		};
		~disk_manager()
		{
			delete p_d_thread_pool;
			p_d_thread_pool = NULL;
		};

	public:
		bool start_proc();

	public:
		bool post_to_disk( disk_task *deal_task );

		void add_disk(disk_info *_disk);
		disk_info *seek_disk( int fd );
		bool del_disk( int fd );

	};
}

#endif /* _DISK_MANAGER_H_ */
