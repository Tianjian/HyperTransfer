#ifndef _TCP_TOOL_H_
#define _TCP_TOOL_H_

using namespace std;
namespace ftp_server
{	
	class TCP_tool
	{
	public:
		TCP_tool()
		{
		};

		~TCP_tool()
		{
		};

		/** 设置socket为非阻塞 */
		int set_nonblock(int fd)
		{
			int flags;
			flags = fcntl(fd, F_GETFL);
			if (flags < 0)
			{
				return flags;
			}
			flags |= O_NONBLOCK;
			if (fcntl(fd, F_SETFL, flags) < 0)
			{
			    return -1;
			}
			return 0;
		};

		int get_free_port(TCP_client *client)
		{
	   		int port = -1;
		    int fd = -1;
    		socklen_t len = 0;

    		struct sockaddr_in sin;
		    memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_port = htons(0);
	    	sin.sin_addr.s_addr = htonl(INADDR_ANY);
		   
    		fd = socket(AF_INET, SOCK_STREAM, 0);
//			cout<<"get_free_port():"<<fd<<endl;
	    	if(fd < 0)
			{
				cout<<"socket() error:"<<strerror(errno)<<endl;
	    		return -1;
	    	}
   
		    if(bind(fd, (struct sockaddr *)&sin, sizeof(sin)) != 0)
    		{
        		cout<<"bind() error:"<<strerror(errno)<<endl;
		        close(fd);
			    return -1;
			}
//			cout<<"get_free_port():"<<fd<<endl;

	    	len = sizeof(sin);
		    if(getsockname(fd, (struct sockaddr *)&sin, &len) != 0)
    		{
        		cout<<"getsockname() error:"<<strerror(errno)<<endl;
		        close(fd);
				return -1;
    		}
			if (set_nonblock(fd) < 0)
			{
				LOG(WARN, "failed to set notify socket to non-blocking");
			}
			if(listen(fd,100) < 0)
			{
				LOG(WARN, "listen failed");
				return -1;
			}
//			cout<<"get_free_port():"<<fd<<endl;

//cout<<"create listen_fd:"<<fd<<fsm_session -> file_name<<endl;
			port = ntohs(sin.sin_port);

			client -> set_listen_fd(fd);

			return port;
		};

	};
}

#endif /* _TCP_TOOL_H_ */
