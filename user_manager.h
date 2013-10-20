#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_

namespace ftp_server
{
	class user_manager
	{
	private:
		map<char *, _userlist *>  user_map;	// 存储用户信息容器

	public:
		user_manager()
		{
			fstream file_stream;
			_userlist *user_list=NULL;
			file_stream.open(USERLISTPATH, ios::in);
			if( !file_stream )
			{
				cout<<"Read User List Faile, Make sure the file \"./conf/userlist\" is exist."
					<<endl;	//	获取用户列表失败，只支持Anonymous用户
				LOG(ERROR, "Read User List Faile, Make sure the file \"./conf/userlist\" is exist. The Server is Exit.");
				exit(1);
			}   
			else
			{
				char *userlist_buf;
				userlist_buf = new char[FILEBUFSIZE];	// 信息缓存区
				char *temp;
				temp = new char[FILEBUFSIZE];		
				int off = 0;		
				int sum = 0;		// 已处理信息长度
				int read_len;		// 待处理信息长度
				int len;			

				while( !file_stream.eof() )
				{
					//	读取用户列表
					memset(userlist_buf, 0, FILEBUFSIZE);
					file_stream.read(userlist_buf + off, FILEBUFSIZE - off);
					read_len = file_stream.gcount();

	  	   			do
					{
						user_list = new _userlist;
		
						/** 获取用户名 */
						memset(temp, 0, 1024);
						strncpy(temp, userlist_buf + sum, read_len);
						memset(user_list -> user, 0, USERLEN);
						strtok(temp,  " ");
						len = strlen(temp);
						strncpy(user_list -> user, temp, len);
	
						++len;
						sum += len;
						read_len -= len;
	   
						/** 获取密码 */
						memset(temp, 0, 1024);
						strncpy(temp, userlist_buf + sum, read_len);
						memset(user_list -> pass, 0, PASSLEN);
						strtok(temp, " ");
						len = strlen(temp);
						strncpy(user_list -> pass, temp, len);

						++len;
						sum += len;
						read_len -= len;

						/** 获取上传权限 */
						memset(temp, 0, 1024);
						strncpy(temp, userlist_buf + sum, read_len);
						strtok(temp, " ");
						len = strlen(temp);
						if(strncasecmp(temp, "true", 4) == 0)
						{
							user_list -> upload_per = true;
						}
						else
						{
							user_list -> upload_per = false;
						}

						++len;
						sum += len;
						read_len -= len;

						/** 获取下载权限 */
						memset(temp, 0, 1024);
						strncpy(temp, userlist_buf + sum, read_len);
						strtok(temp, " ");
						len = strlen(temp);
						if(strncasecmp(temp, "true", 4) == 0)
						{
							user_list -> download_per = true;
						}
						else
						{
							user_list -> download_per = false;
						}

						++len;
						sum += len;
						read_len -= len;

						/** 获取删除权限 */
						memset(temp, 0, 1024);
						strncpy(temp, userlist_buf + sum, read_len);
						strtok(temp, "\n");
						len = strlen(temp);
						if(strncasecmp(temp, "true", 4) == 0)
						{
							user_list -> dele_per = true;
						}
						else
						{
							user_list -> dele_per = false;
						}

						++len;
						sum += len;
						read_len -= len;

						/** 将用户信息插入容器 */
						pair<map<char *, _userlist *>::iterator, bool> 
							insert_pair;
						insert_pair = user_map.insert(pair<char *, _userlist *>
							(user_list ->user, user_list));

					}while( read_len );
				}
				delete[] userlist_buf;
				delete[] temp;
			}
			delete user_list;
			LOG(INFO, "Load User List Success.");
		};
		~user_manager()
		{
			user_map.erase(user_map.begin(), user_map.end()); 
		};

	public:
		_userlist *seek_user( char *user )
		{
			_userlist *user_list;
	        map<char *, _userlist *>::iterator iter;
	        iter = user_map.begin();
	        while( iter != user_map.end() )
	        {
	            if( strncasecmp(user, (char *)iter->first, strlen(user))==0)
	            {
	                user_list = (_userlist *)iter->second;
//cout<<"PASSWORD:"<<user_list -> pass<<endl;
	                return user_list;
//	                break;
	            }
	            else
				{
					++iter;
				}
			}
			return NULL;
		};

	};
};
#endif
