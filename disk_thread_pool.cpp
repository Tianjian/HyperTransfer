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

#include "finite_state_machine.h"



namespace ftp_server
{
	extern disk_manager *g_disk_man;
	extern session_manager *g_session_man;
	extern TCP_interface *g_tcp;

	static void *thread_function( void* arg )
	{
		printf("disk thread is running\n");
		pthread_detach(pthread_self());
		
		disk_thread *p_d_t = (disk_thread *)arg;
		int i = p_d_t -> t_id;	// 线程ID
		_disk_pending_item *temp_item;

//		temp_session = NULL;
		temp_item = NULL;
		while(1)
		{
			pthread_mutex_lock(&(g_disk_man -> p_d_thread_pool -> dt_mutex[i]));

			while( TAILQ_EMPTY(&(p_d_t -> p_d_pending_pool.disk_pending_head)) )
			{
				pthread_cond_wait(&(g_disk_man -> p_d_thread_pool -> dt_cond[i]),
					&(g_disk_man -> p_d_thread_pool -> dt_mutex[i]));
			}
		    
			temp_item = TAILQ_FIRST(&(p_d_t -> p_d_pending_pool.disk_pending_head));
			p_d_t -> fsm_task = temp_item -> deal_task;
				
			/** 复制完删除队列中的任务及临时变量 */
			TAILQ_REMOVE(&(p_d_t -> p_d_pending_pool.disk_pending_head), temp_item, disk_queue_entry);
			delete temp_item;
			temp_item ->deal_task = NULL;

			temp_item = NULL;
			if( p_d_t -> fsm_task != NULL )
			{
				p_d_t -> g_disk_fsm -> roll_state_machine( p_d_t -> fsm_task);
				delete p_d_t -> fsm_task;
				p_d_t -> fsm_task = NULL;
			}
			pthread_mutex_unlock(&(g_disk_man -> p_d_thread_pool->dt_mutex[i]));

//			printf("roll_disk_fsm\n");

		}
		return NULL;
	}

	bool disk_thread_pool::start_running()
	{
		int i;
		int ret;
		pthread_attr_t thread_attr;
		pthread_attr_init (&thread_attr);
		pthread_attr_setstacksize (&thread_attr, STACKSIZE );

		for( i = 0 ; i < max_num ; i++ )
		{
			p_threads[i].t_id = i;
			p_threads[i].b_run = true;

			ret = pthread_create( &(p_threads[i].disk_id), &thread_attr, 
				&thread_function , (void *)&p_threads[i]);
			if(ret != 0)
			{
				printf("Create disk thread error\n");
				return false;
			}
			else
			{
				printf("Create disk thread success\n");
			}
		}
		return true;
	}


	bool disk_thread_pool::stop_running( bool b_force )
	{
		for ( int i = 0 ; i< max_num ; i++ )
		{
			if(!p_threads[i].b_run) continue;

			if( b_force )
			{
				pthread_cancel( p_threads[i].disk_id );
			}
			else
			{
				pthread_join( p_threads[i].disk_id ,NULL);
			}
		}
		return true;
	}

};

