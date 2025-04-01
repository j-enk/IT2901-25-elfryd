#include "lock_detect_event.h"

static void log_lock_detect_event(const struct app_event_header *aeh)
{
    const struct lock_detect_event *event = cast_lock_detect_event(aeh);

    APP_EVENT_MANAGER_LOG(aeh, "New lock detect event, button is pressed=%d\n", event->is_pressed);
}

APP_EVENT_TYPE_DEFINE(
    lock_detect_event,
    log_lock_detect_event,
    NULL,
    APP_EVENT_FLAGS_CREATE(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE));

void create_submit_lock_detect_event(bool is_pressed)
{
    struct lock_detect_event *event = new_lock_detect_event();

    event->is_pressed = is_pressed;

    APP_EVENT_SUBMIT(event);
}