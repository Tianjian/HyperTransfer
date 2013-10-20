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
	extern session_manager *g_session_man;
	extern finite_state_machine g_fsm;

	/** 会话处理线程 */
	static void *thread_function( void* arg )
	{
		printf("session_thread is running\n");
		pthread_detach(pthread_self());
		
		session_thread *p_s_t = (session_thread *)arg;	//	指向该线程的指针
		int i = p_s_t -> t_id;	// 线程ID
		_session_pending_item *temp_item;
		
        while(1)
		{
			pthread_mutex_lock(&(g_session_man -> p_s_thread_pool -> st_mutex[i]));
			// 线程空闲时，抢锁

			/** 当该线程的任务队列为空时，等待信号 */
			while(TAILQ_EMPTY(&(p_s_t -> p_s_pending_pool.session_pending_head)))
			{
				pthread_cond_wait(&(g_session_man -> p_s_thread_pool -> st_cond[i]),
					&(g_session_man -> p_s_thread_pool -> st_mutex[i]));
			}
			
//printf("Session Thread %d is processing the information.\n", i);
			// 确定该信息由本线程进行处理

			/** 从待处理队列中取出第一个信息，复制到该线程处理的地址，
				完成后，删除队列中的任务及临时变量 */
			temp_item = TAILQ_FIRST(&(p_s_t -> p_s_pending_pool.session_pending_head));
			p_s_t -> fsm_task = temp_item -> deal_task;
			TAILQ_REMOVE(&(p_s_t -> p_s_pending_pool.session_pending_head),
				temp_item, queue_entry);
			delete temp_item;
			temp_item = NULL;


			//	如果会话不为空，交给状态机处理，等待结果
			if( p_s_t -> fsm_task != NULL )
			{
				g_fsm.roll_state_machine(p_s_t -> fsm_task);
			}

			delete (p_s_t -> fsm_task);
			p_s_t -> fsm_task = NULL;

			pthread_mutex_unlock(&(g_session_man -> p_s_thread_pool -> st_mutex[i]));
			//	赋值结束，解锁，等待下一个任务
			
		}
		return NULL;
	}

	/** 启动会话处理线程 */
	bool session_thread_pool::start_running()
	{
		int i;
		int ret;
		pthread_attr_t thread_attr;
		pthread_attr_init (&thread_attr);
		pthread_attr_setstacksize (&thread_attr, STACKSIZE );
//cout<<max_num<<endl;
		/** 启动每个会话处理线程 */
		for( i = 0 ; i < max_num ; i++ )
		{
			p_threads[i].t_id = i;
			p_threads[i].b_run = true;
			ret = pthread_create( &(p_threads[i].session_id), &thread_attr, 
				&thread_function , (void *)&p_threads[i]);
			if(ret != 0)
			{
				printf("Create session thread error\n");
				return false;
			}
			else
			{
				printf("Create session thread %d success\n", i);
			}
		}
		return true;
	}

	/** 停止所有线程 */
	bool session_thread_pool::stop_running( bool b_force )
	{
		for ( int i = 0; i < max_num; i++ )
		{
			if(!p_threads[i].b_run)
			{
				continue;
			}
			if( b_force )
			{
				pthread_cancel(p_threads[i].session_id );
			}
			else
			{
				p_threads[i].b_run = false;
				pthread_join(p_threads[i].session_id, NULL);
			}
		}
		return true;
	}
}
