#include <SoftwareSerial.h>
uint8_t ID;
#define RX_PIN 8
#define TX_PIN 9
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIME_ms 10


#define NUMBER_OF_HOLDING_REGISTERS 0 // FC:3
#define NUMBER_OF_INPUT_REGISTERS 10 //FC:4
SoftwareSerial mySerial(RX_PIN, TX_PIN);//Rx,Tx,

uint16_t holding_registers[NUMBER_OF_HOLDING_REGISTERS];
uint16_t input_registers[NUMBER_OF_INPUT_REGISTERS];

uint8_t B[8];//received bytes buffer


void configure_slave(uint8_t id) {
  ID = id;
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT); //Probably also configured by SoftwareSerial library.
  pinMode(TX_PIN, OUTPUT);//Probably also configured by SoftwareSerial library.
  mySerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
}


void slave_operate() {
  if (mySerial.available() >= 1)delay(WAIT_RESPONSE_TIME_ms);
  else return;


  uint8_t number_of_bytes_received = mySerial.available();
  if (number_of_bytes_received == 8) {//write or read request
    for (int i = 0; i < 8; i++) {
      B[i] = mySerial.read();
    }
  } else {
    while (mySerial.available())mySerial.read();
    return;
  }
  //---------------ERROR CHECK
  //1-ID CHECK
  if (B[0] != ID) {
    while (mySerial.available())mySerial.read();
    return;
  }
  //2-CRC CHECK
  uint16_t received_CRC = (((uint16_t)B[7]) << 8) + B[6];
  uint16_t expected_CRC = generate_CRC_16_bit(6, B[0], B[1], B[2], B[3], B[4], B[5]);

  if (received_CRC != expected_CRC) {
    while (mySerial.available())mySerial.read();
    slave_write(3, ID, 128, 7, 0, 0, 0);
    return;
  }

  //------------------

  if (B[1] == 3) {//read holding registers
    //QUERY: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-NUMBER_OF_REG(SIG), 5-NUMBER_OF_REG(LST), 6-CRC(LST), 7-CRC(SIG)
    uint16_t holding_register_addres = ((uint16_t)B[2] << 8) + B[3];

    if (holding_register_addres >= 0 && holding_register_addres < NUMBER_OF_HOLDING_REGISTERS) {

           
      uint8_t val_sig = holding_registers[holding_register_addres] >> 8;
      uint8_t val_lst = holding_registers[holding_register_addres] % 256;
      //RESPONSE: 0-ID, 1-FUNC_CODE, 2-BYTE_COUNT, 3-REG_VAL(SIG), 4-REG_VAL(LST), 5-CRC(LST), 6-CRC(SIG);
      slave_write(5, B[0], B[1], 2, val_sig, val_lst, 0);
    } else slave_write(3, ID, 128, 7, 0, 0, 0);//holding register address is wrong
  }

  else if (B[1] == 4) { //read input registers
    //QUERY: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-NUMBER_OF_REG(SIG), 5-NUMBER_OF_REG(LST), 6-CRC(LST), 7-CRC(SIG)
    uint16_t input_register_addres = ((uint16_t)B[2] << 8) + B[3];

    if (input_register_addres >= 0 && input_register_addres < NUMBER_OF_INPUT_REGISTERS) {
      uint8_t val_sig = input_registers[input_register_addres] >> 8;
      uint8_t val_lst = input_registers[input_register_addres] % 256;
      //RESPONSE: 0-ID, 1-FUNC_CODE, 2-BYTE_COUNT, 3-REG_VAL(SIG), 4-REG_VAL(LST), 5-CRC(LST), 6-CRC(SIG);
      slave_write(5, B[0], B[1], 2, val_sig, val_lst, 0);
    } else slave_write(3, ID, 128, 7, 0, 0, 0);//!input register address is wrong
  }

  else if (B[1] == 6) { //write input registers
    //QUERY: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-REG_VALUE(SIG), 5-REG_VAL(LST), 6-CRC(LST), 7-CRC(SIG)
    uint16_t input_register_address = ((uint16_t)B[2] << 8) + B[3];

    if (input_register_address >= 0 && input_register_address < NUMBER_OF_INPUT_REGISTERS) {
      input_registers[input_register_address] = 0;
      input_registers[input_register_address] = (((uint16_t)B[4]) << 8) + B[5];
      //RESPONSE: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-REG_VALUE(SIG), 5-REG_VAL(LST), 6-CRC(LST), 7-CRC(SIG)
      slave_write(6, B[0], B[1], B[2], B[3], B[4], B[5]);
    } else slave_write(3, ID, 128, 7, 0, 0, 0);//!holding register address is wrong
  } else {
    slave_write(3, ID, 128, 7, 0, 0, 0);//! unknown function code
  }


}



void slave_write( uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t CRC = generate_CRC_16_bit(number_of_bytes, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;
  delay(WAIT_RESPONSE_TIME_ms);

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  mySerial.write(B_0);
  if (number_of_bytes >= 2 )  mySerial.write(B_1);
  if (number_of_bytes >= 3 )  mySerial.write(B_2);
  if (number_of_bytes >= 4 )  mySerial.write(B_3);
  if (number_of_bytes >= 5 )  mySerial.write(B_4);
  if (number_of_bytes >= 6 )  mySerial.write(B_5);
  mySerial.write(CRC_LEAST); //CRC (LST)
  mySerial.write(CRC_SIGNIFICANT); //CRC (SIG)
  digitalWrite(OUT_ENABLE_PIN, LOW);

}
//MAGICAL CRC_16 MODBUS code.
uint16_t generate_CRC_16_bit(uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  if (number_of_bytes >= 2 )  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  if (number_of_bytes >= 3 )  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  if (number_of_bytes >= 4 )  remainder = CRC_16_bit_for_1BYTE(B_3, remainder);
  if (number_of_bytes >= 5 )  remainder = CRC_16_bit_for_1BYTE(B_4, remainder);
  if (number_of_bytes >= 6 )  remainder = CRC_16_bit_for_1BYTE(B_5, remainder);
  return remainder;

}
uint16_t CRC_16_bit_for_1BYTE(uint16_t data, uint16_t last_data) {
  //if this is first data (i.e LAST_DATA==null), LAST_DATA= 65535 = FFFF
  uint16_t key = 40961; //1010 0000 0000 0001
  data = data ^ last_data;//XOR
  for (int i = 0; i < 8; i++) {
    boolean should_XOR = false;
    if (data % 2 == 1)should_XOR = true;
    data = data >> 1;
    if (should_XOR)data = data ^ key;
  }
  return data;
}
//-----------------------------------------
uint8_t get_number_of_holding_register(){
  return NUMBER_OF_HOLDING_REGISTERS;
}
void set_holding_register(uint16_t address, uint16_t val) {
  if (address >= 0 && address < NUMBER_OF_HOLDING_REGISTERS) {
    holding_registers[address] = val;
  }
}
uint16_t get_holding_register(uint16_t address) {
  if (address >= 0 && address < NUMBER_OF_HOLDING_REGISTERS) {
    return holding_registers[address];
  } else {
    return 0;
  }
}

uint8_t get_number_of_input_register(){
  return NUMBER_OF_INPUT_REGISTERS;
}

void set_input_register(uint16_t address, uint16_t val) {
  if (address >= 0 && address < NUMBER_OF_INPUT_REGISTERS) {
    input_registers[address] = val;
  }
}
uint16_t get_input_register(uint16_t address) {
  if (address >= 0 && address < NUMBER_OF_INPUT_REGISTERS) {
    return input_registers[address];
  } else {
    return 0;
  }
}
