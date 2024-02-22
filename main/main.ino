#define MASTER // this arduino is MASTER, otherwise comment this line to make it SLAVE

//=============================================//
//              Constants                      //
//=============================================//
const int RED_LED_PIN = PE5; // Pin 3 on arduino 
const int BLUE_LED_PIN = PE4; // Pin 2 on arduino

//=============================================//
//              Global variables               //
//=============================================//
bool newdata_flag = false;
int last_char_received;
int timer_time = 0;
volatile uint8_t timer_counter = 0;
#ifdef MASTER 
    volatile bool timeout_flag = true;
    int departure_time = 0;
    int arrival_time = 0;
    int turnaround_time = 0;
#else
    const char SLAVE_ID = '1';
#endif

//=============================================//
//              Function Declarations          //
//=============================================//
void setup() {
  Serial.begin(115200, SERIAL_8N2); //serial interface
  Serial1.end(); //turn off TX by default
  Serial2.begin(1200, SERIAL_8N2); //turn on RX
  timerInit(); //initialize background timer

  #ifdef MASTER 
    Serial.println("Master Arduino is ready");
  #else
    Serial.println("Slave Arduino is ready");
  #endif

  DDRE |= (1 << RED_LED_PIN);
  DDRE |= (1 << BLUE_LED_PIN);

}


//=============================================//
//              INFINITE LOOP                  //
//=============================================//
void loop() {
  #ifdef MASTER
    if(timeout_flag == true){
      timeout_flag = false;
      Serial1.write("0000000000"); // Prepare analog circuit to receive 'T'
      enableTimer(1000); // 1s timeout
      Serial1.write("T");
      departure_time = millis();
    }
    newdata_flag = receive();
    if(newdata_flag && last_char_received == 'T') //if char is T
    {
      disableTimer();
      timeout_flag = true;
      arrival_time = millis();
      last_char_received == '0';
      turnaround_time = arrival_time - departure_time;
      Serial.print("TT: ");
      Serial.println(turnaround_time);
    }
  #else
    newdata_flag = receive();
    if(newdata_flag && last_char_received == 'T') //if char is T
    {
      last_char_received == '0';
      enableTimer(100);
    }
  #endif

}

bool receive(){
  if (Serial.available())
  {
    last_char_received = Serial.read();
    return true;
  }
  return false;
}


//=============================================//
//              TIMER 1                        //
//=============================================//
void timerInit(){ //timer with ISR triggering every 1ms
  cli();  //disable interrupts
  //Reset Timer1 Control Reg A
  TCCR1A = 0;
  TCCR1B &= ~0xDF;
  TCCR1B |= 0x08; 

  // Reset Timer1 and set compare value
  TCNT1 = 0;
  OCR1A = 16000;

  //Enable Timer1 compare interrupt 
  TIMSK1 = (1 << OCIE1A);
  sei(); //enable interrupts
}

void disableTimer(){
  timer_time = -1;
  timer_counter = 0;
  TCCR1B &= ~0x01;
}
void enableTimer(int time){
  timer_time = time; // Set timer timeout time
  timer_counter = 0;
  TCCR1B |= 0x01; // This enables timer or clock source 
}


ISR(TIMER1_COMPA_vect){
  //PORTE ^= (1 << PE5);
  if( (timer_counter >= timer_time - 1) && (timer_time != -1) ){ //after timer timeouts, do the following:
    #ifdef MASTER
      timeout_flag = true;
      timer_counter = 0;
      disableTimer();
    #else
      Serial1.begin(1200, SERIAL_8N2);
      Serial1.write("0000000000");
      Serial1.write("T");
      Serial1.end();
      timer_counter = 0;
      disableTimer();
    #endif
  }else{
    timer_counter++;
  }
}