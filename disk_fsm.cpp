
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

using namespace std;


using namespace ftp_server;
namespace ftp_server
{
	extern disk_manager *g_disk_man;
	extern session_manager *g_session_man;
	extern TCP_interface *g_tcp;

	int disk_fsm::roll_state_machine(disk_task *input_task)
	{
		if(input_task == NULL)
		{
			LOG(INFO, "input_task == NULL");
			return 0;
		}
		LOG(INFO, "input_task -> get_action()"<<input_task -> get_action());
		int ret = 0;
		disk_info *fsm_disk;

		if(DATA_TASK_INDEX_START == input_task -> get_action())
		{
			fsm_disk = new disk_info(input_task);
			g_disk_man -> add_disk(fsm_disk);
//cout<<"input_task -> get_cur_path() "<<input_task -> get_cur_path() <<endl;
			fsm_disk -> set_data_action(run_index_start(fsm_disk));
//			cout<<fsm_disk -> get_file_buffer()<<endl;
			g_tcp -> data_post(fsm_disk -> get_fd(), fsm_disk -> get_list_info(), fsm_disk -> get_list_info_len());
			return 0;
		}
		else
		if(DATA_TASK_STOR_START == input_task -> get_action())
		{
			fsm_disk = new disk_info(input_task);
			g_disk_man -> add_disk(fsm_disk);
//cout<<"input_task -> get_cur_path() "<<input_task -> get_cur_path() <<endl;
			ret = run_stor_start(fsm_disk);
			if (0 == ret)
			{
				fsm_disk -> set_data_action(DISK_STOR_START);
				session_task *task = new session_task(input_task -> get_ctrl_fd(), SESSION_TASK_STOR_START, 0);
				g_session_man -> post_to_session( task );

				return 0;
			}
			else
			{
				return ret;
			}
		}
		else
		if(DATA_TASK_RETR_START == input_task -> get_action())
		{
			fsm_disk = new disk_info(input_task);
			g_disk_man -> add_disk(fsm_disk);
//cout<<"input_task -> get_cur_path() "<<input_task -> get_cur_path() <<endl;
			fsm_disk -> set_data_action(run_retr_start(fsm_disk));
			g_tcp -> data_post(fsm_disk -> get_fd(), fsm_disk -> get_file_buffer(), fsm_disk -> get_buf_len());

			return 0;
		}
		else
		{
			fsm_disk = g_disk_man -> seek_disk( input_task -> get_ctrl_fd() );
			if(fsm_disk != NULL)
			{
				LOG(INFO,"disk_info:fd ="<<input_task -> get_ctrl_fd()<<"is exist");
//				cout<<"disk_info:fd ="<<input_task -> get_ctrl_fd()<<"is exist"<<endl;
			}
			else
			{
				LOG(INFO,"disk_info:fd ="<<input_task -> get_ctrl_fd()<<"is not exist");
//				cout<<"disk_info:fd ="<<input_task -> get_ctrl_fd()<<"is not exist"<<endl;
				return 0;
			}
		}

		if(DATA_TASK_WRITE_DONE == input_task -> get_action())
		{
			switch(fsm_disk -> get_data_action())
			{
				/** 目录下载请求 */
				case(DISK_INDEX_LOADING):
				{
//					cout<<"DISK_INDEX_LOADING"<<endl;
					fsm_disk -> set_data_action(run_index_loading(fsm_disk));

					g_tcp -> data_post(fsm_disk -> get_fd(), fsm_disk -> get_list_info(), fsm_disk -> get_list_info_len());
				}
				break;
				
				case(DISK_INDEX_FINISH):
				{
//					cout<<"DISK_INDEX_FINISH"<<endl;
					session_task *task = new session_task(input_task -> get_ctrl_fd(), SESSION_TASK_TRANS_OK, 0);
					g_session_man -> post_to_session( task );
					g_disk_man -> del_disk(fsm_disk -> get_fd());
				}
				break;
							
				/** 文件下载请求 */
				case(DISK_RETR_LOADING):
				{
					fsm_disk -> set_data_action(run_retr_loading(fsm_disk));
//					cout<<fsm_disk -> get_file_buffer()<<endl;
					g_tcp -> data_post(fsm_disk -> get_fd(), fsm_disk -> get_file_buffer(), fsm_disk -> get_buf_len());
				}
				break;
				
				case(DISK_RETR_FINISH):
				{
//					cout<<"DISK_RETR_FINISH"<<endl;
					session_task *task = new session_task(input_task -> get_ctrl_fd(), SESSION_TASK_TRANS_OK, 0);
					g_session_man -> post_to_session( task );
					g_disk_man -> del_disk(fsm_disk -> get_fd());
				}
				break;
	
				default:
					cout<<"unknow action: "<<fsm_disk -> get_data_action()<<endl;
					break;
			}

		}
		else
		if(DATA_TASK_STORING == input_task -> get_action())
		{
//			int action = fsm_disk -> get_data_action();
//			if(DISK_STOR_DONE == action || DISK_STOR_START == action)
			{
				fsm_disk -> set_data_action(DISK_STORING);
				ret = run_stor_loading(input_task, fsm_disk);
				if (0 == ret)
				{
					LOG(ERROR , "nothing was written");
					return 0;
				}
				else
				if (-1 == ret)
				{
					LOG(ERROR , "write error: "<<strerror(errno));
					return -1;
				}
				else
				{
					fsm_disk -> set_data_action(DISK_STOR_DONE);
//					session_task *task = new session_task(input_task -> get_ctrl_fd(), SESSION_TASK_STOR_START, 0);
//					g_session_man -> post_to_session( task );

					return 0;
				}
			}
/*			else
			{
				cout<<"disk_fsm error"<<endl;
				return -1;
			}
*/		}
		else
		if(DATA_TASK_STOR_OK == input_task -> get_action())
		{
/*			int action = fsm_disk -> get_data_action();
			if(DISK_STOR_DONE == action || DISK_STOR_START == action)
			{
*/				fsm_disk -> set_data_action(DISK_STORING);
				ret = run_stor_finish(input_task, fsm_disk);
				if (0 == ret)
				{
					LOG(ERROR , "nothing was written");
					return 0;
				}
				else
				if (-1 == ret)
				{
					LOG(ERROR , "write error: "<<strerror(errno));
					return -1;
				}
				else
				{
					fsm_disk -> set_data_action(DISK_STOR_DONE);

//					Util getMD5;
//					string md5_result;	
//					md5_result = getMD5.get_file_signature(fsm_disk -> get_cur_path());
//					cout<<md5_result<<endl;
//					cout<<fsm_disk -> get_cur_path()<<endl;
//					exit(1);

					session_task *task = new session_task(input_task -> get_ctrl_fd(), SESSION_TASK_TRANS_OK, 0);
					g_session_man -> post_to_session( task );
					g_disk_man -> del_disk(fsm_disk -> get_fd());
					return 0;
				}
/*			}
			else
			{
				cout<<"disk_fsm error"<<endl;
					return -1;
			}*///cout<<"STOR_FINISH:"<<input_task -> get_buf_len()<<endl;
//			int len = run_stor_finish(input_task, fsm_disk);
//cout<<"STOR_FINISH:"<<len<<endl;
//			fsm_disk -> set_data_action();
//			g_tcp -> post_action(fsm_disk -> get_fd(), EVENT_NOTIFY_DATA_DELETE);	
		}
		else
		if(DATA_TASK_DELE == input_task -> get_action())
		{					
			g_disk_man -> del_disk(fsm_disk -> get_fd());
		}
		else
		{
			cout<<"input_task -> get_action()"<<input_task -> get_action()<<endl;
		}
		return 0;
	}

	int disk_fsm::run_index_start( disk_info *fsm_disk )
	{		
		fsm_disk -> clear_list_info();
		if( fsm_disk -> get_list() != NULL )
		{
			return DISK_INDEX_LOADING;
		}
		else
		{
			return DISK_INDEX_FINISH;
		}
		/*	只考虑LIST+文件夹，暂不考虑LIST+文件*/
	}
	int disk_fsm::run_index_loading( disk_info *fsm_disk )
	{
		fsm_disk -> clear_list_info();
		if( fsm_disk -> get_list(3) != NULL )
		{
			return DISK_INDEX_LOADING;
		}
		else
		{
			return DISK_INDEX_FINISH;
		}
	}

	int disk_fsm::run_retr_start( disk_info *fsm_disk )
	{
		fsm_disk -> open_readonly(); // 打开文件句柄（只读）

		fsm_disk -> seek_file_handle();

		/** 文件尚未下载完 */
		if(fsm_disk -> read_from_disk() > 0)
		{
			return DISK_RETR_LOADING;
		}
		/** 文件下载完，通知会话管理器 */
		else
		{
			fsm_disk -> close_file_handle();
			return DISK_RETR_FINISH;
		}
	}
	int disk_fsm::run_retr_loading( disk_info *fsm_disk )
	{
		/** 文件尚未下载完 */
		if(fsm_disk -> read_from_disk() > 0)
		{
			return DISK_RETR_LOADING;
		}
		/** 文件下载完，通知会话管理器 */
		else
		{
			fsm_disk -> close_file_handle();
			return DISK_RETR_FINISH;
		}
	}

	/** 数据上传过程 */
	int disk_fsm::run_stor_start( disk_info *fsm_disk )
	{
		int rest_size = fsm_disk -> get_rest_size();
		if( rest_size == 0)
		{
			fsm_disk -> open_create();
		}
		else
		{
			fsm_disk -> open_writeonly();
			fsm_disk -> seek_file_handle();
			fsm_disk -> set_rest_size(0);
		}
		return 0;
	}

	int disk_fsm::run_stor_loading( disk_task *input_task, disk_info *fsm_disk )
	{
		int ret;
		ret = fsm_disk -> write_to_disk(input_task);
		LOG(INFO, "run_stor_loading:fsm_disk -> write_to_disk(input_task)"<<ret);
		return ret;
		//	将上传信息写入磁盘
	}

	/** 上传完毕 */
	int disk_fsm::run_stor_finish( disk_task *input_task, disk_info *fsm_disk )
	{
		int len = fsm_disk -> write_to_disk(input_task);
		//	将上传信息写入磁盘

		fsm_disk -> close_file_handle();

		return len;
	}
	
}
