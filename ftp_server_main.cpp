#include "common_include.h"
#include "common_def.h"

#include "TCP_client.h"
#include "TCP_manager.h"
#include "TCP_interface.h"
#include "TCP_tool.h"

#include "session_manager.h"


#include "file_index.h"
#include "disk_task.h"
#include "disk_info.h"
#include "disk_pending_pool.h"
#include "disk_fsm.h"
#include "disk_thread.h"
#include "disk_thread_pool.h"
#include "disk_manager.h"

#include "finite_state_machine.h"

using namespace std;
using namespace ftp_server;


namespace ftp_server
{
	TCP_interface *g_tcp;
	session_manager *g_session_man;
	disk_manager *g_disk_man;
	_ftpserver_config g_config;
	TCP_tool *g_tcp_tool;
	finite_state_machine g_fsm;

}

bool init_config( const char *path )
{
    string spath;
    spath.assign( path );
    try
    {
        ConfigFile config(spath);

        try
        {
            config.readInto( g_config.server_home, "server_home" );
            config.readInto( g_config.log_config_file , "server_log_config_file" );
            config.readInto( g_config.session_thread_num , "session_thread_num" );
            config.readInto( g_config.disk_thread_num , "disk_thread_num" );
            config.readInto( g_config.client_life , "client_life" );
			config.readInto( g_config.time_interval , "time_interval" );
            config.readInto( g_config.server_port , "server_port" );
			config.readInto( g_config.server_LAN_ip , "server_LAN_ip" );	//	内网IP
			config.readInto( g_config.server_WAN_ip , "server_WAN_ip" );	//	外网IP
            config.readInto( g_config.init_path , "init_path" );
	        config.readInto( g_config.auth_ip , "auth_ip" );
			config.readInto( g_config.auth_port , "auth_port" );
	        config.readInto( g_config.start_port , "start_port" );
			config.readInto( g_config.end_port , "end_port" );
        }
        catch( ConfigFile::key_not_found& ex )
        {
            cerr<<"Cannot Found key : " << ex.key <<endl;
            return false;
        }
        return true;

    }

    catch(ConfigFile::file_not_found& ex )
    {
        cerr<<"cannot open config file: "<<spath<<endl;
        return false;
    }
}

int main(int argc, char **argv)
{
	char config_pathname[PATHSIZE];

	strncpy(config_pathname, "./conf/ftp.conf", PATHSIZE);
	
	cout<<"The Hyper Transfer server is starting up."<<endl;

	signal(SIGPIPE, SIG_IGN);

	/** Read Config File */
    if (!init_config(config_pathname))
    {
        cerr<<"Cannot Init Config : "<< config_pathname << endl;
        exit(1);
    }
	else
	{
		cout<<"Config init success."<<endl;
	}
  	setenv( "FTP_SERVER_HOME" , g_config.server_home.c_str() , 1);
	
	/** Init Logger */
	if( init_logger( g_config.server_home + g_config.log_config_file )==false)
	{
		cerr<<"Cannot Init Log "<< g_config.server_home + g_config.log_config_file << endl;
        exit(1);
	}
	else
	{
		LOG( INFO , "Log Init Success" );
		cout<<"Log init success."<<endl;
	}

	g_tcp_tool = new TCP_tool();

	g_tcp = new TCP_interface();
    g_tcp -> set_timeout( g_config.time_interval, g_config.client_life);
	// 时间间隔 会话生命

	/*启动支持TCP协议接口*/
	if(g_tcp -> start_proc() == false)
	{
		exit(1);
	}
	
	g_session_man = new session_manager();
	/*启动会话管理器*/
	if(g_session_man -> start_proc() == false)
	{
		exit(1);
	}

	g_disk_man = new disk_manager();
	/*启动磁盘管理器*/
	if(g_disk_man -> start_proc() == false)
	{
		exit(1);
	}

	while(1) 
	{
		sleep((unsigned int)10);
	}

    LOG( INFO , "The Hyper Transfer server is closed" );
    cout<<"The Hyper Transfer server is closed."<<endl;

	return 0;
}
