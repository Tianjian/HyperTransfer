#include "common_include.h"
#include "common_def.h"
#include "util.h"

string Util::get_file_signature(string file_path)
{
	int fd;
	long file_size = 0;
	char file_size_char[FILESIZE_CHARLEN];	// 文件大小
	struct stat file_info;
	const char* file_path_char = file_path.c_str();
	string file_signature;
	char md5_result[MD5SUM_CHARLEN];
	char buf[READ_BLOCK];
	if(	stat(file_path_char, &file_info) != -1)
	{
		/**
			S_ISREG是否是一个常规文件.
			S_ISLNK(st_mode):是否是一个连接.
			S_ISDIR是否是一个目录.
			S_ISCHR是否是一个字符设备.
			S_ISBLK是否是一个块设备.
			S_ISFIFO是否是一个FIFO文件.
			S_ISSOCK是否是一个SOCKET文件.
		*/
		if(S_ISREG(file_info.st_mode))
		{
			file_size = (long)file_info.st_size;

			sprintf(file_size_char, "%ld ", file_size);
		}
		else
		{
			file_signature = file_path;
			file_signature.append(" is not a regular file");

			return file_signature;
		}
	}
	else
	{
		return strerror(errno);
	}

	fd = open(file_path_char, O_RDONLY);

	if(file_size <= SMALL_FILE)
	{
		get_md5sum(fd, file_size, md5_result, buf);
		strncat(md5_result, "_", 2);
		file_signature = md5_result;
		file_signature.append(file_size_char);
	}
	else
	{
		get_md5sum(fd, FILE_BLOCK, md5_result, buf);
		strncat(md5_result, "_", 2);
		file_signature = md5_result;

		lseek(fd, file_size / 2, SEEK_SET);
		get_md5sum(fd, FILE_BLOCK, md5_result, buf);
		strncat(md5_result, "_", 2);
		file_signature.append(md5_result);

		lseek(fd, -FILE_BLOCK , SEEK_END);
		get_md5sum(fd, FILE_BLOCK, md5_result, buf);
		strncat(md5_result, "_", 2);
		file_signature.append(md5_result);

		file_signature.append(file_size_char);
	}

	close(fd);
	toLowerString(file_signature);
	return file_signature;
}

void Util::get_md5sum(int fd, long file_size, char* result, char* buf)
{
	int i;
	int size = 0;
	long total = 0;
	char tmp[3] = {'\0'};

	MD5_CTX ctx;
	MD5_Init(&ctx);

	while(total < file_size)
	{
		size = read(fd, buf, READ_BLOCK);
		total += size;
		MD5_Update(&ctx, buf, size);
	}

	unsigned char md[16];
	MD5_Final(md, &ctx);

	memset(result, 0, MD5SUM_CHARLEN);
	for(i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02X", md[i]);
		strncat(result, tmp, 3);
	}
}

void Util::toLowerString(string &str)
{
    transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
}
