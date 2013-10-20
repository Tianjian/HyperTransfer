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


namespace ftp_server
{
	extern disk_manager *g_disk_man;
	extern session_manager *g_session_man;
	extern TCP_interface *g_tcp;

    bool disk_manager::start_proc()
    {
		/** 启动磁盘管理器的线程 */
		return p_d_thread_pool -> start_running();
    }
		
	/** 将需要磁盘处理的信息通知到磁盘管理器 */
	bool disk_manager::post_to_disk(disk_task *deal_task )
	{		
		if(deal_task == NULL)
		{
			return false;
		}

		_disk_pending_item *temp_item = new struct _disk_pending_item;
		//新建一个会话队列项

		int i;
		i = deal_task -> get_ctrl_fd() % g_config.session_thread_num;

		/** 传到待磁盘处理队列 */
		temp_item -> deal_task = deal_task;
        g_disk_man -> p_d_thread_pool -> p_threads[i].tailq_insert(temp_item);
		pthread_cond_signal(&g_disk_man -> p_d_thread_pool->dt_cond[i]);
		return true;
	}

	void disk_manager::add_disk(disk_info *_disk)
	{
		pair<map< int , disk_info* >::iterator, bool> insert_pair;
		insert_pair = g_disk_man -> m_disk_info_map.insert(pair< int, disk_info* >( _disk -> get_fd() , _disk));
		return ;
	}

	disk_info* disk_manager::seek_disk( int fd )
	{
		map<int , disk_info *>::iterator iter;
		iter = m_disk_info_map.find( fd );
		if( iter != m_disk_info_map.end() )
		{
			return (disk_info *)iter->second;
		}
		else
		{
			return NULL;
		}
	}
	bool disk_manager::del_disk( int fd )
	{
		map< int , disk_info * >::iterator iter;
		iter = m_disk_info_map.find( fd );	
		if( iter != m_disk_info_map.end() )
		{
			disk_info *pclt = (disk_info *)iter->second;
			m_disk_info_map.erase( iter );
			delete pclt;
			pclt = NULL;
		}
		return true;
	}
}
