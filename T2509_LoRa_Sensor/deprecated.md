
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
