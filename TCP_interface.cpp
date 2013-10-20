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

using namespace std;

namespace ftp_server
{
	extern session_manager *g_session_man;
	extern disk_manager *g_disk_man;
	extern _ftpserver_config g_config;
	extern TCP_interface *g_tcp;
	extern TCP_tool *g_tcp_tool;

	/** 运行TCP接口线程 */
	void *run_TCP(void *par)
    {
		cout<<"TCP Interface is running."<<endl;
		TCP_interface *p_TCP_i = (TCP_interface *)par;

		TCP_manager *p_TCP_m = p_TCP_i -> get_TCP_manager();	//获取TCP管理器

//		p_TCP_m -> listen_socket = 
			p_TCP_m -> init_listen_socket(); //p_TCP_m -> listen_socket);
		cout<<"Init listen socket success."<<endl;

        p_TCP_i -> init_TCP_event(p_TCP_i);
		cout<<"Init TCP event success."<<endl;
        return NULL;
    }

	/** 主程序通过该函数启动TCP_interface */
    bool TCP_interface::start_proc()
    {
        pthread_t TCP_id;
        int ret;
		
		/*启动支持TCP协议接口的线程*/
        ret = pthread_create(&TCP_id, NULL, run_TCP, (void *)this);
		if(ret != 0)
        {
            cout<<"Create TCP thread error: "<<strerror(ret)<<endl;
            return false;
        }
        else
        {
			cout<<"Create TCP interfae success."<<endl;
            return true;
        }
		pthread_detach(TCP_id);
    }
}
