#ifndef _SESSION_H_
#define _SESSION_H_

namespace ftp_server
{
extern _ftpserver_config g_config;
	class session
	{
	private:
		int fd;			//	控制连接

		int action;			// 控制连接动作
		int data_action;	// 数据连接动作

		/** 存储写数据的队列 */
    	TAILQ_HEAD( , _data_buffer_q ) data_write_q;

		/** 会话基本信息 */
		char client_ip[IPLEN];	// 该客户端IP
		char server_ip[IPLEN];	// 根据客户端为内外网指定服务器IP

		/** 会话基本信息 */
		char username[USERLEN];	//	用户名
		char password[PASSLEN];	//	密码
		bool log_in;		//	用户登录标志
		_userlist *user_info;	//	存储用户名信息
		bool upload_per;	//	上次权限
		bool download_per;	//	下载权限
		bool dele_per;		//	删除权限

		char root_path[PATHSIZE];		// 用户登录根目录
		int root_path_len;					// 当前目录长度
		char cur_path[PATHSIZE];		// 用户当前目录
		int path_len;					// 当前目录长度

		char recv_command[COMMANDSIZE];		// 服务器接收的控制命令
		unsigned int recv_command_len;	// 命令的长度
		char ret_command[COMMANDSIZE];	// 服务器返回的命令
		unsigned int ret_command_len;		// 返回命令的长度
		char processing_command[COMMANDSIZE];	// 服务器处理的命令
		unsigned int processing_command_len;	// 处理命令的长度

		/** 列表信息 */
/*		dirlnk *cur_dir;			//	列表表头
		char list_info[INDEXSIZE];	//	存储列表信息
*/
		char file_name[PATHSIZE];	// 文件名
		long file_len;		// 文件大小
		long rest_size;		// 续传位置（上传下载）
		long appe_size;		// 续传位置（上传）
//		int file_handle;			// 文件句柄


	    struct timeval start_time;		// 传输起始时间
		struct timeval end_time;		// 传输结束时间
	    struct timezone tz;

		time_t last_operate;		// 会话最后操作时间
	public:
		session(session_task *fsm_task)
		{

			/** 初始化句柄及会话动作 */
			fd = fsm_task -> get_fd();
			action = fsm_task -> get_action();
			data_action = DATA_WAITING;
			TAILQ_INIT( &data_write_q );

			strncpy(client_ip, fsm_task -> get_buf(), IPLEN );	// 初始化客户端IP

			log_in = false;	// 登录标志为未登录
			user_info = NULL;	//用户信息空
			memset(username, 0, USERLEN);
			upload_per = false;	//无上传权限
			download_per = false;	//无下载权限
			dele_per = false;	//无删除权限

			/** 初始化用户根路径和当前路径 */
			strncpy(root_path, g_config.init_path.c_str(), PATHSIZE);
			root_path_len = strlen(root_path) - 1;
			strncpy(cur_path, g_config.init_path.c_str(), PATHSIZE);
            path_len = 0;

			/** 初始化控制连接接收和返回信息的长度 */
			recv_command_len = 0;
            ret_command_len = 0;

//			cur_dir = NULL;	// 列表队列为空

//			file_handle = -1;	// 初始化文件句柄
			file_len = 0;		// 空文件文件长度为0

			rest_size = 0;		// 续传位置为0
			appe_size = 0;		// 续传位置为0


		};
		~session()
		{
			struct _data_buffer_q *bufq;
			while( !TAILQ_EMPTY(&data_write_q) )
			{
				bufq = TAILQ_FIRST(&data_write_q);
				TAILQ_REMOVE( &data_write_q, bufq, data_entries);
				if( bufq->buf != NULL )
				{
//					delete[] bufq->buf;
					bufq -> buf = NULL;
				}
        		delete bufq;
				bufq = NULL;
	        }
//			delete[] list_info;
//			list_info = NULL;
			user_info = NULL;
		};

		void post();
		void data_post();
		void index_post();
		void pasv_post();
		void post_transfer_ok();


		int get_fd()
		{
			return fd;
		};

		void set_server_ip()
		{
			int a=0,i;
			if(strncmp(client_ip, "192.168", 7) == 0)
			{
				strncpy(server_ip, g_config.server_LAN_ip.c_str(), IPLEN);
//cout<<"ip"<<server_ip<<endl;
			}
			else
			if(strncmp(client_ip, "10.", 3) == 0)
			{
				strncpy(server_ip, g_config.server_LAN_ip.c_str(), IPLEN);
//cout<<"ip"<<server_ip<<endl;
			}
			else
			if (strncmp(client_ip, "172.", 4) == 0)
			{
		        for(i=4; client_ip[i]!='.'; i++)
		        {
		            a = a * 10 + client_ip[i] - '0';
		        }
				if( a >= 16 && a <= 31)
				{
					strncpy(server_ip, g_config.server_LAN_ip.c_str(), IPLEN);
//cout<<"ip"<<server_ip<<endl;
				}
				else
				{
					strncpy(server_ip, g_config.server_WAN_ip.c_str(), IPLEN);
//cout<<"ip"<<server_ip<<endl;
				}
			}
			else
			{
				strncpy(server_ip, g_config.server_WAN_ip.c_str(), IPLEN);
//				cout<<"ip"<<server_ip<<endl;

			}
		};

		char* get_server_ip()
		{
			return server_ip;
		};		
		
		char* get_client_ip()
		{
			return client_ip;
		};

		void set_recv_command(const char *buf)
		{
			strncpy(recv_command, buf, COMMANDSIZE);
			recv_command_len = strlen(recv_command);
		};

		void cat_recv_command(const char *buf, int len)
		{
			/** 将新的命令连接到原来的命令后面 */
			strncpy(recv_command + recv_command_len, buf, COMMANDSIZE);
			recv_command_len = strlen(recv_command);
//			cout<<"cat:"<<recv_command<<endl;
		};

		int cmp_recv_command(const char *buf)
		{
			return strncasecmp(recv_command, buf, COMMANDSIZE);
		};

		char* get_recv_command()
		{
			char backup_command[COMMANDSIZE];
			memset(backup_command, 0, COMMANDSIZE);
			strncpy(backup_command, recv_command, recv_command_len);
			strtok(backup_command, "\r\n");
			if(recv_command_len == strlen(backup_command))
			{
				return NULL;
			}
			else
			{
				return recv_command;
			}
		};
		void reset_recv_command(int len)
		{			
			strncpy(recv_command, recv_command+len, COMMANDSIZE);
			recv_command_len = strlen(recv_command);
		};

		char* get_ret_command()
		{
			return ret_command;
		};

		void set_ret_command(const char* buf)
		{
			strncpy(ret_command, buf, COMMANDSIZE);
			ret_command_len = strlen(ret_command);
		};

		void set_ret_command(const char* buf, int len)
		{
			ret_command_len = len;
			memset(ret_command, 0, COMMANDSIZE);
			strncpy(ret_command, buf, len);
		};

		void set_processing_command(const char* buf, int len)
		{
			processing_command_len = len;
			strncpy(processing_command, buf, COMMANDSIZE);
		};

		void set_processing_command(const char* buf)
		{
			strncpy(processing_command, buf, COMMANDSIZE);
			ret_command_len = strlen(processing_command);
		};

		int cmp_processing_command(const char *buf)
		{
			return strncasecmp(processing_command, buf, COMMANDSIZE);
		};

		int cmp_processing_command(const char *buf, int len)
		{
			return strncasecmp(processing_command, buf, len);
		};

		char *get_processing_command()
		{
			return processing_command;
		};

		void set_data_action(int new_action)
		{
			data_action = new_action;
		};

		int get_data_action()
		{
			return data_action;
		};

		void set_action(int new_action)
		{
			action = new_action;
		};

		int get_action()
		{
			return action;
		};

		bool get_login()
		{
			return log_in;
		};

		void set_login(bool login_flag)
		{
			log_in = login_flag;
		};

		char *get_root_path()
		{
			return root_path;
		};

		int get_root_path_len()
		{
			return root_path_len;
		};

		void set_cur_path(const char *path)
		{
			strncpy(cur_path, path, PATHSIZE);
		};

		char *get_cur_path()
		{
			return cur_path;
		};

		void set_file_len(long size)
		{
			file_len = size;
		};

		void set_rest_size(long rest)
		{
			rest_size = rest;
		};

		long get_rest_size()
		{
			return rest_size;
		};

		void set_appe_size(long appe)
		{
			appe_size = appe;
		};

		long get_appe_size()
		{
			return appe_size;
		};

		void set_file_name(const char *a_path)
		{
			strncpy(file_name, a_path, PATHSIZE);
		};

		char *get_file_name()
		{
			return file_name;
		};

		char *get_relative_name()
		{
			return file_name + root_path_len;
		};

		long get_file_len()
		{
			return file_len;
		};

		/** 设置文件传输起始时间 */
		void set_start_time()
		{
			gettimeofday (&start_time , &tz);
	    };

		/** 设置文件传输结束时间 */
		void set_end_time()
		{
			gettimeofday (&end_time, &tz);
	    };

		/** 计算文件传输持续时间 */
		long get_duration()
		{
			return ( ( end_time.tv_sec - start_time.tv_sec ) * 1000
				+ ( end_time.tv_usec - start_time.tv_usec) / 1000 );
		};


		/** 设置用户信息 */
		void set_user_info(_userlist *user)
		{
			user_info = user;
		};

		/** 获取用户名 */
		char *get_username()
		{
			return username;
		};

		/** 设置用户名 */
		void set_username(const char* temp)
		{
			strncpy(username, temp, USERLEN);
		};

		/** 比较用户名 */
		int cmp_username(const char *buf)
		{
			return strncasecmp(username, buf, USERLEN);
		};

		/** 设置密码 */
		void set_password()
		{
			strncpy(password, user_info -> pass, PASSLEN);
		};

		/** 比较密码 */
		int cmp_password()
		{
			return strncmp(processing_command, password, PASSLEN);
		};

		/** 设置用户的上传、下载、删除权限 */
		void set_permission()
		{
			upload_per = user_info -> upload_per;
			download_per = user_info -> download_per;
			dele_per = user_info -> dele_per;
		};

        /** 设置用户的上传、下载、删除权限 */
        void set_permission(bool flag)
        {
            upload_per = flag;
            download_per = flag;
            dele_per = flag;
        };
		/** 获取用户的上传权限 */
		bool get_upload_per()
		{
			return upload_per;
		};

		/** 获取用户的下载权限 */
		bool get_download_per()
		{
			return download_per;
		};

		/** 获取用户的删除权限 */
		bool get_dele_per()
		{
			return dele_per;
		};
	};
}

#endif /* _SESSION_H_ */
