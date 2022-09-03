//SOFTWARE_SERIAL: 4,8,9
//RELAYS: 6,7,10,11,12,13
//BACKDOOR: 3
#define NUMBER_OF_RELAYS 6
#define BACKDOOR_PIN 3
const uint8_t RELAYS[] = {6,7,10,11,12,13};//13 for built-in LED


void setup() {
  Serial.begin(9600);  
  configure_slave(52);  
  for(uint8_t i = 0 ; i< NUMBER_OF_RELAYS;i++){
    pinMode(RELAYS[i],OUTPUT);
  }

  pinMode(BACKDOOR_PIN,INPUT);
}

void loop() {  
  slave_operate();
  operate_loop();
}

unsigned long last_time_main_operate = 0;

void operate_loop(){

  //update time
  if(millis()-last_time_main_operate>1000){
    last_time_main_operate = last_time_main_operate+1000 ;
    //her şeyi çıkar    
    for(uint8_t i=0;i<get_number_of_input_register();i++){
      uint16_t dummy = get_input_register(i);
      if(dummy!=0){
        dummy = dummy -1;
        set_input_register(i,dummy);       
      }     
    }
  }

  //control relays
  if(digitalRead(BACKDOOR_PIN) == 1){
    for(uint8_t i=0; i<NUMBER_OF_RELAYS; i++){   
      digitalWrite(RELAYS[i],HIGH);
      delay(500);
    }  
    return;
  }
  for(uint8_t i=0; i<NUMBER_OF_RELAYS; i++){    
    uint16_t dummy = get_input_register(i);
    if(dummy == 0){
      digitalWrite(RELAYS[i],LOW);
    }else{
      digitalWrite(RELAYS[i],HIGH);
    }
  }
}
