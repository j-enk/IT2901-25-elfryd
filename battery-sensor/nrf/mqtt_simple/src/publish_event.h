#include <app_event_manager.h>
struct publish_event
{
    struct app_event_header header;
    char *data;
};

APP_EVENT_TYPE_DECLARE(publish_event);