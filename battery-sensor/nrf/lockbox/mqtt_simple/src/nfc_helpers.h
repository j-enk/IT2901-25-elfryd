#include <zephyr/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <st25r3911b_nfca.h>
#include <nfc/ndef/msg_parser.h>
#include <nfc/ndef/le_oob_rec_parser.h>
#include <nfc/t2t/parser.h>
#include <nfc/t4t/ndef_file.h>
#include <nfc/t4t/isodep.h>
#include <nfc/t4t/hl_procedure.h>
#include <nfc/ndef/ch_rec_parser.h>
#include <zephyr/sys/byteorder.h>
#include <stdio.h>

// Variable that holds current key status
extern bool key_present_state;

#define NFCA_BD 128
#define BITS_IN_BYTE 8
#define MAX_TLV_BLOCKS 10
#define MAX_NDEF_RECORDS 10
#define NFCA_T2T_BUFFER_SIZE 1024
#define NFCA_LAST_BIT_MASK 0x80
#define NFCA_FDT_ALIGN_84 84
#define NFCA_FDT_ALIGN_20 20

#define NFC_T2T_READ_CMD 0x30
#define NFC_T2T_READ_CMD_LEN 0x02

#define NFC_T4T_ISODEP_FSD 256
#define NFC_T4T_ISODEP_RX_DATA_MAX_SIZE 1024
#define NFC_T4T_APDU_MAX_SIZE 1024

#define NFC_NDEF_REC_PARSER_BUFF_SIZE 128

#define NFC_TX_DATA_LEN NFC_T4T_ISODEP_FSD
#define NFC_RX_DATA_LEN NFC_T4T_ISODEP_FSD

#define T2T_MAX_DATA_EXCHANGE 16
#define TAG_TYPE_2_DATA_AREA_MULTIPLICATOR 8
#define TAG_TYPE_2_DATA_AREA_SIZE_OFFSET (NFC_T2T_CC_BLOCK_OFFSET + 2)
#define TAG_TYPE_2_BLOCKS_PER_EXCHANGE (T2T_MAX_DATA_EXCHANGE / NFC_T2T_BLOCK_SIZE)

#define TRANSMIT_DELAY 3000
#define ALL_REQ_DELAY 2000

extern uint8_t tx_data[NFC_TX_DATA_LEN];
extern uint8_t rx_data[NFC_RX_DATA_LEN];

extern struct k_poll_event events[ST25R3911B_NFCA_EVENT_CNT];
extern struct k_work_delayable transmit_work;

extern struct st25r3911b_nfca_buf nfc_tx_buf;
extern const struct st25r3911b_nfca_buf nfc_rx_buf;

enum nfc_tag_type
{
    NFC_TAG_TYPE_UNSUPPORTED = 0,
    NFC_TAG_TYPE_T2T,
    NFC_TAG_TYPE_T4T
};

enum t2t_state
{
    T2T_IDLE,
    T2T_HEADER_READ,
    T2T_DATA_READ
};

struct t2t_tag
{
    enum t2t_state state;
    uint16_t data_bytes;
    uint8_t data[NFCA_T2T_BUFFER_SIZE];
};

struct t4t_tag
{
    uint8_t data[NFC_T4T_ISODEP_RX_DATA_MAX_SIZE];
    uint8_t ndef[MAX_TLV_BLOCKS][NFC_T4T_APDU_MAX_SIZE];
    uint8_t tlv_index;
};

struct key_status
{
    bool current_state;
    bool previous_state
};

extern enum nfc_tag_type tag_type;
extern struct t2t_tag t2t;
extern struct t4t_tag t4t;

extern const struct st25r3911b_nfca_cb cb;
extern const struct nfc_t4t_isodep_cb t4t_isodep_cb;
extern const struct nfc_t4t_hl_procedure_cb t4t_hl_procedure_cb;

void nfc_tag_detect(bool all_request);

int ftd_calculate(uint8_t *data, size_t len);

int nfc_t2t_read_block_cmd_make(uint8_t *tx_data, size_t tx_data_size, uint8_t block_num);

int t2t_header_read(void);

void ndef_le_oob_rec_analyze(const struct nfc_ndef_record_desc *le_oob_rec_desc);

void ndef_rec_analyze(const struct nfc_ndef_record_desc *ndef_rec_desc);

void ndef_data_analyze(const uint8_t *ndef_msg_buff, size_t nfc_data_len);

void t2t_data_read_complete(uint8_t *data);

int t2t_on_data_read(const uint8_t *data, size_t data_len, void (*t2t_read_complete)(uint8_t *));

int on_t2t_transfer_complete(const uint8_t *data, size_t len);

void transfer_handler(struct k_work *work);

void nfc_field_on(void);

void nfc_timeout(bool tag_sleep);

void nfc_field_off(void);

void tag_detected(const struct st25r3911b_nfca_sens_resp *sens_resp);

void anticollision_completed(const struct st25r3911b_nfca_tag_info *tag_info, int err);

void transfer_completed(const uint8_t *data, size_t len, int err);

void tag_sleep(void);

void t4t_isodep_selected(const struct nfc_t4t_isodep_tag *t4t_tag);

void t4t_isodep_error(int err);

void t4t_isodep_data_send(uint8_t *data, size_t data_len, uint32_t ftd);

void t4t_isodep_received(const uint8_t *data, size_t data_len);

void t4t_isodep_deselected(void);

void t4t_hl_selected(enum nfc_t4t_hl_procedure_select type);

void t4t_hl_cc_read(struct nfc_t4t_cc_file *cc);

void t4t_hl_ndef_read(uint16_t file_id, const uint8_t *data, size_t len);

void delay(uint32_t delay);

int setup_nfc(void);