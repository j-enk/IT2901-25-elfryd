#include <app_event_manager.h>

struct nfc_event
{
    struct app_event_header header;
    bool is_present;
};

APP_EVENT_TYPE_DECLARE(nfc_event);

void create_submit_nfc_event(bool is_present);
