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

	int finite_state_machine::roll_state_machine(session_task *input_session_task)
	{
		if(input_session_task == NULL)
		{
			LOG(WARN, "input_session_task == NULL");
			return 0;
		}
		int ret = 0;
		int action = input_session_task -> get_action();
		session *temp_session = NULL;

		
		/** 有新用户建立连接 */
		if(SESSION_TASK_CTRL_ACCEPT == action)
		{
			temp_session = new session(input_session_task);
			g_session_man -> add_session(temp_session);
			LOG(INFO, "Add new session");

			ret = g_fsm.run_fsm_accept(temp_session);
			/** 新用户初始化成功 */
			if(0 == ret)
			{
				temp_session -> set_action(CTRL_WAITING);
			}
			/** 新用户初始化失败 */
			else
			{
				;
			}
			return ret;
		}
		else
		{
			temp_session = g_session_man -> seek_session( input_session_task -> get_fd() );
			if(temp_session != NULL)
			{
				LOG(DEBUG, "Session:fd = "<<input_session_task -> get_fd()<<" is exist");
			}
			else
			{
				LOG(DEBUG, "Session:fd = "<<input_session_task -> get_fd()<<" is not exist");
				return -1;
			}
		}



		LOG(DEBUG, "input_session_task -> get_action:"<<hex<<action);
		switch(action)
		{
			/** 发送PASV返回码给客户端 */
			case(SESSION_TASK_PASV):
				ret = run_pasv(temp_session, input_session_task -> get_buf_len());
				if(0 == ret)
				{
					temp_session -> set_action(CTRL_WAITING);
					temp_session -> set_data_action(DATA_PASV_OK);
				}
				/** 新用户初始化失败 */
				else
				{
					;
				}
				break;

			/** data_accept触发后，发出的session_task */
			case(SESSION_TASK_DATA_ACCEPT):
			{
				switch(temp_session -> get_data_action())
				{

				/** LIST传输开始 */
				case(DATA_LIST):
					ret = g_fsm.run_list_load(temp_session);
					break;
				
				/** STOR传输开始 */
				case(DATA_STOR):
					ret = g_fsm.run_stor_load(temp_session);
					break;
					
				/** RETR传输开始 */
				case(DATA_RETR):
					ret = g_fsm.run_retr_load(temp_session);
					break;

				default:
					LOG(DEBUG, "DATA_ACCEPT: unknow data_action: "<<hex<<temp_session -> get_action());
					break;
				}
				break;
			}
			case(SESSION_TASK_STOR_START):
				ret = g_fsm.run_stor_start(temp_session);
				break;

			case(SESSION_TASK_TRANS_OK):
				ret = g_fsm.run_transfer_ok(temp_session);
	//			temp_session -> set_action(CTRL_WAITING);
	//			temp_session -> set_data_action(DATA_WAITING);
				break;

			/** 有命令输入，需要执行 */
			case(SESSION_TASK_READ):
				temp_session -> set_action(SESSION_TASK_READ);
				temp_session -> cat_recv_command( input_session_task -> get_buf(), input_session_task -> get_buf_len());
				ret = g_fsm.run_fsm_command(temp_session);
				if(0 == ret)
				{
					temp_session -> set_action(CTRL_WAITING);
				}
				/** 新用户初始化失败 */
				else
				{
					;
				}
				break;

			/** 客户端断开连接，删除会话 */
			case(SESSION_TASK_DISCONN):
//				cout<<"error:  input_session_task -> get_action:"<<SESSION_TASK_DISCONN<<endl;
				ret = -1;
//				ret = g_fsm.run_delete_session(input_session_task);
				break;

			/** 客户端异常断开，删除会话 */
			case(SESSION_TASK_CONN_ERR):
//				cout<<"error:  input_session_task -> get_action:"<<SESSION_TASK_CONN_ERR<<endl;
				ret = -1;
//				ret = g_fsm.run_delete_session(input_session_task);
				break;
			
			default:
				cout<<"unknow action: "<<action<<endl;
				ret = -1;
				break;
		}
		return ret;
	}


	/** 有新用户建立连接，保存IP， 发送欢迎语 */
	int finite_state_machine::run_fsm_accept(session *fsm_session)
	{		
		fsm_session -> set_server_ip();	//	根据客户是内外网
		fsm_session -> set_ret_command("220 Hyper Transfer is ready.\r\n");
		LOG(INFO, fsm_session -> get_ret_command());
		fsm_session -> post();
		return 0;
	}
		
	/** 用户退出，删除会话 */
	int finite_state_machine::run_delete_session(session *fsm_session)
	{
		int fd = fsm_session -> get_fd();
		g_session_man -> del_session(fd);
		disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_DELE);
		LOG(INFO, "disk_task *fsm_task = new disk_task(fsm_session -> get_fd(), DATA_TASK_DELE");
		g_disk_man -> post_to_disk(fsm_task);
		
		return 0;
	}

	/** 判断命令是否完整，若完整，进入相应的命令处理子状态机 */
	int finite_state_machine::run_fsm_command(session *fsm_session)
	{
		int ret = 0;
        char bak_command[COMMANDSIZE];	// 备份命令
		int bak_len;	// 备份命令的长度

		char deal_command[COMMANDSIZE];	// 处理命令
		int deal_len;	// 处理后命令的长度

		char *left_command = NULL;// = new char[COMMANDSIZE];


		/** 备份一条命令行，以防万一 */
		strncpy(bak_command, fsm_session -> get_recv_command(), COMMANDSIZE);
		bak_command[ COMMANDSIZE -1] = 0;
        bak_len = strlen(bak_command);
		
		/** 获取要处理的命令及其长度 */
        strncpy(deal_command, bak_command, COMMANDSIZE);
		strtok(deal_command, "\r\n");
		deal_len = strlen(deal_command);

		if((deal_len + 2 ) > bak_len)
		{
//			printf("Command is incomplete.\n");//命令不完整，直接返回
			return -1;
		}
		else
		if((deal_len + 2 ) == bak_len)
		{
			/** 命令完整，复制到处理指令区 */
			fsm_session -> set_processing_command(deal_command);
			fsm_session -> reset_recv_command(bak_len);
		}
		else
		{
			/** 命令完整，复制到处理指令区 */
			fsm_session -> set_processing_command(deal_command);
			// 将要处理的命令复制到处理区
			
			left_command = strstr(bak_command, "\r\n");
			fsm_session -> set_recv_command(left_command + 2);
			// 未处理的命令保存在接收命令区
//			printf("!!!!!!!!bak_command%s left_command%s\n", deal_command, left_command);
		}
//		printf("!!!!!!!!deal_command%s\n", deal_command );
        if(strncmp(deal_command, "USER ", 5) == 0)
	    {
			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_USER(fsm_session);
        }
		else
        if(strncmp(deal_command, "PASS ", 5) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_PASS(fsm_session);
        }
		else
		if(strncmp(deal_command, "TYPE  ", 5) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_TYPE(fsm_session);
        }
        else
		if(strncmp(deal_command, "CWD", 3) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 3);
            g_fsm.com_CWD(fsm_session);
        }
        else
		if(strncmp(deal_command, "CDUP", 4) == 0)
        {
 			fsm_session -> set_processing_command(" ..");
            g_fsm.com_CWD(fsm_session);
        }
 		else
		if (strncmp(deal_command, "PWD", 3) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 3);
            g_fsm.com_PWD(fsm_session);
		}
		else
       if(strncmp(deal_command, "APPE ", 5) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_APPE(fsm_session);
        }

 		else
        if(strncmp(deal_command, "SIZE ", 5) == 0)
        {
 			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_SIZE(fsm_session);
        }
		else
        if(strncmp(deal_command, "REST", 4) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_REST(fsm_session);
        }
        else
		if(strncmp(deal_command, "PASV", COMMANDSIZE) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
			g_fsm.com_PASV(fsm_session);
		} 
		else
        if(strncmp(deal_command, "LIST", 4) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 4);
            ret = g_fsm.com_LIST(fsm_session);
        }
		else
        if(strncmp(deal_command, "STOR ", 5) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 5);
            ret = g_fsm.com_STOR(fsm_session);
        }
		else
        if(strncmp(deal_command, "RETR ", 5) == 0)
        {
			fsm_session -> set_processing_command(deal_command + 5);
            ret = g_fsm.com_RETR(fsm_session);
        }
		else
/*		if(strncmp(deal_command, "MKD ", 4) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_MKD(fsm_session);
		}
		else
		if(strncmp(deal_command, "RMD ", 4) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_RMD(fsm_session);
		}
		else
		if(strncmp(deal_command, "DELE ", 5) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 5);
            g_fsm.com_DELE(fsm_session);
		}
*/		
		if (strncmp(deal_command, "SYST", 4) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_SYST(fsm_session);
		}
		else
		if (strncmp(deal_command, "FEAT", 4) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_FEAT(fsm_session);
		}
/*		else
		if (strncmp(deal_command, "QUIT", 4) == 0)
		{
			fsm_session -> set_processing_command(deal_command + 4);
            g_fsm.com_QUIT(fsm_session);
		}
*/		else
		{
            g_fsm.com_BAD(fsm_session);
		}
		return ret;
	}

	/** 错误命令或参数不完整 */
    void finite_state_machine::com_BAD(session *fsm_session)
    {
//		printf("********com_BAD********\n");
		fsm_session -> set_ret_command("501 Bad or missing parameters.\r\n");
		fsm_session -> post();
    }


	/** 处理命令 —— USER */
    void finite_state_machine::com_USER(session *fsm_session)
    {
		int ret;
        char ret_command[COMMANDSIZE];
		_userlist *user_info;
		user_info = NULL;
		ret = fsm_session -> cmp_processing_command("Anonymous");
		if(ret == 0)
		{
			fsm_session -> set_username( "Anonymous" );
			fsm_session -> set_ret_command("331 User name okay, need password.\r\n");
		}
		ret = fsm_session -> cmp_processing_command("BGIShare");
		if(ret == 0)
		{
			fsm_session -> set_username( "BGIShare" );
			fsm_session -> set_ret_command("331 User name okay, need password.\r\n");
		}
		else
		{
			user_info = g_session_man -> p_user_manager -> seek_user(fsm_session -> get_processing_command());
			if(user_info != NULL)
			{
				fsm_session -> set_user_info(user_info);
				fsm_session -> set_username(user_info -> user);
				fsm_session -> set_password();

				strncpy(ret_command, "331 Password required for ", COMMANDSIZE);
				strncat(ret_command, fsm_session -> get_processing_command(), COMMANDSIZE);
				strncat(ret_command, "\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);

				user_info = NULL;
			}
			else
			{
				fsm_session -> set_ret_command("530 Not logged in.\r\n");
			}
		}
		fsm_session -> post();
    }
	
	/** 处理命令 —— PASS */
	void finite_state_machine::com_PASS(session *fsm_session)
    {
        int ret;
        char ret_command[COMMANDSIZE];
		ret = fsm_session -> cmp_username("Anonymous");

		/** 匿名登录不需要验证密码 */
		if(ret == 0)
		{
			fsm_session -> 
				set_ret_command("230 User Anonymous logged in, proceed.\r\n");
			fsm_session -> set_login(true);	//	设置用户为已登录状态
		}
		else
		/** 非匿名用户验证密码 */
		if(fsm_session -> cmp_username("\0") != 0)
		{
			ret = fsm_session -> cmp_password();
			/** 验证通过 */
			if(ret == 0)
			{
				fsm_session -> set_login(true);	//	设置用户为已登录状态
				fsm_session -> set_permission();	//	设置用户权限
				/** 构造返回信息 */
				strncpy(ret_command, "230 User ", COMMANDSIZE);
				strncat(ret_command, fsm_session -> get_username(), COMMANDSIZE);
				strncat(ret_command,  " logged in, proceed.\r\n", COMMANDSIZE);
				fsm_session -> set_ret_command(ret_command);
			}
			else
			{
				fsm_session -> set_ret_command("530 Not logged in.\r\n");
			}
		}
		/** 非匿名用户没有输入密码 */
		else
        {
			fsm_session -> set_ret_command("530 Not logged in.\r\n");
        }
        fsm_session -> post();
	}

	/** 处理命令 —— SYST */
	void finite_state_machine::com_SYST(session *fsm_session)
    {
//		printf("********com_SYST********\n");
        int ret;
		struct utsname name;
		ret = uname(&name);

		if(ret == 0)
        {
			fsm_session -> set_ret_command("215 UNIX Type: L8\r\n");
        }
        else
		{
			fsm_session -> set_ret_command("215 \r\n");
		}
        fsm_session -> post();

	}

	/** 处理命令 —— FEAT */
	void finite_state_machine::com_FEAT(session *fsm_session)
    {
//		printf("********com_FEAT********\n");
		fsm_session -> set_ret_command("211-Extensions supported:\r\n PASV\r\n SIZE\r\n REST\r\n APPE\r\n211 End.\r\n");
        fsm_session -> post();
	}
}
