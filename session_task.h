#ifndef _SESSION_TASK_H_
#define _SESSION_TASK_H_
#include "common_def.h"

namespace ftp_server
{
	class session_task
	{
	private:
		int fd;
//		int data_fd;
		int action;
		char *buf;
		int buf_len;
	public:
		session_task(int new_fd)
		{
			fd = new_fd;
			action = -1;
			buf_len = -1;
			buf = NULL;
		};


		session_task(int new_fd, int new_buf_len)
		{
			fd = new_fd;
			action = -1;
			buf_len = new_buf_len;
			buf = new char[buf_len];
			memset(buf, 0, buf_len);
		};

		session_task(int new_fd, int new_action, int new_buf_len)
		{
			fd = new_fd;
			action = new_action;
			buf = NULL;
			if(new_buf_len != 0)
			{
				buf_len = new_buf_len;
				buf = new char[buf_len];
				memset(buf, 0, buf_len);
			}
		};

		session_task(int new_fd, int new_action, char* new_buf, int new_buf_len)
		{
			fd = new_fd;
			action = new_action;
			buf_len = new_buf_len;
			buf = new char[buf_len];
			strncpy(buf, new_buf, buf_len);
		};
		~session_task()
		{
			if(buf != NULL)
			{
				delete buf;
				buf = NULL;
			}
		};
		int get_fd()
		{
			return fd;
		};
		int get_action()
		{
			return action;
		};
		void set_action(int new_action)
		{
			action = new_action;
		}
		int get_buf_len()
		{
			return buf_len;
		};
		void set_buf_len(int new_buf_len)
		{
			buf_len = new_buf_len;
		}
		char* get_buf()
		{
			return buf;
		};
	};
}
#endif /* _SESSION_TASK_H_ */
