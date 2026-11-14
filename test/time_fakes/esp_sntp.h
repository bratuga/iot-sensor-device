#pragma once
#include <sys/time.h>
void sntp_set_time_sync_notification_cb(void(*)(timeval*));
