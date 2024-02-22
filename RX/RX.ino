/* CONSTANTS */
const int RED_LED_PIN = PE5; // Pin 3 on arduino 
const int BLUE_LED_PIN = PE4; // Pin 2 on arduino 
const char SLAVE_ID = '1'; //identification number (1-9) of THIS slave
const uint8_t INCOMING_MESSAGE_LENGTH = 4;
const char COMMAND_LIST[3] = {'U', 'F', 'G'}; //list of recognizable commands

String incoming_message;
unsigned long StartTime;
unsigned long EndTime;
//String incoming_message = "U551"; //FOR TEST PURPOSES ONLY
//String incoming_message = "F461"; //FOR TEST PURPOSES ONLY
//String incoming_message = "G471"; //FOR TEST PURPOSES ONLY

//MESSAGE FUNCTIONS
void receive(); // receives the preset data
bool recognizeMessage(String message); // checks whether received message can be recognized
bool checkSpacing(String message); // checks proper spacing in the message
bool checkCommands(String message); //checks whether the message includes a recognizable command
bool checkSum(String message); // checksum checker

//COMMAND FUNCTIONS
void commandHandler(String message); //calls the function corresponding to the received command
void commandU(); //perfroms command U
void commandF(); //perfroms command F
void commandG(); //perfroms command G

//TIMER
const uint16_t t1_comp = 16000;
const uint16_t t1_initial = 0; //16000; 31250
//uint8_t timer_counter = 0;

void timerInit();
void disableTimer();
void enableTimer();
void clockInit();

void setup() {
  Serial.begin(115200, SERIAL_8N2);
  Serial1.end(); //turn off TX by default
  Serial2.begin(1200, SERIAL_8N2);
  DDRE |= (1 << RED_LED_PIN);
  DDRE |= (1 << BLUE_LED_PIN);
  sei();
  timerInit();
  clockInit();
  Serial.println("Receiver is ready");

}


/*------------------------FUNCTIONAL PART OF THE CODE---------------------------*/
void loop() {

  if(Serial2.available() > 0){
    receive();
  }
  //commandHandler(incoming_message);
  
  /*
  if (recognizeMessage(incoming_message)){
    Serial.println("[SUCCESS]: Received message is recognized");

    if(isTalkToMe(incoming_message)){
      Serial.println("[SUCCESS]: Master is talking to me!");
      Serial.println("[INFO]: Received command: ");
      Serial.println(incoming_message);
      commandHandler(incoming_message);
    }
    else{
      Serial.println("[WARNING]: Master is not talking to me!");
      Serial.println("[INFO]: Received command: ");
      Serial.println(incoming_message);
    }

  }
  else{
    Serial.println("[ERROR]: Received message is not recognized");
    Serial.println("[INFO]: Received command: ");
    Serial.println(incoming_message);
  }
  incoming_message = "";
  delay(2000);
  */
}

bool recognizeMessage(String message){
  return (checkCommand(message) && checkSum(message)) ? 1 : 0;
}

bool checkCommand(String message){
  bool flag = 0;
  for(uint8_t i = 0; i < sizeof(COMMAND_LIST)/sizeof(COMMAND_LIST[0]); i++) {
    if (message.charAt(0) == COMMAND_LIST[i])
      flag = 1;
  }
  return flag;
}

bool checkSum(String message){
 uint8_t hex_characters = ((message.charAt(1) - '0' ) << 4) + message.charAt(2)- '0';
 uint8_t command_character = message.charAt(0);
 return (hex_characters == command_character) ? 1 : 0;
}

bool isTalkToMe(String message){
  return (message.charAt(INCOMING_MESSAGE_LENGTH-1) == SLAVE_ID) ? 1 : 0;
}

void commandHandler(String message){  
  if (message.charAt(0) == 'U') commandU();
  //if (message.charAt(0) == 'F') commandF();
  //if (message.charAt(0) == 'G') commandG();
}

void commandU(){ // FUNCTION INSTANTLY RESPONDS TO MASTER AND TOGGLES LED AFTER
  static int led_value = 0;
  Serial2.end();
  Serial1.begin(1200, SERIAL_8N2);
  //for(int i = 0; i < 1; i++){
  Serial1.write('\0');
  Serial1.write("<U>");
  //}
  led_value = 1 - led_value; 
  digitalWrite(BLUE_LED_PIN, led_value);

  Serial2.begin(1200, SERIAL_8N2);
  Serial1.end();
  //Serial.println(StartTime);
  //Serial.println(EndTime);
  //Serial.println("Receive time: ");
  //Serial.println(StartTime-EndTime);
}

void commandF(){ //FUNCTION TOGGLES DIGITAL OUTPUT AT PIN 2
  static int led_value = 0;
  Serial.println("DOING COMMAND F!");
  Serial.println("Toggling LED!");
  led_value = 1 - led_value;
  digitalWrite(BLUE_LED_PIN, led_value);
}

void commandG(){
  Serial.println("DOING COMMAND G!");
}

void timerInit(){
  //Reset Timer1 Control Reg A
  TCCR1A = 0;
  TCCR1B &= ~0xDF;
  TCCR1B |= 0x08; 

  //Set the prescaler of 1
  TCCR1B |= 0x01; //

  // Reset Timer1 and set compare value
  TCNT1 = t1_initial;
  OCR1A = t1_comp;

  //Enable Timer1 compare interrupt 
  TIMSK1 = (1 << OCIE1A);
}

void disableTimer(){
  TCCR1B &= ~0x01;
}
void enableTimer(){
  TCCR1B |= 0x01; // This enables timer or clock source 
}

void clockInit(){
  //Reset Timer1 Control Reg A
  TCCR3A = 0;
  TCCR3B &= ~0xDF;
  TCCR3B |= 0x08; 

  //Set the prescaler of 1
  TCCR3B |= 0x01; //

  // Reset Timer1 and set compare value
  TCNT3 = 0;
  OCR3A = 197;

  //Enable Timer1 compare interrupt 
  TIMSK3 = (1 << OCIE3A);
  TCCR3B |= 0x01; // This enables timer or clock source 
}

void disableClock(){
  TCCR2B &= ~0x01;
}
void enableClock(){
  TCCR2B |= 0x01; // This enables timer or clock source 
}

ISR(TIMER1_COMPA_vect){
  static uint8_t timer_counter = 0;
  //PORTE ^= (1 << PE5);
  if(timer_counter >= 100-1){
    //commandHandler(incoming_message);
      Serial2.end();
      Serial1.begin(1200, SERIAL_8N2);
      //for(int i = 0; i < 1; i++){
      Serial1.write("000000000000000000");
      Serial1.write("<U>");
      //}
      Serial2.begin(1200, SERIAL_8N2);
      Serial1.end();


    incoming_message = "";
    timer_counter = 0;
    disableTimer();
    Serial.println("100ms!");
  }else{
    timer_counter++;
  }
}

ISR(TIMER3_COMPA_vect){
  PORTE ^= (1 << BLUE_LED_PIN);
}

void receive(){

  String trashData = Serial2.readStringUntil('<');
  enableTimer();
  incoming_message = Serial2.readStringUntil('>');
 
  Serial.println(incoming_message);
}
