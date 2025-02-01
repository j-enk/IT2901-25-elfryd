#include "nfc_event.h"

static void log_nfc_event(const struct app_event_header *aeh)
{
    const struct nfc_event *event = cast_nfc_event(aeh);

    APP_EVENT_MANAGER_LOG(aeh, "New NFC event, tag is_present=%d\n", event->is_present);
}

APP_EVENT_TYPE_DEFINE(
    nfc_event,
    log_nfc_event,
    NULL,
    APP_EVENT_FLAGS_CREATE(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE));

void create_submit_nfc_event(bool is_present)
{
    struct nfc_event *event = new_nfc_event();

    event->is_present = is_present;

    APP_EVENT_SUBMIT(event);
}