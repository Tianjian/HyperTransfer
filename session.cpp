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



namespace ftp_server
{
	extern TCP_interface *g_tcp;
	extern session_manager *g_session_man;
	extern disk_manager *g_disk_man;

	void session::post()
	{
		g_tcp -> post(fd, ret_command, ret_command_len);
	}

	void session::post_transfer_ok()
	{
		g_tcp -> post_transfer_ok(fd, EVENT_NOTIFY_TRANS_OK, ret_command, ret_command_len);
	}
	
/*	void session::post_download_start()
	{
		g_tcp -> post_transfer_ok(fd, EVENT_NOTIFY_DOWN_START, ret_command, ret_command_len);
	}


	void session::data_post()
	{
		g_tcp -> data_post(post_fd, fd, data_fd, file_buffer, buf_len);
	}

	void session::index_post()
	{
		g_tcp -> data_post(post_fd, fd, data_fd, list_info, strlen(list_info));
	}
*/
	void session::pasv_post()
	{
		g_tcp -> post_action(fd, EVENT_NOTIFY_PASV);
	}
}
