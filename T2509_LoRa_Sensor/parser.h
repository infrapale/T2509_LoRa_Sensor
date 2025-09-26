#ifndef __PARSER_H__
#define __PARSER_H__

#define CMD_TAG_LEN     5
#define CMD_MAX_VALUES  10


typedef enum
{
  CMD_UNDEFINED = 0,
  CMD_RADIO_SEND,
  CMD_RADIO_RECEIVE, 
  CMD_SET_POWER,
  CMD_RADIO_RESET,
  CMD_SET_SF,
  CMD_RADIO_REPLY,
  CMD_GET_RSSI,
  CMD_GET_ROLE,
  CMD_GET_MSG,
  CMD_GET_CNTR,
  CMD_SET_MODEM_CONF,
  CMD_NBR_OF
} cmd_et;

typedef enum
{
  RADIO_UNDEFINED = 0,
  RADIO_LORA_433  = 1,
  RADIO_LORA_868  = 2,
  RADIO_RFM69_433 = 3,
  RADIO_RFM69_868 = 4,
} radio_et;


typedef enum
{
  STATUS_UNDEFINED = 0,
  STATUS_AVAILABLE,
  STATUS_OK_FOR_ME,
  STATUS_INCORRECT_FRAME,
} msg_status_et;




typedef struct
{
    char  tag[CMD_TAG_LEN];
} cmd_st;


typedef struct
{
  char      tag[CMD_TAG_LEN];
  cmd_et    tag_indx;
  uint8_t   nbr_values;
  int16_t   value[CMD_MAX_VALUES];
} msg_data_st;


typedef struct
{
    uint8_t tindx;
    //msg_st rx;
    //msg_st tx;
} parser_ctrl_st;

void parser_initialize(void);



#endif
