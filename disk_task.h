#ifndef _DISK_TASK_H_
#define _DISK_TASK_H_


namespace ftp_server
{
	class disk_task
	{
	private:
		int ctrl_fd;
		int action;
		char cur_path[PATHSIZE];		// 用户当前目录
		long rest_size;					// 续传位置（-1文件尾，0不变，正整数，偏移位置）
		char* file_buffer;
		int buf_len;

	public:
		disk_task(int new_ctrl_fd, int new_action)
		{
			ctrl_fd = new_ctrl_fd;
			action = new_action;
			rest_size = 0;
			file_buffer = NULL;
			buf_len = 0;
		};
		disk_task(int new_ctrl_fd, int new_action, char* new_path)
		{
			ctrl_fd = new_ctrl_fd;
			action = new_action;
			strncpy(cur_path, new_path, PATHSIZE);
			rest_size = 0;
			file_buffer = NULL;
			buf_len = 0;

		};
		disk_task(int new_ctrl_fd, int new_action, char* new_path, long offset)
		{
			ctrl_fd = new_ctrl_fd;
			action = new_action;
			strncpy(cur_path, new_path, PATHSIZE);
			rest_size = offset;
			file_buffer = NULL;
			buf_len = 0;

		};

		~disk_task()
		{
			if(file_buffer != NULL)
			{
				delete[] file_buffer;
				file_buffer = NULL;
			}
		};
		int get_ctrl_fd()
		{
			return ctrl_fd;
		};
		int get_action()
		{
			return action;
		};
		void set_action(int new_action)
		{
			action = new_action;
		};
		char* get_cur_path()
		{
			return cur_path;
		};
		void set_cur_path(char* new_path)
		{
			strncpy(cur_path, new_path, PATHSIZE);
		};
		int cmp_path(char *buf)
		{
			return strncasecmp(cur_path, buf, PATHSIZE);
		};
		int get_rest_size()
		{
			return rest_size;
		};
		void set_rest_size(int new_rest_size)
		{
			rest_size = new_rest_size;
		};
		char* get_file_buffer()
		{
			return file_buffer;
		};
		int get_buf_len()
		{
			return buf_len;
		};
		void set_new_file_buffer(char* new_file_buffer, int new_buf_len)
		{
			if(file_buffer != NULL)
			{
				exit(1);
			}
			file_buffer = new char[FILEBUFSIZE];
			memset(file_buffer, 0, FILEBUFSIZE);
			memcpy(file_buffer, new_file_buffer, new_buf_len);
			buf_len = new_buf_len;
		};
/*		void set_file_buffer(char* new_file_buffer, int new_buf_len)
		{
			memset(file_buffer, 0, FILEBUFSIZE);
			memcpy(file_buffer, new_file_buffer, new_buf_len);
			buf_len = new_buf_len;
		};
*/	};
}
#endif /* _DISK_TASK_H_ */
