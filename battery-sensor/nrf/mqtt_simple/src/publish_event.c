#include "publish_event.h"
#include "mqtt_helpers.h"
#include "lock_detect_event.h"
#include "nfc_event.h"
#include <stdio.h>

static void log_publish_event(const struct app_event_header *aeh)
{
    const struct publish_event *event = cast_publish_event(aeh);

    APP_EVENT_MANAGER_LOG(aeh, "data=%s", event->data);
}

static bool event_handler(const struct app_event_header *aeh)
{
    
    if (is_lock_detect_event(aeh))
    {
        const struct lock_detect_event *event = cast_lock_detect_event(aeh);

        char data[50];
        sprintf(data, "%d?LOCK_EVENT=%d", CONFIG_LOCKBOX_ID,event->is_pressed);
        printk("Lock event received: %d\n", event->is_pressed);

        int ret = data_publish(
            &client,
            MQTT_QOS_1_AT_LEAST_ONCE,
            data,
            strlen(data));
        
        if (ret == 0)
        {
            printk("Publishing data succeeded");
        }
        else
        {
            printk("Publishing data failed");
        }
        return true;
    }


    if (is_nfc_event(aeh))
    {
        const struct nfc_event *event = cast_nfc_event(aeh);

        char data[50];
        sprintf(data, "%d?KEY_EVENT=%d", CONFIG_LOCKBOX_ID,event->is_present);
        printk("NFC event received: %d\n", event->is_present);
        int ret = data_publish(
            &client,
            MQTT_QOS_1_AT_LEAST_ONCE,
            data,
            strlen(data));

        if (ret == 0)
        {
            printk("Publishing data succeeded");
        }
        else
        {
            printk("Publishing data failed");
        }

        return true;
    }
    return true;
}


APP_EVENT_TYPE_DEFINE(
    publish_event,
    log_publish_event,
    NULL,
    APP_EVENT_FLAGS_CREATE(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE));

APP_EVENT_LISTENER(publish_event, event_handler);
APP_EVENT_SUBSCRIBE(publish_event, nfc_event);
APP_EVENT_SUBSCRIBE(publish_event, lock_detect_event);