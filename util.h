#ifndef __UTIL_H__
#define __UTIL_H__

using namespace std;
class Util
{	
public:
	Util()
	{
	};
	~Util()
	{
	};
public:
	string get_file_signature(string file_path); 

private:
	void get_md5sum(int fd, long file_size, char* result, char* buf);
	void toLowerString(string &str);
};

#endif /* __UTIL_H__ */
