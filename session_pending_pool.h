#ifndef _SESSION_PENDING_POOL_H_
#define _SESSION_PENDING_POOL_H_
#include "common_include.h"
#include "session_task.h"

namespace ftp_server
{
    struct _session_pending_item
    {
        session_task *deal_task;
        TAILQ_ENTRY(_session_pending_item) queue_entry;
    };
	class session_pending_pool
	{
	public:
		TAILQ_HEAD(, _session_pending_item) session_pending_head;
	public:
		session_pending_pool()
		{
		     TAILQ_INIT(&session_pending_head);
		};
		~session_pending_pool()
		{
			_session_pending_item *temp;
			temp = TAILQ_FIRST(&session_pending_head);
			while( temp != NULL )
			{
				TAILQ_REMOVE( &session_pending_head, temp, queue_entry);
				if( temp->deal_task != NULL )
				{
//					temp->deal_task = NULL;
				}
				delete temp;
				temp = TAILQ_FIRST( &session_pending_head );
			}
		};

	};

}

#endif /* _SESSION_PENDING_POOL_H_ */
