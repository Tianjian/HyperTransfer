#ifndef _DISK_FSM_H_
#define _DISK_FSM_H_

namespace ftp_server
{

	class disk_fsm
	{
	public:
		disk_fsm()
		{
		};
		~disk_fsm()
		{
		};

	public:
		int roll_state_machine(disk_task *fsm_task);

		int run_index_start( disk_info *fsm_disk);
		int run_index_loading( disk_info *fsm_disk );

		int run_retr_start( disk_info *fsm_disk );
		int run_retr_loading( disk_info *fsm_disk );

		int run_stor_start( disk_info *fsm_disk );
		int run_stor_loading( disk_task *input_task, disk_info *fsm_disk );
		int run_stor_finish( disk_task *input_task, disk_info *fsm_disk );
	};
}

#endif /* _DISK_FSM_H_ */
