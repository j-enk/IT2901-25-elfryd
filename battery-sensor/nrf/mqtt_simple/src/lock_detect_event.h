#include <app_event_manager.h>

struct lock_detect_event 
{
    struct app_event_header header;
    bool is_pressed;
};

APP_EVENT_TYPE_DECLARE(lock_detect_event);

void create_submit_lock_detect_event(bool is_pressed);
