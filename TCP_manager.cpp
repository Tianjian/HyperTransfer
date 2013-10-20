#include "common_include.h"
#include "common_def.h"

#include "TCP_client.h"
#include "TCP_manager.h"
#include "TCP_interface.h"
#include "TCP_tool.h"


namespace ftp_server
{
	extern _ftpserver_config g_config;
	extern TCP_tool *g_tcp_tool;

	/** 初始化FTP服务器的listen套接字 */
	int	TCP_manager::init_listen_socket()
	{
		int reuseaddr_on = 1;

		struct sockaddr_in listen_addr;

		listen_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(listen_socket < 0)
		{
			LOG(WARN, "listen failed");
			return 1;
		}
		else
		{
			LOG(INFO, "init listen socket success");
		}

		if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, 
			&reuseaddr_on, sizeof(reuseaddr_on)) == -1)
		{
			LOG(WARN, "setsockopt failed");
			exit(1);
		}
		else
		{
			LOG(INFO, "setsockopt success");
		}

		memset(&listen_addr, 0, sizeof(listen_addr));
		listen_addr.sin_family = AF_INET;
		listen_addr.sin_addr.s_addr = INADDR_ANY;

	    if(g_config.server_port > 0 && g_config.server_port < 65535)
		{
			set_ctrl_port(g_config.server_port);
		}
			
		listen_addr.sin_port = htons(get_ctrl_port());

		errno = bind(listen_socket, (struct sockaddr *)&listen_addr,
			sizeof(listen_addr));
		if(errno < 0)
		{
			cout<<"bind failed"<<strerror(errno)<<endl;
        	LOG(WARN, "bind failed"<<strerror(errno));
			exit(1);
		}
		if(listen(listen_socket, 1000) < 0)
		{
			LOG(WARN, "listen failed");
			exit(1);
		}

		if(g_tcp_tool -> set_nonblock(listen_socket) < 0)
		{
			LOG(WARN, "failed to set server socket to non-blocking");
			exit(1);
		}
		return listen_socket;
	}

	bool TCP_manager::add_client(TCP_client *p_client, int fd)
	{
		pair<map< int , TCP_client *>::iterator, bool> insert_pair;

		insert_pair = this -> m_client_map.insert(pair< int, TCP_client *>( fd , p_client ));
		return insert_pair.second ;
	}
	
	TCP_client * TCP_manager::seek_client( int fd )
	{
		map< int , TCP_client *>::iterator iter;
		iter = this -> m_client_map.find( fd );
		if( iter != this -> m_client_map.end() )
		{
			return (TCP_client *)iter->second;
		}
		else
		{
			return NULL;
		}
	}
	
	bool TCP_manager::del_client( int fd )
	{
		map< int , TCP_client* >::iterator iter;
		iter = this -> m_client_map.find( fd );	
		if( iter != m_client_map.end() )
		{
			TCP_client* pclt = (TCP_client* )iter->second;
			m_client_map.erase( iter );
//			LOG(DEBUG, "del_client"<<fd);
			delete pclt;
			pclt = NULL;
		}
		return true;
	}
}
