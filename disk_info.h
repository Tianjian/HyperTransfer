#ifndef _DISK_INFO_H_
#define _DISK_INFO_H_

using namespace ftp_server;

namespace ftp_server
{
extern _ftpserver_config g_config;
	class disk_info
	{
	private:
		int fd;			//	控制连接

		int data_action;	// 数据连接动作

		char cur_path[PATHSIZE];		// 用户当前目录
		int path_len;					// 当前目录长度

		/** 列表信息 */
		dirlnk *cur_dir;			//	列表表头
		char list_info[INDEXSIZE];	//	存储列表信息

//		char file_name[PATHSIZE];	// 文件名
		long file_len;		// 文件大小
		long rest_size;		// 续传位置（上传下载）
		int file_handle;	// 文件句柄

		char file_buffer[FILEBUFSIZE];	// 文件数据缓存区
		int buf_len;					// 缓存数据长度

	    struct timeval start_time;		// 传输起始时间
		struct timeval end_time;		// 传输结束时间
	    struct timezone tz;

		file_index g_file_index;
	public:
		disk_info(disk_task *fsm_task)
		{

			/** 初始化句柄、会话动作、用户当前路径 */
			fd = fsm_task -> get_ctrl_fd();
			data_action = fsm_task -> get_action();
			strncpy(cur_path, fsm_task -> get_cur_path(), PATHSIZE);
			rest_size = fsm_task -> get_rest_size();	// 续传位置为0

			cur_dir = NULL;	// 列表队列为空

			file_handle = -1;	// 初始化文件句柄
			file_len = 0;		// 空文件文件长度为0

		};
		~disk_info()
		{
			if(file_handle != -1)
			{
				close(file_handle);
			}
		};

		void data_post();

		int get_fd()
		{
			return fd;
		};

		int get_data_action()
		{
			return data_action;
		};
		
		void set_data_action(int new_action)
		{
			data_action = new_action;
		};

		void set_cur_path(char *path)
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

		void set_cur_dir( dirlnk *dir )
		{
			cur_dir = dir;
		};

		dirlnk *get_cur_dir()
		{		
			return cur_dir;
		};

		void open_create()
		{
			file_handle = open(cur_path, O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
//			cout<<"open_create():"<<file_handle<<endl;
		};

		void open_writeonly()
		{
			file_handle = open(cur_path, O_WRONLY);
//			cout<<"open_writeonly():"<<file_handle<<endl;
		};
		
		void open_readonly()
		{
			file_handle = open(cur_path, O_RDONLY);
//			cout<<"open_readonly():"<<file_handle<<endl;
		};

		void seek_file_handle()
		{
			lseek(file_handle, rest_size, SEEK_SET);
		};

		int read_from_disk()
		{
			buf_len = read(file_handle, file_buffer, FILEBUFSIZE);
			return buf_len;
		};

		int write_to_disk(disk_task *input_task)
		{
			return write(file_handle, input_task -> get_file_buffer(), input_task -> get_buf_len());
		};

		void close_file_handle()
		{

//			cout<<"close_file_handle:"<<file_handle<<endl;
			if(file_handle != -1)
			{
				close(file_handle);
			}
		};

		dirlnk *get_list()
		{
			cur_dir = g_file_index.get_list(list_info, cur_path);
			return cur_dir;
		};
				
		dirlnk *get_list(int i)
		{
			cur_dir = g_file_index.get_list(list_info, cur_path, cur_dir);
			return cur_dir;
		};

		char* get_file_buffer()
		{
			return file_buffer;
		};

		void clear_list_info()
		{
			buf_len = 0;
			memset(list_info, 0, INDEXSIZE);
		};

		int get_buf_len()
		{
			return buf_len;
		}

		char* get_list_info()
		{
			return list_info;
		};

		int get_list_info_len()
		{
			return strlen(list_info);
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

	};
}

#endif /* _DISK_INFO_H_ */
