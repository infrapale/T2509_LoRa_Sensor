


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
