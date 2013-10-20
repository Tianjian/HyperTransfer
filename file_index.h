#ifndef _FILE_INDEX_H_
#define _FILE_INDEX_H_

#include "common_include.h"

namespace ftp_server
{
	class file_index
	{
	public:
		file_index()
		{
		};
		~file_index()
		{
		};
	public:
		dirlnk *get_list(char *info, char *pwd);
		dirlnk *get_list(char *info, char *pwd, dirlnk *head );
		dirlnk *get_dir_detail(char *dirname);
		dirlnk *get_dir(dirlnk *dir_head, char *dir_info, char *pwd);

//		dirlnk *get_list_a(char *info, char *pwd);
//		dirlnk *get_list_a(char *info, char *pwd, dirlnk *head );
//		dirlnk * get_dir_a(dirlnk *dir_head, char *dir_info, char *pwd);
		int get_file(char *ret_file_info, char *pwd, char *file_name);
		void get_para1(char *cmmd, char *para1, int& flag);
	};
}
#endif
