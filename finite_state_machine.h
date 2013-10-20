#ifndef FINITE_STATE_MACHINE_H_
#define FINITE_STATE_MACHINE_H_

namespace ftp_server
{
	class finite_state_machine
	{
	public:
		finite_state_machine()
		{
		};
		~finite_state_machine()
		{
		};
	public:
		int roll_state_machine(session_task *fsm_task);
		int run_fsm_accept(session *fsm_session);

		int run_fsm_command(session *fsm_session);
		int run_data_uploading(session *fsm_session);
		int run_data_upload(session *fsm_session);
		int run_finish_upload(session *fsm_session);
		int run_finish_download(session *fsm_session);
		int run_transfer_ok(session *fsm_session);
		int run_datafd_error(session *fsm_session);

		int run_data_error(session *fsm_session);


		int run_delete_session(session *fsm_session);

		void com_BAD(session *fsm_session);		//指令错误或参数缺失。
		void com_USER(session *fsm_session);	
		void com_PASS(session *fsm_session);

		int com_PASV(session *fsm_session);			//	解析PASV命令
		int run_pasv(session *fsm_session, int port_num);	//	返回PASV返回码

        int com_LIST(session *fsm_session);			//	解析LIST命令
		int run_list_load(session *fsm_session);	//	接受数据连接，发送返回码

		int com_STOR(session *fsm_session);			//	解析STOR命令
		int run_stor_load(session *fsm_session);	//	接受数据连接
		int run_stor_start(session *fsm_session);	//	发送返回码

		int com_RETR(session *fsm_session);			//	解析RETR命令
		int run_retr_load(session *fsm_session);	//	接受数据连接，发送返回码


        void com_TYPE(session *fsm_session);
		void com_CWD(session *fsm_session);
		void com_PWD(session *fsm_session);
		void com_MKD(session *fsm_session);
		void com_RMD(session *fsm_session);
		void com_DELE(session *fsm_session);
		void com_SYST(session *fsm_session);
		void com_FEAT(session *fsm_session);
		void com_QUIT(session *fsm_session);
        void com_SIZE(session *fsm_session);
		void com_REST(session *fsm_session);
		void com_APPE(session *fsm_session);

		char *change_path(char *cur_path, char *cwd, char *root_path);
		char *com_CDUP(char *temp_path);
		int get_free_port(session *fsm_session);
	};
}
#endif
