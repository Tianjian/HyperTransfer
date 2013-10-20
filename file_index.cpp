#include "common_include.h"
#include "file_index.h"

namespace ftp_server
{
	/** 通过路径(pwd)获取该路径下的详细列表(info) */
	dirlnk *file_index::get_list(char *info ,char *pwd)
	{
		/** 通过路径(pwd)获取该路径下的列表表头(head) */
		dir_lnk *head = get_dir_detail(pwd);
		/** 通过列表表头(head)获取详细列表(info) */
		return get_dir(head, info, pwd);	
	}

	/** 通过路径(pwd)和当前的列表表头(head)获取该路径下的详细列表(info) */
	dirlnk *file_index::get_list(char *info ,char *pwd, dir_lnk *head )
	{
		/** 通过列表表头(head)获取详细列表(info) */
		return get_dir(head, info, pwd);
	}

	/** 通过路径(pwd)获取该路径下的列表表头(head) */
	dirlnk *file_index::get_dir_detail(char * dirname)
	{
		DIR *dir;
		struct dirent *drt;

		dir = opendir(dirname);	//	打开路径(dirname)下的文件夹

		if(dir == NULL)
		{
			perror("Cannot open the desired directory");
			return NULL;
		}

		dirlnk * dir_head = NULL;
		dirlnk * cur_item = NULL;

		while((drt = readdir(dir)) != NULL)	//	读取路径(dirname)下的列表
		{
			/**	忽略“.”和“..”两项 */
			if((strncmp(drt -> d_name, ".", 2) == 0) || 
				(strncmp(drt -> d_name, "..", 3) == 0))
			{
				continue;
			}

			dirlnk *next_item = new dirlnk;

			if(cur_item == NULL)
			{
				cur_item = next_item;
			}
			else
			{
				cur_item -> next = next_item;
			}

			cur_item = next_item;
			if(dir_head == NULL)
			{
				dir_head=cur_item;
			}
			strncpy(cur_item -> d_name, drt -> d_name, strlen(drt -> d_name) + 1);
		}
		if(cur_item != NULL)
		{
			cur_item -> next = NULL;
		}
		closedir( dir);

		return dir_head;
	}

	dirlnk *file_index::get_dir(dirlnk *dir_head, char *dir_info, char *pwd)
	{
		char perm[8][4] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};		
        unsigned int mask=0700;
        struct stat file_stat;

        dirlnk * cur_dir = dir_head;
		dirlnk * dir_temp;
        int i,j,k = 0;
		char s[20];
		char temp_time[13];
		char temp[FILEINFOLEN];
		strncpy(dir_info, "\0", FILEINFOLEN);

        while(cur_dir!=NULL)
		{

			mask=0700;
			info file_info;

	        char name[PATHSIZE];
            strncpy(name, pwd, PATHSIZE);
            name[PATHSIZE-1] = 0;
            strncat(name, "/", PATHSIZE);
			strncat(name, cur_dir -> d_name, PATHSIZE);

			if(lstat(name, &file_stat) == -1)
			{
				cur_dir = cur_dir -> next;
				continue;
			}

			if(S_ISREG(file_stat.st_mode))
			{
				file_info.permission[0] = '-';
			}
			if(S_ISDIR(file_stat.st_mode))
			{
				file_info.permission[0] = 'd';
			}
			i = 3;
			j = 0;

			while(i > 0)
			{
				file_info.permission[1 + j * 3] = 
					perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][0];
				file_info.permission[2 + j * 3] = 
					perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][1];
				file_info.permission[3 + j * 3] = 
					perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][2];
				i--;
				j++;
				mask >>= N_BITS;
			}

			file_info.permission[10] = '\0';
			sprintf(s, "%s ",file_info.permission);
			strncpy(temp, s, FILEINFOLEN);

			sprintf(s, "%4d ",(int)file_stat.st_nlink);
			strncat(temp, s, FILEINFOLEN);

			sprintf(s, "%-8s ", getpwuid(file_stat.st_uid) -> pw_name);
			strncat(temp, s, FILEINFOLEN);

			sprintf(s, "%-8s ", getgrgid(file_stat.st_gid) -> gr_name);
			strncat(temp, s, FILEINFOLEN);

			sprintf(s,"%ld ", (long)file_stat.st_size);
			strncat(temp, s, FILEINFOLEN);

			file_info.mod_time=file_stat.st_atime;
			memset(temp_time, 0, 13);
			strncpy(temp_time, ctime(&file_info.mod_time) + 4, 12);
			sprintf(s, "%s ", temp_time);
			strncat(temp, s, FILEINFOLEN);
         
			sprintf(name, "%s\r\n", cur_dir -> d_name);
			strncat(temp, name, FILEINFOLEN);

			dir_temp = cur_dir;
			cur_dir = cur_dir -> next;
			
			delete dir_temp;
			dir_temp = NULL;
//			if(cur_dir -> d_name[0] =='.')
//			{
//				continue;
//			}
						
			strncat(dir_info, temp, FILEINFOLEN);
			
			++k;
			if( k >= 32 )
			{
				return cur_dir;
			}
		}
		return NULL;
	}

	int file_index::get_file(char *ret_file_info, char *pwd, char *file_name)
	{
		char perm[8][4] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
		
        unsigned int mask=0700;
        struct stat file_stat;

        int i,j;
		char s[20];
		memset(s, 0, 20);
		char temp_time[13];
		memset(temp_time, 0, 13);
		mask = 0700;
		info file_info;

	    char name[PATHSIZE];
		memset(name, 0, PATHSIZE);
        strncpy(name, pwd, PATHSIZE);
        strncat(name, "/", PATHSIZE);
		strncat(name, file_name, PATHSIZE);

		if(lstat(name, &file_stat) == -1)
		{
			return -1;
		}

		if(S_ISREG(file_stat.st_mode))
		{
			file_info.permission[0] = '-';
		}

		i=3;
		j=0;

		while(i > 0)
		{
			file_info.permission[1 + j * 3] = 
				perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][0];
			file_info.permission[2 + j * 3] = 
				perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][1];
		 	file_info.permission[3 + j * 3] = 
				perm[(file_stat.st_mode & mask) >> (i - 1) * N_BITS][2];
			i--;
			j++;
			mask >>= N_BITS;
		}

		file_info.permission[10]='\0';
		sprintf(s,"%s ",file_info.permission);
		strncat(ret_file_info, s, INDEXSIZE);

		sprintf(s,"%4d ", (int)file_stat.st_nlink);
		strncat(ret_file_info, s, INDEXSIZE);

		sprintf(s,"%-8s ", getpwuid(file_stat.st_uid) -> pw_name);
		strncat(ret_file_info, s, INDEXSIZE);

		sprintf(s,"%-8s ", getgrgid(file_stat.st_gid) -> gr_name);
		strncat(ret_file_info, s, INDEXSIZE);

		sprintf(s,"%ld ", file_stat.st_size);
		strncat(ret_file_info, s, INDEXSIZE);

		file_info.mod_time = file_stat.st_atime;
		strncpy(temp_time, ctime(&file_info.mod_time) + 4, 12);
		sprintf(s,"%s ", temp_time);
		strncat(ret_file_info, s, INDEXSIZE);
         
		strncat(ret_file_info, file_name, INDEXSIZE);
		strncat(ret_file_info, "\r\n", INDEXSIZE);

		return 0;
	}

	// 查找第一个参数，flag标志是否为绝对路径
	void file_index::get_para1(char * cmmd, char *para1, int& flag)
	{
		char *p ;
		char *q;
		p = strchr(cmmd,' ');
		p++;
		while(*p == ' ')
		{
			p++;
		}
		q=p;
		if(*p =='/')
		{
			flag = 1;
		}
		else 
		{
			flag = 0;
		}
		cout<<"q: "<<q<<endl;
		p = strchr(q,' ');
		cout<<"q: "<<q<<endl;
		if(p ==NULL)
		{
			p = strchr(q ,'\r');
		}
		cout<<"q: "<<q<<endl;
		strncpy(para1,q,p-q);
	}
}
