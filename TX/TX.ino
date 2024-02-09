/*CSUN ULTRASONIC TRANSPONDER TEAM FALL 2023
Interrogator Main Code Version 1.0
Authors: Kenny Alas, Jose Gonzalez, Tymofii Manak, Daniel Beltran
*/

String data; // used in formating user input
String command; // holds command input from user

int timerdelay = 0;
bool key = false;
const int RED_LED_PIN = PE5; // Pin 13 on arduino 
const int BLUE_LED_PIN = PE4; // Pin 13 on arduino 

const uint16_t t1_comp = 5000;
const uint16_t t1_initial = 0; //16000; 31250

const uint16_t speedOfSound = 1.122;
uint16_t Serial_Read_Time = 9.0;
uint16_t Preset_Time_Delay = 100.0;
uint16_t startTime, endTime,totalTime, flightTime, distance;
uint16_t GlobalTimeCount = 0;

//===============================================
//              Function Declarations
//===============================================
void setup(); // Arduino Initialization Function 

void user_input(); // Recieves user input

void format(); // Formats the data being transmitted

void transmit(); // Transmits formatted data

void receive(); // Receives the formatted data

void timer_Init(); // Initializes timer 1ms (8000 pract/ theory 8170)

void disable_timer(); // Disables 1ms Timer

void enable_timer(); // Enables 1ms Timer

//===============================================
//              Function Definitions
//===============================================


void setup() 
{
  Serial.begin(1200,SERIAL_8N2);            // Initializes the Serial Port Used for serial monitor on computer
  Serial1.begin(1200,SERIAL_8N2);           // Initializes Transmiter serial port initiation (Pin 18 on Mega)
  Serial2.begin(1200, SERIAL_8N2);          // Initializes Receiver serial port initiation (Pin 17 on Mega)
  Serial2.setTimeout(500);

  DDRE |= (1<< RED_LED_PIN);
  DDRE |= (1<< BLUE_LED_PIN);

  // Enable Global interrupts 
  sei();
  timerInit();
  Serial.println("Transmitter is ready");
}

void loop() {
  user_input();  // Takes in user input the formats it for proper transmition 
  if (key == true){
    enable_timer();
    transmit();
  }  
  while(Serial2.available() == 0);
  if(Serial2.available() > 0 ) {
    receive();
    disable_timer();
    GlobalTimeCount = 0; 
  }
  
}

void enable_timer() {
  TCCR1B |= 0x01;  // This enables timer or clock source 
}
void disable_timer(){
  TCCR1B &= 0xFE; //1111 1110
  TCNT1 = 0;
}

void timerInit(){
  //Reset Timer1 Control Reg A
  TCCR1A = 0;
  TCCR1B &= ~0xDF;

  //Set the prescaler of 1
  TCCR1B |= 0x08;
  //enable_timer();
 
  // Reset Timer1 and set compare value
  TCNT1 = 0;
  OCR1A = 16000;

  //Enable Timer1 compare interrupt 
  TIMSK1 = (1 << OCIE1A);
}

void user_input(){
  //Takes in User Input For Transmittion. 
  Serial.println("Enter Input Command + Transponder ID");
  while(Serial.available() == 0){}
  command = Serial.readString();
  format();
}

ISR(TIMER1_COMPA_vect){
  PORTE ^= (1 << BLUE_LED_PIN);
  GlobalTimeCount++;
}

void format(){
  // Formats the input command + transponder id to look like this <U551> inside "data" (U = command; 55 = Ascii value of command; 1 = Slave ID) 
  data = "";
  data.concat("<");
  int ascii = command.charAt(0);
  String temp = String(ascii, HEX); //converts command to Hex
  data.concat(command.charAt(0)); //add the command first
  data.concat(temp); //add ascii hex value to the end of data
  data.concat(command.charAt(1)); //add address to the end of data
  data.concat(">");
  key = true;
 }


void transmit()
{
  for(int i = 0; i < 1; i++){
    for (int i = 0 ; i < 1; i++ )
    {
      Serial1.write(NULL);
    }
    Serial.println("Sending Data: " + data );
    for (int i = 0 ; i < data.length(); i++ )
    {
      Serial1.write(data.charAt(i));
    }
    //delay(1000);
  }
  //Serial.print("DOOOOONE");
  data = "";
  key = false;  
}

void receive()
{

  String trashData = Serial2.readStringUntil('<');   // This is here to help with properly receiving a message ('<' Acts as a start bit )
  String incomingData = Serial2.readStringUntil('>');

  totalTime = GlobalTimeCount; 
  flightTime = (totalTime - 3.0*(Serial_Read_Time) - Preset_Time_Delay)/2.0 ; 
  distance = speedOfSound*flightTime;

  Serial.print("Incoming Data: ");
  Serial.println(incomingData);

  Serial.print("Flight Time: ");
  Serial.println(flightTime);

  Serial.print("Total Time: ");
  Serial.println(totalTime);

  Serial.print("Total Distance: ");
  Serial.println(distance);

  flightTime = 0; 
  distance = 0;

}