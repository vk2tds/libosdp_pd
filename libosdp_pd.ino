/*
 * Copyright (c) 2021-2021 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define OSDP_EXPORT
#define OSDP_NO_EXPORT

//#include <iostream>
#include <unistd.h>
#include <osdp.hpp>

HardwareSerial &rs485Serial = Serial;
HardwareSerial &debugSerial = Serial;



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
  //(void)(data);
  (void)(buf);
  //PTT On
  // TODO: Should be data.write etc.
  rs485Serial.write (buf, len);
  rs485Serial.flush();
  //PTT Off

  return len;
}

int sample_pd_recv_func(void *data, uint8_t *buf, int len)
{
  (void)(data);
  (void)(buf);
  (void)(len);

  // Fill these

  return 0;
}

int pd_command_handler(void *self, struct osdp_cmd *cmd)
{
  (void)(self);

  Serial.print ("PD: CMD ");
  Serial.println (cmd->id);

  switch (cmd->id){
    CMD_KEYSET:
      if (cmd->keyset.type == 1){
        // store cmd->data;
      }
      break;
    CMD_OUTPUT:
      //cmd->output.output_no, // 0 = first output
      //cmd->output.control_code, // 1 = Off. 2 = On. See osdp.h for other values. e.g. Timed Values
      break;

    
  }



  return 0;
}

osdp_pd_info_t info_pd = {
  .baud_rate = 115200, // Ignored
  .address = 101,
  .flags = 0,
  .id = {
    .version = 1,
    .model = 153,
    .vendor_code = 31337,
    .serial_number = 0x01020304,
    .firmware_version = 0x0A0B0C0D,
  },
  .cap = (struct osdp_pd_cap []) {
    {
      .function_code = OSDP_PD_CAP_READER_LED_CONTROL,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_READER_AUDIBLE_OUTPUT,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_OUTPUT_CONTROL,
      .compliance_level = 1,
      .num_items = 1
    },
    {
      .function_code = OSDP_PD_CAP_CONTACT_STATUS_MONITORING, // MAY NOT BE IMPLEMENTED...
      .compliance_level = 1,
      .num_items = 1
    },
    { static_cast<uint8_t>(-1), 0, 0 } /* Sentinel */
  },
  .channel = {
    .data = nullptr, // Ideally put a reference here to the rs485Serial
    .id = 0,
    .recv = sample_pd_recv_func,
    .send = sample_pd_send_func,
    .flush = nullptr // We need to flush before PTT stops so ignore this. 
  },
  .scbk = nullptr,
};




  OSDP::PeripheralDevice pd;


void setup() {
  rs485Serial.begin (115200);
  debugSerial.begin (115200);
  pd.logger_init(OSDP_LOG_DEBUG, printf);

  // info_pd.scbk = load from somewhere...
  
  pd.setup(&info_pd);
  pd.set_command_callback(pd_command_handler);

}

void loop() {
    pd.refresh();

    //if (WIEGAND){
    //  ospd_event_type oet = {
    //    .type = OSDP_EVENT_CARDREAD,
    //    .cardread = {
    //      .reader_no = 1, \\ not used by lib
    //      .length = 8,
    //      .data = 'ABCD1234';
    //    };
    //   pd.notify_event (oet);
    //  }

    // your application code.
    delay(1); // 1000 mSec

}
