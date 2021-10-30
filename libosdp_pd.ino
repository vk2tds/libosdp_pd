/*
 * Copyright (c) 2021-2021 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define OSDP_EXPORT
#define OSDP_NO_EXPORT
#define CONFIG_OSDP_PACKET_TRACE TRUE

//#include <iostream>
#include <unistd.h>
#include <osdp.hpp>




//HardwareSerial debugSerial = Serial; // (PA3, PA3);
HardwareSerial Serial1 (PA10, PA9); // (PA10, PA9); // D2, D8, Rx, Tx


#define RS485_PTT_PIN PB3 // D3




//keystore_file = "/tmp/pd_scbk"

//def store_scbk(key):
//    with open(keystore_file, "w") as f:
//        f.write(key.hex())
//
//def load_scbk():
//    key = bytes()
//    if os.path.exists(keystore_file):
//        with open(keystore_file, "r") as f:
//            key = bytes.fromhex(f.read())
//    return key/
//
//def handle_command(command):
//    print("PD received command: ", command)/
//
//    if command['command'] == osdp.CMD_KEYSET:
//        if command['type'] == 1:
//            store_scbk(command['data'])




int sample_pd_send_func(void *data, uint8_t *buf, int len)
{
  static int state = LOW;
  digitalWrite(LED_BUILTIN, state);
  state = !state;
  digitalWrite (RS485_PTT_PIN, HIGH);
  delayMicroseconds(200);
  Serial1.write (buf, len);
  Serial1.flush();
  Serial1.flush();
  ////delay(1);
  digitalWrite (RS485_PTT_PIN, LOW);
  //digitalWrite(LED_BUILTIN, LOW);  

  return len;
}

int sample_pd_recv_func(void *data, uint8_t *buf, int len)
{

  int index = 0;
  while (Serial1.available()){
    buf[index] = Serial1.read();
    //Serial.println (buf[index]);
    index++;
    if (index >= len)
      break;
    if (index > 4)
      break; // vk2tds hack to break up packets
  }
  return index;
  // how do we share this between two PD Contexts? Do we grab the data and make sure it is sent to both? Is it shared internally?

}

int pd_command_handler(void *self, struct osdp_cmd *cmd)
{
 //Serial.print ("CALLBACK: PD: CMD ");
  //Serial.println (cmd->id);
  //Serial.flush();
  
  switch (cmd->id){
    case OSDP_CMD_MFG:
      //Serial.println ("MFG");
      return -9; // We dont know any user defined commands
      break;
    case OSDP_CMD_KEYSET:
      if (cmd->keyset.type == 1){
        // store cmd->data;
      }
      break;
    case OSDP_CMD_OUTPUT:
      //cmd->output.output_no, // 0 = first output
      //cmd->output.control_code, // 1 = Off. 2 = On. See osdp.h for other values. e.g. Timed Values
      return 0;
      break;
    case OSDP_CMD_SENTINEL:
      // Outputs I think. 
      // Reply with REPLY_LSTATR
      // [0] = Tamper; [1] = Power; ???
      return 0;
      break;
    case OSDP_CMD_BUZZER:
      Serial.println("Buz!");
      return 0;
      break;
    // CMD_POLL will not come here I think as this is for things like readers etc. 

    
  }



  return 0;
}

osdp_pd_info_t info_pd = {
  .baud_rate = 115200, // Ignored
  .address = 100,
  .flags = OSDP_FLAG_INSTALL_MODE, // Was 0.  such as OSDP_FLAG_INSTALL_MODE etc. 
  .id = {
    .version = 2,
    .model = 154,
    .vendor_code = 31336,
    .serial_number = 0x01020303,
    .firmware_version = 0x0A0B0C0C,
  },
  .cap = (struct osdp_pd_cap []) {
    {
      .function_code = OSDP_PD_CAP_READER_LED_CONTROL,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_CARD_DATA_FORMAT,
      .compliance_level = 2,
      .num_items = 1
    },
    //{
    //  .function_code = OSDP_PD_CAP_READER_AUDIBLE_OUTPUT,
    //  .compliance_level = 1,
    //  .num_items = 1
    //},
    {
      .function_code = OSDP_PD_CAP_OUTPUT_CONTROL,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_READERS,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_COMMUNICATION_SECURITY,
      .compliance_level = 1, // Try 1 later
      // more doc/libosdp/secure-channel.rst
      .num_items = 0
    },
//    {
//      .function_code = OSDP_PD_CAP_CHECK_CHARACTER_SUPPORT,
//      .compliance_level = 1, 
//      .num_items = 0
//    },
//    {
//      .function_code = OSDP_PD_CAP_CONTACT_STATUS_MONITORING, // MAY NOT BE IMPLEMENTED...
//      .compliance_level = 1,
//      .num_items = 1
//    },
    { static_cast<uint8_t>(-1), 0, 0 } /* Sentinel */
  },
  .channel = {
    .data = nullptr, // Ideally put a reference here to the rs485Serial
    .id = 0, // Not sure what this does
    .recv = sample_pd_recv_func,
    .send = sample_pd_send_func,
    .flush = nullptr // We need to flush before PTT stops so ignore this. 
  },
  .scbk = nullptr,
};




  OSDP::PeripheralDevice pd;


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode (RS485_PTT_PIN, OUTPUT);
  digitalWrite (RS485_PTT_PIN, LOW);
  
  Serial1.begin (115200);
  Serial.begin (115200);

  randomSeed(analogRead(0));

  Serial.println ("Welcomne to the Thunderdome...");
  Serial.flush();
  //Serial1.println ("Welcome to the other Thunderdome...");
  pd.logger_init(OSDP_LOG_MAX_LEVEL, printf);

  // info_pd.scbk = load from somewhere...

  info_pd.id.serial_number = random (1000000,9999999);
  info_pd.id.model = random (100,199);
  info_pd.id.version = random (100,199);
  info_pd.id.vendor_code = random (1000000,9999999);
  info_pd.id.firmware_version = random (1000000,9999999);

  info_pd.address = 1; //random (5,16);
  pd.setup(&info_pd);
  pd.set_command_callback(pd_command_handler);

}

void loop() {
    pd.refresh();

    if (random(1000000) == 1){
      osdp_event oet = {
        .type = OSDP_EVENT_CARDREAD,
        .cardread = {
          .reader_no = 1, // not used by lib
          .length = 8,
          //.data = 'ABCD1234',
        },
      };
      oet.cardread.data[0] = 'A';
      oet.cardread.data[0] = 'B';
      oet.cardread.data[0] = 'C';
      oet.cardread.data[0] = 'D';
      oet.cardread.data[0] = '1';
      oet.cardread.data[0] = '2';
      oet.cardread.data[0] = '3';
      oet.cardread.data[0] = '4';
      
       pd.notify_event (&oet);

      
    }

    //if (WIEGAND){
    //  }

    // your application code.
    //delay(1); // 1000 mSec

}
