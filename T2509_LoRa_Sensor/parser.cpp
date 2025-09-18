/******************************************************************************
*******************************************************************************

Send Radio Mesage:
<RSND;from;target;radio;pwr;sf;rnbr;bnbr>\n
<RSND;1;2;3;14;12;222;210>

Set Power Level:
<SPWR;pwr>
<SPWR;20>


******************************************************************************/

#include <Arduino.h>
#include <stdint.h> 
#include "main.h"
#include "parser.h"
#include "atask.h"
#include "rfm.h"
//#define UART_0 Serial1

parser_ctrl_st  parser_ctrl;
extern rfm_ctrl_st rfm_ctrl;
char            send_msg[RH_RF95_MAX_MESSAGE_LEN];

void parser_task(void);

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st parser_task_handle    =  {"Parser Task    ", 100,     0,     0,  255,    0,  1,  parser_task };

cmd_st commands[CMD_NBR_OF] = 
{
  [CMD_UNDEFINED]       = {"UNDF"},
  [CMD_RADIO_SEND]      = {"RSND"},
  [CMD_RADIO_RECEIVE]   = {"RREC"},
  [CMD_SET_POWER]       = {"SPWR"},
  [CMD_RADIO_RESET]     = {"RRST"},
  [CMD_SET_SF]          = {"S_SF"},
  [CMD_RADIO_REPLY]     = {"RREP"},
  [CMD_GET_RSSI]        = {"RSSI"},
  [CMD_GET_ROLE]        = {"ROLE"},
  [CMD_GET_MSG]         = {"GMSG"},
  [CMD_GET_CNTR]        = {"CNTR"},
  [CMD_SET_MODEM_CONF]  = {"SMCF"},
};

msg_data_st msg_data = {0};
msg_data_st rply_data = {0};


void parser_initialize(void)
{
    parser_ctrl.tindx =  atask_add_new(&parser_task_handle);
    rfm_ctrl.rx_msg.avail = false;
}

msg_status_et read_uart(String *Str)
{
    msg_status_et status = STATUS_UNDEFINED;
    if (Serial1.available())
    {
        //Serial.println("rx is available");
        *Str = Serial1.readStringUntil('\n');
        if (Str->length()> 0)
        {
            rfm_ctrl.rx_msg.avail = true;
            //rx_msg.str.remove(rx_msg.str.length()-1);
            //Serial1.println(rx_msg.str);
            status = STATUS_AVAILABLE;
        }
    } 
    return status;
}


msg_status_et parse_frame(String *Str)
{
    msg_status_et status = STATUS_UNDEFINED;
    //rfm_send_msg_st *rx_msg = &send_msg; 
    bool do_continue = true;
    uint8_t len;
    Str->trim();
    Serial.print(*Str);
    len = Str->length();
    Serial.println(len);
    if ((Str->charAt(0) != '<') || 
        (Str->charAt(len-1) != '>'))  do_continue = false;
    if (do_continue)
    {   
        status = STATUS_OK_FOR_ME;
        #ifdef DEBUG_PRINT
        Serial.print("Buffer frame is OK\n");
        #endif
    }
    else status = STATUS_INCORRECT_FRAME;
    return status;
}

//char msg_str[80];
void parser_build_msg_from_fields(char *msg, msg_st *msg_data)
{
    sprintf(msg,"<%s;%d;%d;%d;%d;%d;%d;%d;%d>",
        msg_data->field.cmnd,
        msg_data->field.from,
        msg_data->field.target,
        msg_data->field.radio,
        msg_data->field.power,
        msg_data->field.rssi,
        msg_data->field.sf,
        msg_data->field.remote_nbr,
        msg_data->field.base_nbr
    );
}

void parser_rd_msg_values(msg_data_st *msg_data, String *StrP)
{
    uint8_t indx1 = 1;
    uint8_t indx2 = StrP->indexOf(';');
    bool    do_continue = true;
    uint8_t end_pos = StrP->length();

    StrP->substring(indx1,indx2).toCharArray( msg_data->tag , CMD_TAG_LEN);
    msg_data->tag_indx = CMD_NBR_OF;
    for (uint8_t i = 0; i < CMD_NBR_OF; i++)
    {
        if(strcmp(msg_data->tag, commands[i].tag) == 0) 
        {
            msg_data->tag_indx = (cmd_et)i;
            break;
        }
    }

    indx1 = indx2+1;
    msg_data->nbr_values = 0;
    if((indx1 < end_pos) && (msg_data->tag_indx < CMD_NBR_OF)) do_continue = true;
    else do_continue = false;

    while(do_continue)
    {
        indx2 = StrP->indexOf(';',indx1+1);
        msg_data->value[msg_data->nbr_values] = StrP->substring(indx1,indx2).toInt();
        indx1 = indx2+1;
        msg_data->nbr_values++;
        if ((indx2 >= end_pos) || (msg_data->nbr_values >= CMD_MAX_VALUES)) do_continue = false;
    }
}


void parser_print_data(msg_data_st *msg_data)
{ 
    Serial.printf("Tag=%s: %d,len = %d: ", msg_data->tag, msg_data->tag_indx, msg_data->nbr_values);
    for (uint8_t i = 0; i < CMD_MAX_VALUES; i++)
    {
        Serial.printf("%d, ",msg_data->value[i]);
    }
    Serial.println("");
}


void parser_radio_reply(uint8_t *msg , int rssi)
{
    String RplyStr;
    msg_status_et rply_status = STATUS_UNDEFINED;

    RplyStr = (char*)msg;
    rply_status = parse_frame(&RplyStr);
    Serial.print("Parsing radio reply:");
    Serial.print(" Status= "); Serial.print(rply_status); 
    Serial.print(" Message= "); Serial.println(RplyStr);
    parser_rd_msg_values(&rply_data, &RplyStr);
    parser_print_data(&rply_data);

    rfm_ctrl.rx_msg.field.from           = rply_data.value[0];
    rfm_ctrl.rx_msg.field.target         = rply_data.value[1];
    rfm_ctrl.rx_msg.field.radio          = rply_data.value[2];
    rfm_ctrl.rx_msg.field.power          = rply_data.value[3];
    rfm_ctrl.rx_msg.field.rssi           = rply_data.value[4];
    rfm_ctrl.rx_msg.field.sf             = rply_data.value[5];
    rfm_ctrl.rx_msg.field.remote_nbr     = rply_data.value[6];
    rfm_ctrl.rx_msg.field.base_nbr       = rply_data.value[7];
    
    rfm_ctrl.rx_msg.avail    = true;
    rfm_ctrl.rx_msg.status   = STATUS_AVAILABLE;
}

void parser_get_reply(void)
{
    if(rfm_ctrl.rx_msg.avail)
    {
        Serial1.printf("<REPL;%d;%d;%d;%d;%d;%d;%d;%d>\n",
            rfm_ctrl.rx_msg.field.from,
            rfm_ctrl.rx_msg.field.start,
            rfm_ctrl.rx_msg.field.radio,
            rfm_ctrl.rx_msg.field.power,
            rfm_ctrl.rx_msg.field.rssi,
            rfm_ctrl.rx_msg.field.sf,
            rfm_ctrl.rx_msg.field.remote_nbr,
            rfm_ctrl.rx_msg.field.base_nbr);
        rfm_ctrl.rx_msg.avail = false;
    }
    else
    {
        Serial1.printf("<FAIL;%d>\n",0);
    }

}

// Get own RSSI
void parser_get_rssi(void)
{
    if(rfm_ctrl.rx_msg.avail)
    {
        Serial1.printf("<RSSI;%d>\n",rfm_ctrl.rssi);
        Serial.printf("<RSSI;%d>\n",rfm_ctrl.rssi);
    }
    else
    {
        Serial1.printf("<FAIL;%d>\n",0);
    }
}

// Get own Role
void parser_get_role(void)
{
    if(rfm_ctrl.node_role == NODE_ROLE_CLIENT)
    {
        Serial1.printf("<ROLE;%d>\n",rfm_ctrl.node_role);
        Serial.printf("<ROLE;%d>\n",rfm_ctrl.node_role);
    }
    rfm_ctrl.sub_task.get_role = true;
}

void parser_get_msg(void)
{
    rfm_ctrl.sub_task.get_msg = true;
}

void parser_get_cntr(void)
{
    if(rfm_ctrl.node_role == NODE_ROLE_CLIENT)
        Serial1.printf("<CNTR;%d>\n",rfm_ctrl.client_cntr);
    else
        Serial1.printf("<CNTR;%d>\n",rfm_ctrl.server_cntr);
}


void parser_exec_command(msg_st *msg, msg_data_st *msg_data)
{

    // Serial.printf("parser_exec_command: %d\n",msg_data->tag_indx);
    if (msg_data->tag_indx < CMD_NBR_OF)
    {
        switch(msg_data->tag_indx)
        {
            case CMD_RADIO_SEND:
                strncpy(msg->field.cmnd, msg_data->tag,CMD_TAG_LEN );
                msg->field.from         = msg_data->value[0];
                msg->field.target       = msg_data->value[1];
                msg->field.radio        = msg_data->value[2];
                msg->field.power        = msg_data->value[3];
                msg->field.rssi         = msg_data->value[4];
                msg->field.sf           = msg_data->value[5];
                msg->field.remote_nbr   = msg_data->value[6];
                msg->field.base_nbr     = msg_data->value[7];
                memset(send_msg,0x00,RH_RF95_MAX_MESSAGE_LEN);
                parser_build_msg_from_fields(send_msg,msg);
                Serial.println(send_msg);
                rfm_set_power(msg->field.power);
                rfm_send_str(send_msg);
                break;
            case CMD_RADIO_RECEIVE:
                break;
            case CMD_SET_POWER:
                Serial.printf("Set Power: %d",msg_data->value[0]);
                rfm_set_power(msg_data->value[0]);
                break;
            case CMD_RADIO_RESET:
                rfm_reset();
                break;
            case CMD_SET_SF:
                rfm_set_sf(msg_data->value[0]);
                break;
            case CMD_RADIO_REPLY:
                parser_get_reply();
                break;
            case CMD_GET_RSSI:
                parser_get_rssi(); 
                break;   
            case CMD_GET_ROLE:
                parser_get_role(); 
                break;   
            case CMD_GET_MSG:
                parser_get_msg(); 
                break;   
            case CMD_GET_CNTR:
                parser_get_cntr(); 
                break;   
            case CMD_SET_MODEM_CONF:
                Serial.printf("Set Modem Conf: %d", msg_data->value[0]);
                rfm_set_modem_conf(msg_data->value[0]);
                break;
        }

    }
}

void parser_task(void)
{
    static String  RxStr;
    //char    test_msg[80];
    static msg_status_et status = STATUS_UNDEFINED;
    // Serial.print("P");
    switch(parser_task_handle.state)
    {
        case 0:
            RxStr.reserve(80);
            parser_task_handle.state = 10;
            break;
        case 10:
            status = read_uart(&RxStr);
            if (status == STATUS_AVAILABLE)
            {
                parser_task_handle.state = 20;
            }
            break;
        case 20:
            status = parse_frame(&RxStr);
            Serial.printf("status= %d\n", status);

            parser_rd_msg_values(&msg_data, &RxStr);
            parser_print_data(&msg_data);
            parser_exec_command(&rfm_ctrl.tx_msg, &msg_data);
            Serial.println(rfm_ctrl.rx_msg.field.cmnd);
            Serial.println(rfm_ctrl.rx_msg.field.base_nbr);

            //parser_build_msg_from_fields(test_msg,&rx_msg);
            //Serial.println(test_msg);
            //rfm_ctrl.rx_msg.avail = false;
            rfm_ctrl.rx_msg.status = STATUS_UNDEFINED;
            parser_task_handle.state = 10;
            break;
        case 30:
            break;
    }
}
