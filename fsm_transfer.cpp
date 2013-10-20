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
	extern TCP_interface *g_tcp;
	extern disk_manager *g_disk_man;
	extern session_manager *g_session_man;
	extern _ftpserver_config g_config;
	extern finite_state_machine g_fsm;

	/** 处理命令 —— SIZE */
    void finite_state_machine::com_SIZE(session *fsm_session)
    {
//		printf("********com_SIZE********\n");
        char ret_command[COMMANDSIZE];

		struct stat file_info;

		strncpy(ret_command, fsm_session -> get_cur_path(), COMMANDSIZE);
		ret_command[ COMMANDSIZE-1 ] = 0;
        strncat(ret_command, fsm_session -> get_processing_command(), COMMANDSIZE);
		fsm_session -> set_file_name(ret_command);

		if(	stat(fsm_session -> get_file_name(), &file_info) != -1)
		{

			if(S_ISDIR(file_info.st_mode))
			{
               	strncpy(ret_command, "550 ", COMMANDSIZE);
				strncat(ret_command, fsm_session -> get_relative_name(), COMMANDSIZE);
				strncat(ret_command, ": not a regular file\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);
			}
			else
			{
	//cout<<fsm_session->file_name<<endl;
				fsm_session -> set_file_len(file_info.st_size);
				strncpy(ret_command, "213 ", COMMANDSIZE);
				char *str = new char[20];
				memset(str, 0, 20);
				sprintf(str, "%ld", file_info.st_size);
				strncat(ret_command, str, COMMANDSIZE);
				delete []str;
				str = NULL;
				strncat(ret_command , "\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);
			}
		}
		else
		{
			strncpy(ret_command, "550 Requested action not taken. \"",
				COMMANDSIZE);
			strncat(ret_command, fsm_session -> get_relative_name(), COMMANDSIZE);
			strncat(ret_command, "\": no such file.\r\n", COMMANDSIZE);
			fsm_session -> set_ret_command(ret_command);
		}
		fsm_session -> post();
	}

	/** 处理命令 —— REST */
	void finite_state_machine::com_REST(session *fsm_session)
	{
//		printf("********com_REST********\n");
        char ret_command[COMMANDSIZE];
		long rest_size = 0;

		if(	fsm_session -> cmp_processing_command(" ", 1) == 0 )
		{
			sscanf(fsm_session -> get_processing_command() + 1, "%ld", &rest_size);
			fsm_session -> set_rest_size(rest_size);
			strncpy(ret_command, "350 Restarting at ", COMMANDSIZE);
			strncat(ret_command, fsm_session -> get_processing_command() + 1, COMMANDSIZE);
			strncat(ret_command, ". Send STOR or RETR to initiate transfer.\r\n", COMMANDSIZE);
			fsm_session -> set_ret_command(ret_command);
		}
		else
		{
			fsm_session -> set_rest_size(0);
			fsm_session -> set_ret_command
				("350 Restarting at 0. Send STOR or RETR to initiate transfer.\r\n");
		}
		fsm_session -> post();
	}

	/** 处理命令 —— PASV */
	int finite_state_machine::com_PASV(session *fsm_session)
	{
		/** 用户尚未成功登录 */
		if(fsm_session -> get_login() == false)
		{
			LOG(WARN, "PASV fail: 530");
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
			LOG(INFO, fsm_session -> get_ret_command());
			fsm_session -> post();
			return -1;
		}
		else
		/** 数据连接未被使用 */
//		if( DATA_WAITING == fsm_session -> get_data_action() )
		{
			fsm_session -> set_action(CTRL_PASV);
			cout<<fsm_session -> get_data_action()<<" "<<fsm_session -> get_action()<<endl;
			fsm_session -> pasv_post();
			return 0;
		}
/*		else
		{
			** 数据连接已经建立，一个会话不能同时建立两个数据连接 *
			LOG(WARN, "PASV fail: 502");
			fsm_session -> set_ret_command("502 Server is busy now, Service not available.\r\n");
		}
  */ //     fsm_session -> post();
	//	return 0;

	} 
	/** 处理未知操作的数据连接 */
	int finite_state_machine::run_pasv(session *fsm_session, int port_num)
	{
		char ret_command[COMMANDSIZE];	//	返回信息
		memset(ret_command, 0, COMMANDSIZE);
		char port[PORT_PART_LEN];	//	返回信息中的端口
		
		char host_ip[PASV_RET_IPLEN];	//	返回信息中的服务器IP
		memset(host_ip, 0, PASV_RET_IPLEN);
		char *server_ip;	

//        char port[10];

		int temp;	//	临时存储端口字段的变量

		string::size_type i;
		if(port_num > g_config.start_port && port_num < g_config.end_port)
		{
			strncpy(ret_command, "227 Entering Passive Mode (",
				COMMANDSIZE);
			server_ip = fsm_session -> get_server_ip();	//	获取服务器IP
		    /** 将服务器IP转化成返回信息特定的格式 */
			for(i = 0; server_ip[i] != '\0' ; ++i)
		    {
		   	    if(server_ip[i] != '.')
		       	{
		           	host_ip[i] = server_ip[i];
		        }
		   	    else
				{
					host_ip[i] = ',';
				}
		    }
			host_ip[i]=',';
			host_ip[i+1] = '\0';
			server_ip = NULL;
			
    	    strncat(ret_command, host_ip, COMMANDSIZE);	
			//	将服务器IP加入返回信息
//			stringstream oss;
//			oss.str("");

			temp = port_num >> 8;	//	获取端口的前8位（二进制）
			memset(port, 0, PORT_PART_LEN);
			snprintf(port, PORT_PART_LEN, "%d", temp);	    	
//			oss	<<ret_command<<temp<<endl;
			strncat(ret_command, port, COMMANDSIZE);
//			cout<<"port"<<port<<"temp"<<temp<<"ret_command"<<ret_command<<endl;
			strncat(ret_command, ",", COMMANDSIZE);
			//	将端口的前8位（二进制）加入返回信息

       		temp = port_num & 0x00FF;	//	获取端口的后8位（二进制）

			memset(port, 0, PORT_PART_LEN);
			snprintf(port, PORT_PART_LEN, "%d", temp);
			strncat(ret_command, port, COMMANDSIZE);
			strncat(ret_command, ").\r\n", COMMANDSIZE);
			//	将端口的后8位（二进制）加入返回信息
//			LOG(WARN, "PASV success: 227");
			fsm_session -> set_ret_command(ret_command);
			LOG(INFO, fsm_session -> get_ret_command());
			fsm_session -> post();
			return 0;
		}
		else
		{
			/** 获取不到空闲端口，服务器忙碌中 */
//			LOG(WARN, "PASV fail: 502");
			fsm_session -> set_ret_command("502 Server is busy now, Service not available.\r\n");
			LOG(INFO, fsm_session -> get_ret_command());
			fsm_session -> post();
			return -1;
		}
	}
	
	/** 处理命令 —— LIST */
    int finite_state_machine::com_LIST(session *fsm_session)
    {
//cout<<"********com_LIST********"<<endl;
		if(fsm_session -> get_login() == false)
		{
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		if(fsm_session -> get_data_action() != DATA_PASV_OK)
		{	
			fsm_session -> set_ret_command("425 Can't open data connection.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		{
			fsm_session -> set_data_action(DATA_LIST);
			return 0;
		}
    }

	int finite_state_machine::run_list_load(session *fsm_session)
	{
        char ret_command[COMMANDSIZE];
		strncpy(ret_command, "150 Data connection accepted from ", COMMANDSIZE);
		strncat(ret_command, fsm_session -> get_server_ip(), COMMANDSIZE);
		strncat(ret_command, ";transfer starting.\r\n", COMMANDSIZE);
		fsm_session -> set_ret_command(ret_command);
		
		fsm_session -> set_data_action(DATA_INDEX_DOWNLOAD);
	
		fsm_session -> post();

		disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_INDEX_START, fsm_session -> get_cur_path());
		LOG(INFO, "disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_INDEX_START, fsm_session -> get_cur_path()");
		g_disk_man -> post_to_disk(fsm_task);
		return 0;
	}

	/** 处理命令 —— STOR */
    int finite_state_machine::com_STOR(session *fsm_session)
    {
//		printf("********com_STOR********\n");
		if(fsm_session -> get_login() == false)
		{
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		if(fsm_session -> get_data_action() != DATA_PASV_OK)
		{	
			fsm_session -> set_ret_command("425 Can't open data connection.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		if(fsm_session -> get_upload_per() == false)
		{
			fsm_session -> set_ret_command("550 Cannot STOR. No permission.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		{
	        char ret_command[COMMANDSIZE];
			char jump_path[PATHSIZE];
			char cur_path[PATHSIZE];
			char *a_path;	//	绝对路径

			struct stat file_info;
		
			/** 获取文件名 */
			strncpy(jump_path, fsm_session -> get_processing_command(), PATHSIZE);
			strncpy(cur_path, fsm_session -> get_cur_path(), PATHSIZE);
			//	jump_path:客户端要跳转的路径 cur_path:当前目录

			a_path = change_path(cur_path, jump_path, fsm_session -> get_root_path());
			fsm_session -> set_file_name(a_path);
			/** 文件已存在，错误，关闭数据连接 */
			if(	stat(a_path, &file_info) != -1)
			{	
				strncpy(ret_command, "550 Cannot STOR. \"", COMMANDSIZE);
				strncat(ret_command, a_path, COMMANDSIZE);
				strncat(ret_command, "\": is exist.\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);

				fsm_session -> post();
				return -1;
			}
			fsm_session -> set_data_action(DATA_STOR);
			return 0;
		}
    }
	/** 处理命令 —— APPE */
    void finite_state_machine::com_APPE(session *fsm_session)
    {
//		printf("********com_APPE********\n");
		if(fsm_session -> get_login() == false)
		{
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
			fsm_session -> post();
		}
		else
		if(fsm_session ->get_upload_per() == false)
		{
			fsm_session -> set_ret_command("550 Cannot APPE. No permission.\r\n");
			fsm_session -> post();
		}
		else
		{
			char jump_path[COMMANDSIZE];
			char cur_path[PATHSIZE];
			char *a_path;	//	绝对路径

			struct stat file_info;
		
			strncpy(jump_path, fsm_session -> get_processing_command(), PATHSIZE);
			/** 获取文件名 */
			strncpy(cur_path, fsm_session -> get_cur_path(), PATHSIZE);
			// cur_path:当前目录

			a_path = change_path(cur_path, jump_path, fsm_session -> get_root_path());
			// a_path: 解析后的绝对路径

			fsm_session -> set_file_name(a_path);

			/** 文件已存在，设置位置为文件尾 */
			if(	stat(a_path, &file_info) != -1)
			{
//				if(fsm_session -> get_rest_size() > file_info.st_size )
				{
					fsm_session -> set_rest_size( file_info.st_size );
				}

			}
			/** 文件不存在，设置位置为0 */
			else
			{
				fsm_session -> set_rest_size( 0 );
			}
			/** 修改标志，等待数据连接建立 */
			fsm_session -> set_data_action(DATA_STOR);

		}
		return ;
	}

	/** 数据连接成功，用户开始上传 */
	int finite_state_machine::run_stor_load(session *fsm_session)
	{
		disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), 
			DATA_TASK_STOR_START, fsm_session -> get_file_name(), 
				fsm_session -> get_rest_size());
		LOG(INFO, "disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_STOR_START, fsm_session -> get_file_name(),fsm_session -> get_rest_size()");
		g_disk_man -> post_to_disk(fsm_task);
		return 0;
	}

	int finite_state_machine::run_stor_start(session *fsm_session)
	{
		char ret_command[COMMANDSIZE];
//		long rest_size = 0;
		strncpy(ret_command, "150 Data connection accepted from ", COMMANDSIZE);
		strncat(ret_command, fsm_session -> get_server_ip(), COMMANDSIZE);
		strncat(ret_command, ";transfer starting.\r\n", COMMANDSIZE);
		fsm_session -> set_ret_command(ret_command);

		fsm_session -> post();
		return 0;
	}

	/** 处理命令 —— RETR */
    int finite_state_machine::com_RETR(session *fsm_session)
    {
//		printf("********com_RETR********\n");

		/** 用户未登录 */
		if(fsm_session -> get_login() == false)
		{
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		/** 未建立数据连接 */
		if(fsm_session -> get_data_action() != DATA_PASV_OK)
		{	
			fsm_session -> set_ret_command("425 Can't open data connection.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		/** 用户无下载权限 */
		if(fsm_session ->get_download_per() == false)
		{
			fsm_session -> set_ret_command("550 Cannot RETR. No permission.\r\n");
			fsm_session -> post();
			return -1;
		}
		else
		{
	        char ret_command[COMMANDSIZE];
			char *a_path;	//	绝对路径
			char jump_path[PATHSIZE];	// 客户端要跳转的路径
			char cur_path[PATHSIZE];	// 当前目录

			struct stat file_info;
			strncpy(jump_path, fsm_session -> get_processing_command(), PATHSIZE);
			strncpy(cur_path, fsm_session -> get_cur_path(), PATHSIZE);
			// 获取用户要跳转的路径和用户当前路径

			a_path = change_path(cur_path, jump_path, fsm_session -> get_root_path());
			// a_path: 解析后的绝对路径

            fsm_session -> set_file_name(a_path);
			
			/** 检查文件是否存在 */
			if(	stat(a_path, &file_info) != -1)
			{
				/** 下载路径为文件夹 */
    	        if(S_ISDIR(file_info.st_mode))
        	    {                
            	    strncpy(ret_command, "550 ", COMMANDSIZE);
                	strncat(ret_command, jump_path, COMMANDSIZE);
	                strncat(ret_command, ": not a regular file\r\n", COMMANDSIZE);
					fsm_session -> set_ret_command(ret_command);
					fsm_session -> post();
					return -1;
    	        }
				/** 下载路径为文件 */
	            else
	            {

					/** 获取断点续传的点 */
					long rest_size = fsm_session -> get_rest_size();
					if(rest_size != 0)
					{
						if(file_info.st_size < rest_size)
						{
							strncpy(ret_command, "554 ", COMMANDSIZE);
							strncat(ret_command, fsm_session -> get_relative_name(), COMMANDSIZE);
							strncat(ret_command, ": invalid REST argument.\r\n", COMMANDSIZE);
							fsm_session -> set_ret_command(ret_command);
							fsm_session -> post();
							return -1;
						}
					}
					fsm_session -> set_file_len(file_info.st_size);
					fsm_session -> set_data_action(DATA_RETR);
					return 0;
				}
			}
			/** 文件不存在 */
			else
		    {
				strncpy(ret_command, "550 Requested action not taken. \"", COMMANDSIZE);
				strncat(ret_command, fsm_session -> get_relative_name(), COMMANDSIZE);
				strncat(ret_command, "\": no such file.\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);

				fsm_session -> post();
				return -1;
			}
		}
	}

	int finite_state_machine::run_retr_load(session *fsm_session)
	{
		char ret_command[COMMANDSIZE];
		strncpy(ret_command, "150 Data connection accepted from ", COMMANDSIZE);
		strncat(ret_command, fsm_session -> get_server_ip(), COMMANDSIZE);
		strncat(ret_command, ";transfer starting for", COMMANDSIZE);
		strncat(ret_command, fsm_session -> get_relative_name(), COMMANDSIZE);
		strncat(ret_command, " (", COMMANDSIZE);
		char *str = new char[20];
		sprintf(str, "%ld", fsm_session -> get_file_len());
		strncat(ret_command, str, COMMANDSIZE);
		delete []str;
		str = NULL;
		strncat(ret_command, " bytes)\r\n", COMMANDSIZE);
		fsm_session -> set_ret_command(ret_command);
		fsm_session -> set_data_action(DATA_FILE_DOWNLOAD);
		fsm_session -> post();

//        fsm_session -> set_start_time();		
		disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_RETR_START, fsm_session -> get_file_name(), fsm_session -> get_rest_size());
		LOG(INFO, "disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_RETR_START, fsm_session -> get_file_name(), fsm_session -> get_rest_size());");

		g_disk_man -> post_to_disk(fsm_task);
		return 0;
	}

	int finite_state_machine::run_transfer_ok(session *fsm_session)
	{
		fsm_session -> set_ret_command("226 Transfer ok.\r\n");
		fsm_session -> set_action(CTRL_WAITING);
		fsm_session -> set_data_action(DATA_WAITING);

		// 设置返回命令
		fsm_session -> post_transfer_ok(); // 发送到客户端
		return 0;
	}

	/** 处理命令 —— TYPE */
	void finite_state_machine::com_TYPE(session *fsm_session)
    {
        if(fsm_session -> cmp_processing_command("A") == 0)
        {
			fsm_session -> set_ret_command("200 Type set to A.\r\n");
        }
        else
		if(fsm_session -> cmp_processing_command("I") == 0)
		{
			fsm_session -> set_ret_command("200 Type set to I.\r\n");
		}
		else
		{
			fsm_session -> set_ret_command
				("504 Command not implemented for the specified argument.\r\n");
		}
		fsm_session -> post();
	}

}
