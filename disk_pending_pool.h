#ifndef _DISK_PENDING_POOL_H_
#define _DISK_PENDING_POOL_H_

namespace ftp_server
{
	/** 待磁盘处理数据队列 */
    struct _disk_pending_item
    {
		disk_task *deal_task;
        TAILQ_ENTRY(_disk_pending_item) disk_queue_entry;
    };


	class disk_pending_pool
	{
	public:
		TAILQ_HEAD(, _disk_pending_item) disk_pending_head;
	public:
		disk_pending_pool()
		{
		     TAILQ_INIT(&disk_pending_head);
		};
		~disk_pending_pool()
		{
			_disk_pending_item *temp;
			temp = TAILQ_FIRST(&disk_pending_head);
			while( temp != NULL )
			{
				TAILQ_REMOVE( &disk_pending_head, temp, disk_queue_entry);
				if( temp->deal_task != NULL )
				{
					temp->deal_task = NULL;
				}
				delete temp;
				temp = TAILQ_FIRST( &disk_pending_head );
			}
		};
		void tailq_insert(_disk_pending_item *temp_item)
		{
			TAILQ_INSERT_TAIL(&disk_pending_head, temp_item, disk_queue_entry);
		};
	};

}

#endif /* _DISK_PENDING_POOL_H_ */
