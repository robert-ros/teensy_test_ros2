#include <Adafruit_NeoPixel.h>


#define PIN 1
#define NUMPIXELS 40

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRBW);

#define PERIOD_MS 20

unsigned long last_time_ms = 0;
unsigned int ticks = 0; 
unsigned int last_time_ack_ms = 0;
bool blink_effect_flag = false;
enum command_types {ROBOT_MOVE, ROBOT_IDLE, ROBOT_EMERGENCY_STOP, ROBOT_BOOTING, ROBOT_CLEAR_SIGNAL, ROBOT_UNKNOWN_COMMAND, ROBOT_ACK};
enum state_machine_flags {LEDS_BOOTING, LEDS_READY, LEDS_RUNNING, LEDS_EXIT};
int state_machine = LEDS_BOOTING;

void setup() {

  Serial.begin(9600);
  pixels.begin();
  pixels.clear();
  
  last_time_ms = millis();
}


bool elapsedLedsTime(unsigned long time_value){
  
  bool time_reached = false;

  ticks++;
  //Serial.println(ticks);

  if (ticks*PERIOD_MS >= time_value){

      time_reached = true;
      ticks = 0;
  }
  else{
    time_reached = false;
  }

  return time_reached;
}

void run_blink_effect(int start_led, int end_led, unsigned long time_ms, int red, int blue, int green){

  if (elapsedLedsTime(time_ms)){

    if (blink_effect_flag == false){
    
       for (int i = start_led-1; i<=end_led-1; i++){
          pixels.setPixelColor(i, pixels.Color(red, blue, green));
       }
  
       pixels.show();
       blink_effect_flag = true;

    }
    else{
      
      for (int i = start_led-1; i<=end_led-1; i++){
          pixels.setPixelColor(i, pixels.Color(0, 0, 0));
       }
  
      pixels.show();
      blink_effect_flag = false;
    }
  }
  
}


void run_paint_effect(int start_led, int end_led, unsigned long time_ms, int red, int blue, int green){

   for (int i = start_led-1; i<=end_led-1; i++){
      pixels.setPixelColor(i, pixels.Color(red, blue, green));
   }

   pixels.show();
  
}

void run_clear_effect(){
  pixels.clear();
  pixels.show();
}

void run_boot_teensy(){
  
  run_blink_effect(1, 40, 500, 150, 150, 150);
}

void run_ready_teensy(){
  
  run_paint_effect(1, 40, 500, 0, 150, 0);
}


void run_exit_teensy(){

  run_paint_effect(1, 40, 500, 0, 0, 150);
}

void run_move_signal(){
  
  run_blink_effect(1, 40, 500, 0, 150, 0);
}

void run_idle_signal(){

  run_paint_effect(1, 40, 500, 0, 150, 0);
}

void run_emergency_signal(){
  
  run_blink_effect(1, 40, 500, 150, 0, 0);
}

void run_clear_signal(){
  
  run_clear_effect();
}


void leds_booting(){
  
  run_boot_teensy();
}

void leds_ready(){

  run_ready_teensy();
}

void leds_exit(){

  run_exit_teensy();
}

void leds_running(int command){

  switch (command){
  case ROBOT_MOVE:
    run_move_signal();
    break;
  case ROBOT_IDLE:  
    run_idle_signal();
    break;
  case ROBOT_EMERGENCY_STOP:
    run_emergency_signal();
    break;
   case ROBOT_CLEAR_SIGNAL:
    run_clear_signal();
    break;  
  }

  //run_boot_teensy();
  //run_ready_teensy();
  //run_exit_teensy();
  
  //run_move_signal();
  //run_idle_signal();
  //run_emergency_signal();
}

int get_led_command(){

  static int current_command = ROBOT_UNKNOWN_COMMAND;
  char value = 0;
  
  while(Serial.available() > 0){

    char inChar = Serial.read();

    if (inChar != '\n'){
      value = inChar;
    }
    
  }

  switch (value){
    case 'a':
      current_command = ROBOT_MOVE;
      last_time_ack_ms = millis();
      break;
    case 'b':  
      current_command = ROBOT_IDLE;
      last_time_ack_ms = millis();
      break;
    case 'c':
      current_command = ROBOT_EMERGENCY_STOP;
      last_time_ack_ms = millis();
      break;  
    case 'd':
      current_command = ROBOT_CLEAR_SIGNAL;
      last_time_ack_ms = millis();
      break;
  }

  return current_command;
}


bool checkConnectionCPU(){

  bool is_connected = true;
  
  if (millis() - last_time_ack_ms > 5000){
  
    is_connected = false;
  }

  return is_connected;
}


void loop() {

  if ((millis() - last_time_ms) > PERIOD_MS){


    if (state_machine == LEDS_BOOTING){

      leds_booting();

      if (SerialUSB){
        state_machine = LEDS_READY;
      }
    }

    else if (state_machine == LEDS_READY){

      leds_ready();
      state_machine = LEDS_RUNNING;
      last_time_ack_ms = millis();
    }

    else if (state_machine == LEDS_RUNNING){

      int command = get_led_command();
      leds_running(command);

      if (checkConnectionCPU() == false){
        state_machine = LEDS_EXIT;
      }
    }

    else if (state_machine == LEDS_EXIT){

      leds_exit();

      get_led_command();
      
      if (checkConnectionCPU() == true){
        state_machine = LEDS_READY;
      }
      
    }
      
    last_time_ms = millis();
    
  }

}
