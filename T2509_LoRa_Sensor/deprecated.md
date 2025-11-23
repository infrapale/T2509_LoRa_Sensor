
// typedef struct
// {
//     bool get_rssi;
//     bool get_role;
//     bool get_msg;
// } sub_task_st;


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

msg_status_et parse_frame(String *Str);



#endif



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
                //parser_get_reply();
                break;
            case CMD_GET_RSSI:
                //parser_get_rssi(); 
                break;   
            case CMD_GET_ROLE:
                parser_get_role(); 
                break;   
            case CMD_GET_MSG:
                parser_get_msg(); 
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



typedef enum
{
  TX_MSG_SEND_REMOTE_TO_BASE,
  TX_MSG_RESET_CNTR,
  TX_MSG_,
  TX_MSG_NBR_OF
} tx_msg_et;


typedef enum
{
  RX_MSG_SEND_REMOTE_TO_BASE,
  RX_MSG_RESET_CNTR,
  RX_MSG_,
  RX_MSG_NBR_OF
} rx_msg_et;


void parser_get_reply(void);

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



void parser_radio_reply(uint8_t *msg , int rssi);

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


#ifndef __UARTX_H__
#define __UARTX_H__
// #include "Arduino.h"


// #define UART_MAX_BLOCK_LEN  8

// void uartx_initialize(void);

// void uartx0_read_uart(void);


#endif

// #include "main.h"
// #include "uartx.h"
// #include "io.h"
// #include "atask.h"

// #define UARTX0 Serial1

// typedef enum
// {
//   FIELD_START   = 0,
//   FIELD_COMMAND,
//   FIELD_FROM,
//   FIELD_TARGET,
//   FIELD_RADIO,
//   FIELD_POWER,
//   FIELD_SPREADING_FACTOR,
//   FIELD_REMOTE_NBR,
//   FIELD_BASE_NBR,
//   FILED_END,
//   FIELD_NBR_OF
// } field_et;

// typedef enum
// {
//   RADIO_UNDEFINED = 0,
//   RADIO_LORA_433  = 1,
//   RADIO_LORA_868  = 2,
//   RADIO_RFM69_433 = 3,
//   RADIO_RFM69_868 = 4,
// } radio_et;

// typedef enum
// {
//   RX_MSG_SEND_REMOTE_TO_BASE,
//   RX_MSG_RESET_CNTR,
//   RX_MSG_,
//   RX_MSG_NBR_OF
// } rx_msg_et;

// typedef enum
// {
//   TX_MSG_SEND_REMOTE_TO_BASE,
//   TX_MSG_RESET_CNTR,
//   TX_MSG_,
//   TX_MSG_NBR_OF
// } tx_msg_et;


// //uart_msg_st         uart;

// typedef struct
// {
//   char      start;
//   char      cmnd[5];
//   uint8_t   from;
//   uint8_t   target;
//   uint8_t   radio;
//   uint8_t   power;
//   uint8_t   sf;
//   uint16_t  remote_nbr;
//   uint16_t  base_nbr;
//   char      end;
// } field_st;



// typedef struct
// {
//   String  str;
//   field_st  field;
//   uint8_t rx_delim[FIELD_NBR_OF];
//   bool      avail;
// } rx_st;

// typedef struct
// {
//     uint8_t tindx;
//     rx_st rx;
// } uartx_st;

// void uartx0_task(void);

// //                                  123456789012345   ival  next  state  prev  cntr flag  call backup
// atask_st uartx0_task_handle    =  {"UART 0 Task    ", 1000,    0,     0,  255,    0,  1,  uartx0_task };


// uartx_st uartx0;

// void uartx_initialize(void)
// {
//     //uartx0.tindx =  atask_add_new(&uartx0_task_handle);
//     uartx0.rx.avail = false;
// }

// void uartx0_read(void)
// {
//     // if (UARTX0.available())
//     // {
//     //     Serial.println("rx is available");
//     //     uartx0.rx.str = UARTX0.readStringUntil('\n');
//     //     if (uartx0.rx.str.length()> 0)
//     //     {
//     //         uartx0.rx.avail = true;
//     //         //uart.rx.str.remove(uart.rx.str.length()-1);
//     //         Serial.println(uartx0.rx.str);
//     //     }
//     // } 

// }

// void uartx0_task(void)
// {
//     //uartx0_read();
// }


// uart_msg_st *uart_get_data_ptr(void)
// {
//     return &uart;
// }

// void uart_initialize(void)
// {
//     uart.rx.avail = false;
// }

// void uart_read_uart(void)
// {
//     if (SerialX.available())
//     {
//         io_led_flash(LED_INDX_BLUE,20);
//         uart.rx.str = SerialX.readStringUntil('\n');
//         if (uart.rx.str.length()> 0)
//         {
//             uart.rx.avail = true;
//             //uart.rx.str.remove(uart.rx.str.length()-1);
//         }
//         #ifdef DEBUG_PRINT
//         Serial.println("rx is available");
//         #endif        
//     } 

// }

// void uart_report_radio_msg(char *radio_receive_msg, int rssi)
// {
//     String msg_str = radio_receive_msg;
//     Serial.print(msg_str); Serial.print(" - ");
//     Serial.println(rssi);
// }

// void uart_parse_rx_frame(void)
// {
//     //rfm_send_msg_st *rx_msg = &send_msg; 
//     bool do_continue = true;
//     uint8_t len;
//     uart.rx.str.trim();
//     uart.rx.len = uart.rx.str.length();
//     if ((uart.rx.str.charAt(0) != '<') || 
//         (uart.rx.str.charAt(uart.rx.len-1) != '>'))  do_continue = false;
//     if (do_continue)
//     {   
//         uart.rx.status = STATUS_OK_FOR_ME;
//         uart.rx.avail = true;
//         #ifdef DEBUG_PRINT
//         Serial.print("Buffer frame is OK\n");
//         #endif
//     }
//     else uart.rx.status = STATUS_INCORRECT_FRAME;
// }

// void uart_build_node_from_rx_str(void)
// {
//     uint8_t indx1;
//     uint8_t indx2;
//     indx1 = 0;  //uart.rx.str.indexOf(':')
//     indx2 = uart.rx.str.indexOf(';');
//     uart.node.zone = uart.rx.str.substring(indx1,indx2);
//     indx1 = indx2+1;
//     indx2 = uart.rx.str.indexOf(';',indx1+1);
//     uart.node.name = uart.rx.str.substring(indx1,indx2);
//     indx1 = indx2+1;
//     indx2 = uart.rx.str.indexOf(';',indx1+1);
//     uart.node.value = uart.rx.str.substring(indx1,indx2);
//     indx1 = indx2+1;
//     indx2 = uart.rx.str.indexOf(';',indx1+1);
//     uart.node.remark = uart.rx.str.substring(indx1,indx2);
//     indx1 = indx2+1;
//     indx2 = uart.rx.str.indexOf(';',indx1+1);
    
// }







// void uart_build_raw_tx_str(void)
// {
//     rfm_receive_msg_st *receive_p = rfm_receive_get_data_ptr();
//     uart.tx.str += (char*) receive_p->radio_msg;
// }

// void uart_rx_send_rfm_from_raw(void)
// {
//     // String payload = uart.rx.str.substring(6,uart.rx.len - 1);
//     String payload = uart.rx.str;
//     payload.toCharArray(uart.rx.radio_msg, MAX_MESSAGE_LEN);
//     rfm_send_radiate_msg(uart.rx.radio_msg);
// }

// void uart_rx_send_rfm_from_node(void)
// {
//     uart.rx.str = uart.rx.str.substring(6,uart.rx.len - 1);
//     uart_build_node_from_rx_str();
//     rfm_send_msg_st *send_p = rfm_send_get_data_ptr();
//     json_convert_uart_node_to_json(send_p->radio_msg, &uart);
//     rfm_send_radiate_msg(send_p->radio_msg);
// }

// void uart_exec_cmnd(uart_cmd_et ucmd)
// {
//     uart_rx_send_rfm_from_raw();
// }


// void uart_print_rx_metadata(void)
// {

//     // Serial.print("Length      "); Serial.println(uart.rx.len);
//     // Serial.print("Avail       "); Serial.println(uart.rx.avail);
//     // Serial.print("Status      "); Serial.println(uart.rx.status);
//     // Serial.print("Module      "); Serial.println(uart.rx.module);
//     // Serial.print("Address     "); Serial.println(uart.rx.addr);
//     // Serial.print("Command     "); Serial.println(uart.rx.cmd);
//     // Serial.print("Format      "); Serial.println(uart.rx.format);
// }    




// void loop_client(void)
// {
//     //Serial.print("C"); Serial.print(rfm_task_handle.state);
//     switch(rfm_task_handle.state)
//     {
//         case 0:
//             rfm_task_handle.state = 5;
//             break;
//         case 5:    
//             if(rfm_ctrl.send_msg_len > 0) 
//             {
//                 rfm_ctrl.reply_status =  REPLY_UNDEFINED;
//                 rfm_task_handle.state = 10;
//                 io_blink(COLOR_BLUE, BLINK_FAST_BLINK);
//             }
//             break;
//         case 10:
//             rf95.send(rfm_ctrl.send_msg, rfm_ctrl.send_msg_len);
//             rf95.waitPacketSent();
//             rfm_ctrl.send_msg_len = 0;
//             if (rf95.waitAvailableTimeout(3000))
//             {
//                 rfm_ctrl.rec_msg_len = RH_RF95_MAX_MESSAGE_LEN-10;
//                 rfm_task_handle.state = 30;
//             }
//             else
//             {
//               rfm_task_handle.state = 40;
//               rfm_ctrl.reply_status =  REPLY_FAILED;
//               Serial1.println("*******REPLY_FAILED*******");
//             }
//             break;
//         case 30:
//             // Should be a reply message for us now   
//             if (rf95.recv(rfm_ctrl.rec_msg, &rfm_ctrl.rec_msg_len))
//             {
//                 rfm_ctrl.reply_status =  REPLY_RECEIVED;
//                 rfm_ctrl.rssi = rf95.lastRssi();
//                 if(rfm_ctrl.rec_msg_len < RH_RF95_MAX_MESSAGE_LEN-1 )
//                 {
//                     rfm_ctrl.rec_msg[rfm_ctrl.rec_msg_len] = 0x00;
//                     Serial1.println((char*)rfm_ctrl.rec_msg);
//                 }
//                 parser_radio_reply(rfm_ctrl.rec_msg, rfm_ctrl.rssi);
//                 //Serial1.print("got reply: ");
//                 //Serial.println((char*)rfm_ctrl.rec_msg);
//                 //Serial.print("RSSI: ");
//                 //Serial.println(rfm_ctrl.rssi, DEC);    
//                 rfm_task_handle.state = 50;  
//             }
//             else
//             {
//                 //Serial.println("recv failed");
//                 rfm_task_handle.state = 40;  
//                 rfm_ctrl.reply_status =  REPLY_FAILED;
//             }            
//             break;
//         case 40:
//             io_blink(COLOR_RED, BLINK_FAST_FLASH);           
//             Serial.println("No reply, is rf95_server running?");
//             rfm_task_handle.state = 50;  
//             break;
//         case 50:    
//             io_blink(COLOR_BLUE, BLINK_OFF); 
//             rfm_timeout = millis() + 1000;
//             rfm_task_handle.state = 60;  
//             break;
//         case 60:
//             if(millis() > rfm_timeout) rfm_task_handle.state = 100; 
//             break;
//         case 100:
//             io_blink(COLOR_RED, BLINK_OFF);
//             rfm_task_handle.state = 0; 
//             break;

//     }
// }

// void loop_server(void)
// {
//     //Serial.print("S"); Serial.print(rfm_task_handle.state);
//     char txt[64];
//     switch(rfm_task_handle.state)
//     {
//         case 0:
//             rfm_task_handle.state = 10;
//             rfm_ctrl.rec_msg_len = RH_RF95_MAX_MESSAGE_LEN-10;
//             break;
//         case 10:
//             if(rfm_ctrl.sub_task.get_role) 
//             {
//                 Serial1.printf("<ROLE;%d>\n",rfm_ctrl.node_role);
//                 rfm_ctrl.sub_task.get_role = false;
//             }
//             if(rfm_ctrl.sub_task.get_rssi) 
//             {
//                 Serial1.printf("<RSSI;%d>\n",rfm_ctrl.rssi);
//                 rfm_ctrl.sub_task.get_rssi = false;
//             }
//             if(rfm_ctrl.sub_task.get_msg) 
//             {
//                 if(rfm_ctrl.reply_status ==  REPLY_REQUEST)
//                 {
//                     //Serial1.println((char*)rfm_ctrl.rec_msg);

//                     // parser_radio_reply(uint8_t *msg , int rssi) // rfm_ctrl.rx_msg.field.from = rply_data.value[0];
//                     rfm_ctrl.rx_msg.field.rssi = rfm_ctrl.rssi;
//                     rfm_ctrl.rx_msg.field.base_nbr = rfm_ctrl.server_cntr;
//                     parser_get_reply(); // rfm_ctrl.rx_msg.field.from

//                     rfm_ctrl.reply_status =  REPLY_UNDEFINED;
//                 }
//                 else 
//                 {
//                     Serial1.printf("<FAIL;0>\n",rfm_ctrl.rssi);
//                 }
//                 rfm_ctrl.sub_task.get_msg = false;
//             }
//             rfm_task_handle.state = 15;
//             break;
//         case 15:
//             if (rf95.available())
//             {
//                 // Should be a message for us now   
//                 if (rf95.recv(rfm_ctrl.rec_msg, &rfm_ctrl.rec_msg_len))
//                 {
//                     rfm_ctrl.reply_status =  REPLY_REQUEST;
//                     rfm_ctrl.rssi = rf95.lastRssi();
//                     if(rfm_ctrl.rec_msg_len < RH_RF95_MAX_MESSAGE_LEN-1 )
//                     {
//                         rfm_ctrl.rec_msg[rfm_ctrl.rec_msg_len] = 0x00;
//                         //Serial1.println((char*)rfm_ctrl.rec_msg);
//                     }
//                     parser_radio_reply(rfm_ctrl.rec_msg, rfm_ctrl.rssi);
//                     rfm_ctrl.server_cntr++;


//                   rfm_ctrl.rssi = rf95.lastRssi(); 
//                   io_blink(COLOR_BLUE, BLINK_FAST_BLINK);
//                   Serial.print("got request: ");
//                   Serial.println((char*)rfm_ctrl.rec_msg);
//                   Serial.print("RSSI: ");
//                   Serial.println(rfm_ctrl.rssi, DEC);
//                   rfm_task_handle.state = 16;
//                   rfm_timeout = millis() + 100;
//                 }
//                 else 
//                 {
//                   io_blink(COLOR_RED, BLINK_FAST_FLASH); 
//                   rfm_task_handle.state = 100;
//                 }
//             }
//             break;
//         case 16:
//              if(millis() > rfm_timeout) rfm_task_handle.state = 20;
//             break;
//         case 20:
//             // Send a reply
//             sprintf(txt,"<RREP;%d;%d;%d;%d;%d;%d;%d;%d>",
//                 rfm_ctrl.rx_msg.field.from,
//                 rfm_ctrl.rx_msg.field.target,
//                 rfm_ctrl.node_role,
//                 rfm_ctrl.power,
//                 rfm_ctrl.rssi,
//                 rfm_ctrl.sf,
//                 rfm_ctrl.rx_msg.field.remote_nbr,
//                 rfm_ctrl.server_cntr
//                 );
//             //rfm_send_str("<RREP;1;2;3;14;-77;12;33;444>");
//             rfm_send_str(txt);
//             //rfm_ctrl.send_msg_len = strlen(txt); 
//             //memcpy(rfm_ctrl.send_msg, txt, rfm_ctrl.send_msg_len);
//             //memcpy(rfm_ctrl.send_msg, "<RREP;1;2;3;14;-77;12;33;444>",RH_RF95_MAX_MESSAGE_LEN);
//             rf95.send(rfm_ctrl.send_msg, rfm_ctrl.send_msg_len);
//             rf95.waitPacketSent();
//             Serial.println("Sent a reply");
//             rfm_task_handle.state = 100;
//             break;
//         case 50:
//             Serial.println("recv failed");
//             rfm_task_handle.state = 100;
//             break;
//         case 100:
//             rfm_timeout = millis() + 100;
//             rfm_task_handle.state = 110;
//             break;
//         case 110:
//             if(millis() > rfm_timeout) 
//             {
//                 io_blink(COLOR_BLUE, BLINK_OFF);
//                 io_blink(COLOR_RED, BLINK_OFF);
//                 rfm_task_handle.state = 10;
//             }    
//             break;

//     }
// }  
