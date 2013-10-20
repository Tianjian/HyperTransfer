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
	
	/** 处理命令 —— CWD */
	void finite_state_machine::com_CWD(session *fsm_session)
    {
		char ret_command[COMMANDSIZE];

		/**  */
		if( fsm_session -> cmp_processing_command("\0") == 0 )
		{
			fsm_session -> set_cur_path(fsm_session -> get_root_path());
			fsm_session -> set_ret_command
				("504 Command not implemented for the specified argument.\r\n");
		}
		else
		if( fsm_session -> cmp_processing_command(" ", 1) == 0 )
		{

			char jump_path[PATHSIZE];
			char cur_path[PATHSIZE];
			char *a_path;	//	绝对路径
			char *r_path;	//	相对路径

			strncpy(jump_path, fsm_session -> get_processing_command() + 1, PATHSIZE);
			//	recv_command: 客户端要跳转的路径
			strncpy(cur_path, fsm_session -> get_cur_path(), PATHSIZE);

			a_path = change_path(cur_path, jump_path, fsm_session -> get_root_path());
			// a_path: 解析后的绝对路径

			if(a_path == NULL)
			{
				strncpy(ret_command, "550 CWD failed. \"", COMMANDSIZE);
				strncat(ret_command, jump_path, 	COMMANDSIZE);
				strncat(ret_command, "\" : no such file or directory.\r\n",
					COMMANDSIZE);	
				fsm_session -> set_ret_command(ret_command);
			}
			else
			{

				struct stat dir_info;
		
				if(lstat(a_path, &dir_info) < 0)
				{
					strncpy(ret_command, "550 CWD failed. \"", COMMANDSIZE);
					strncat(ret_command, jump_path, COMMANDSIZE);
					strncat(ret_command, "\" : no such file or directory.\r\n",
						COMMANDSIZE);
					fsm_session -> set_ret_command(ret_command);
				}
				else
				if(strstr(a_path, fsm_session -> get_root_path()) == NULL)
				{
					strncpy(ret_command, "550 CWD failed. \"", COMMANDSIZE);
					strncat(ret_command, jump_path, COMMANDSIZE);
					strncat(ret_command, "\" : Permission denied.\r\n",
						COMMANDSIZE);
					fsm_session -> set_ret_command(ret_command);
				}
				else
				if(S_ISDIR(dir_info.st_mode))
				{
					/** 如果路径最后没有"/",补充"/" */
					if(strncmp(a_path + strlen( a_path ) - 1, "/", 1) != 0)
					{
						strncat(a_path,  "/", COMMANDSIZE);
					}

					fsm_session -> set_cur_path( a_path );

					strncpy(ret_command, "250 CWD command successful. \"", 
						COMMANDSIZE);
					r_path = a_path + strlen(fsm_session -> get_root_path()) - 1;
					strncat(ret_command, r_path , COMMANDSIZE); 
					strncat(ret_command, "\" is current directory.\r\n", 
						COMMANDSIZE);
					fsm_session -> set_ret_command(ret_command);
				}
				else
				{
					strncpy(ret_command, "550 CWD failed. \"", COMMANDSIZE);
					r_path = a_path + strlen(fsm_session -> get_root_path());
					strncat(ret_command, r_path , COMMANDSIZE);
					strncat(ret_command + strlen(ret_command), 
						"\" : no such directory, maybe is file.\r\n", 
							COMMANDSIZE);
					fsm_session -> set_ret_command(ret_command);
				}
			}
			a_path = NULL;
			r_path = NULL;
		}
		else
		{
			fsm_session -> set_ret_command("500 Unknown command.\r\n");
		}
		fsm_session -> post();	
	}

	/** 处理命令 —— PWD */
	void finite_state_machine::com_PWD(session *fsm_session)
    {
//		printf("********com_PWD********\n");
        char ret_command[COMMANDSIZE];

		if(fsm_session -> get_login())
		{
			char *a_path = fsm_session -> get_cur_path();
			char *r_path = a_path + fsm_session -> get_root_path_len();
				
			strncpy(ret_command, "257 \"", COMMANDSIZE);
			strncat(ret_command, r_path, COMMANDSIZE);
			strncat(ret_command,  "\" is current directory.\r\n" , COMMANDSIZE);
			fsm_session -> set_ret_command(ret_command);
		}
        else
        {
			fsm_session -> set_ret_command("550 PWD failed.\r\n");
        }
		fsm_session -> post();

	}


	char *finite_state_machine::change_path(char *cur_path, char *cwd, char *root)
	{
		char temp[PATHSIZE];
		
		if(strcmp(cwd, "/") == 0)			// 返回根目录
		{
			return root;
		}
		else
		if(strncmp(cwd, "\0", 1) == 0)		// 返回当前路径
		{
			return cur_path;
		}
		else
		if(strncmp(cwd, "/", 1) == 0)		// 返回绝对路径
		{
			if(strncmp(cwd, "/../", PATHSIZE) == 0)		// ../
			{
				cur_path = com_CDUP(cur_path);
				strncpy(temp, cwd + 4, PATHSIZE);
				return change_path(cur_path, temp, root);
			}
			else
			{
				strncpy(temp, root, PATHSIZE);
				temp[PATHSIZE -1 ] = 0;
                strncat(temp, cwd + 1, PATHSIZE);
				strncpy(cwd, temp, PATHSIZE);
				return cwd;
			}
		}
		else
		if(strncmp(cwd, ".", 1) == 0)
		{
			if(strncmp(cwd + 1, ".", 1) == 0)
			{
				if(strncmp(cwd + 2, ".", 1) == 0)
				{
					if(strncmp(cwd + 3, ".", 1) == 0)	// ....：返回根目录
					{
						return root;
					}
					else
					if(strncmp(cwd + 3, "\0", 1) == 0)	// ...：返回上两级菜单
					{
						cur_path = com_CDUP(cur_path);
						return com_CDUP(cur_path);
					}
					else										//	...任意字符：错误菜单
					{
						return NULL;
					}
				}
				else
				if(strncmp(cwd + 2, "/", 1) == 0)		// ../
				{
					cur_path = com_CDUP(cur_path);
					strncpy(temp, cwd + 3, PATHSIZE);
					return change_path(cur_path, temp, root);
				}
				else
				if(strncmp(cwd + 2, "\0", 1) == 0)		// ..
				{
					return com_CDUP(cur_path);
				}
				else											// ..任意字符：错误菜单
				{
					return NULL;
				}
			}
			else
			if(strncmp(cwd + 1, "/", 1) == 0)			// ./
			{
				strncpy(temp, cwd + 2, PATHSIZE);
				return change_path( cur_path, temp, root);
			}
			else												// .任意字符：错误菜单
			{
				return NULL;
			}
		}
		else													// 相对路径
		{
			strncat(cur_path, cwd, PATHSIZE);
			return cur_path;
		}
	}

	/** 处理命令 —— CDUP */
	char *finite_state_machine::com_CDUP(char *temp_path)
    {
		
		if(strncmp(temp_path, "/\0", 2)==0)
		{
			return temp_path;
		}
		else
		{
			int i;
			i = strlen(temp_path);
			i -= 2;
			for(; i > 0; i--)
			{
				if(strncmp(temp_path + i, "/", 1) == 0)
				{
					strncpy(temp_path, temp_path, i + 1);
					strncpy(temp_path + i + 1, "\0", 1);

					break;
				}
				else
				{
					continue;
				}
			}
			return temp_path;
		}
	}

}
