#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <AccelStepper.h>
#include <rotator.h>
#include <comms.h>
Rotator rotator;
Comms comms;
#define DIR_AZ 39 /*PIN for Azimuth Direction*/ //4
#define STEP_AZ 45 /*PIN for Azimuth Steps*/  //18 //nav 0 ? nav nekas jēdzīgs
#define DIR_EL 47 /*PIN for Elevation Direction*/ //33
#define STEP_EL 48 /*PIN for Elevation Steps*/ //23

#define EN 40 /*PIN for Enable or Disable Stepper Motors*/ //19
#define EN2 33 /*PIN for Enable or Disable Stepper Motors*/ //32
//#define Microstepping 5
#define STEPPERS_ENABLE() digitalWrite(EN, HIGH)
#define STEPPERS_DISABLE() digitalWrite(EN, LOW)

#define STEPPERS_ENABLE2() digitalWrite(EN2, HIGH)
#define STEPPERS_DISABLE2() digitalWrite(EN2, LOW)

#define SPR 1600 /*Step Per Revolution driver 200*/
#define RATIO 30 /*54 Gear ratio*/
#define T_DELAY 60000 /*Time to disable the motors in millisecond*/
#define HOME_AZ 0 /*Homing switch for Azimuth*/
#define HOME_EL 0 /*Homing switch for Elevation*/

#define MAX_AZ_ANGLE 365 /*Maximum Angle of Azimuth for homing scanning*/
#define MAX_EL_ANGLE 365 /*Maximum Angle of Elevation for homing scanning*/

#define MAX_SPEED 800 //300, 800
#define MAX_ACCELERATION 600 //100, 600


uint8_t homingEnabled = 0;

#define MIN_PULSE_WIDTH 50/*in microsecond*  50*/

#define DEFAULT_HOME_STATE HIGH /*Change to LOW according to Home sensor*/

#define HOME_DELAY 10000 /*6000 Time for homing Decceleration in millisecond*/

#define TRACK_DETECT_TRESH_MS 13000UL // kolko ms od posledneho paketu chapeme, ze trackujeme satelit.
unsigned long last_track_ms = 0;
uint8_t is_tracking = 0;
// v rezime track detect rotator ide do urceneho bodu najkratsou cestou

#define BufferSize 256
#define BaudRate 115200

/*Global Variables*/
unsigned long t_DIS = 0; /*time to disable the Motors*/
/*Define a stepper and the pins it will use*/
AccelStepper AZstepper(1, STEP_AZ, DIR_AZ);
AccelStepper ELstepper(1, STEP_EL, DIR_EL);
/*Error Handling*/
/*Convert degrees to steps*/
long deg2step(double deg) 
{
  double a = RATIO*SPR*deg/360;
  return(a);
}

/*Convert steps to degrees*/
double step2deg(long Step) 
{
  return(360.00*Step/(SPR*RATIO));
}
/*Check if is argument in number*/
boolean isNumber(char *input)
{
  for (long i = 0; input[i] != '\0'; i++)
  {
    if (isalpha(input[i]))
      return false;
  }
   return true;
}
void error(long num_error)
{
  switch (num_error)
  {
    /*Azimuth error*/
    case (0):
      while(1)
      {
        Serial.println("AL001");
        delay(100);
      }
    /*Elevation error*/
    case (1):
      while(1)
      {
        Serial.println("AL002");
        delay(100);
      }
    default:
      while(1)
      {
        Serial.println("AL000");
        delay(100);
      }
  }
}

/*Homing Function*/
void Homing(long AZsteps, long ELsteps)
{
  long value_Home_AZ = DEFAULT_HOME_STATE;
  long value_Home_EL = DEFAULT_HOME_STATE;
  boolean isHome_AZ = false;
  boolean isHome_EL = false;
  
  AZstepper.moveTo(AZsteps);
  ELstepper.moveTo(ELsteps);
  
  while(isHome_AZ == false || isHome_EL == false)
  {
    value_Home_AZ = digitalRead(HOME_AZ);
    value_Home_EL = digitalRead(HOME_EL);
    /*Change to LOW according to Home sensor*/
    if (value_Home_AZ == DEFAULT_HOME_STATE)
    {
      AZstepper.moveTo(AZstepper.currentPosition());
      isHome_AZ = true;
    }   
    /*Change to LOW according to Home sensor*/
    if (value_Home_EL == DEFAULT_HOME_STATE)
    {
      ELstepper.moveTo(ELstepper.currentPosition());
      isHome_EL = true;
    }
    if (AZstepper.distanceToGo() == 0 && !isHome_AZ)
    {
      error(0);
      break;
    }
    if (ELstepper.distanceToGo() == 0 && !isHome_EL)
    {
      error(1);
      break;
    }
    AZstepper.run();
    ELstepper.run();
  }
  /*Delay to Deccelerate*/
  long time = millis();  
  while(millis() - time < HOME_DELAY)
  {  
    AZstepper.run();
    ELstepper.run();
  }
  /*Reset the steps*/
  AZstepper.setCurrentPosition(0);
  ELstepper.setCurrentPosition(0); 
}

int is_number(char a)
{
  if(a == '0' || a == '1' || a == '2' || a == '3' || a == '4' || a == '5' || a == '6' || a == '7' || a == '8' || a == '9' || a == '.' || a == '+' || a == '-' || a == ' '){
    return 1; 
  }
  return 0;
}

// zistime, kolko casu preslo od posledneho prijateho prikazu na zmenu smeru
void track_timer_reset_detect(void)
{           
            unsigned long ms = millis();
            Serial.println(ms - last_track_ms);
            if((ms - last_track_ms) < TRACK_DETECT_TRESH_MS){
              is_tracking = 1;
            }else{
              is_tracking = 0;
            }
            last_track_ms = ms;
}

double my_abs(double in)
{
  if(in < 0) return -in;
  return in;
}

// skrati cestu azimutu, treba pri prechode cez nulu
// pouziva sa pri tracking, teda ked packety chodia rychlo za sebou
// ked neni tracking, vrati sa to na absolutnu polohu, aby sa odmotali kable!
double shortest_az(double in_old, double in_new)
{
  double a, b, c;
  a = my_abs(in_old - in_new);
  b = my_abs(in_old - (in_new + 360));
  c = my_abs(in_old - (in_new - 360));
  
  if((a < b) && (a < c)) return in_new;
  if((b < a) && (b < c)) return in_new + 360;
  return in_new - 360;
}
 
/*EasyComm 2 Protocol & Calculate the steps*/
void cmd_proc(long &stepAz, long &stepEl)
{
  /*Serial*/
  char buffer[BufferSize];
  char incomingByte;
  char *Data = buffer;
  char *rawData;
  static long BufferCnt = 0;
  char data[100];
  
  double angleAz, angleEl;
  
  /*Read from serial*/
  while (Serial.available() > 0)
  {
    incomingByte = Serial.read();
    //Serial.write(incomingByte); // Debug adam
    /* XXX: Get position using custom and test code */
    if (incomingByte == '!')
    {
      /*Get position*/
      Serial.print("TM");
      Serial.print(1);
      Serial.print(" ");
      Serial.print("AZ");
      Serial.print(10*step2deg(AZstepper.currentPosition()), 1);
      Serial.print(" ");
      Serial.print("EL");
      Serial.println(10*step2deg(ELstepper.currentPosition()), 1);
      if(is_tracking){
        Serial.println("t");
      }else{
        Serial.println("n");
      }
    }
    /*new data*/
    else if (incomingByte == '\n' || incomingByte == '\r' || incomingByte == 'D')
    {
      buffer[BufferCnt] = 0;
      if (buffer[0] == 'A' && buffer[1] == 'Z')
      {
        if (buffer[2] == ' ' && buffer[3] == 'E' && buffer[4] == 'L')
        {
          /*Get position*/
          Serial.print("AZ");
          Serial.print(step2deg(AZstepper.currentPosition()), 1);
          Serial.print(" ");
          Serial.print("EL");
          Serial.print(step2deg(ELstepper.currentPosition()), 1);
          Serial.println(" ");
        }
        else
        {
          uint8_t a, b;

          if(is_number(buffer[2])){
            // orbitron bez rezimu Split out pise AZ
             a = 2; //pozicia prvej cislice azimut   AZ
          }else{
            // orbitron v rezime Split out pise AZ:
            a = 3; //pozicia prvej cislice azimut   AZ, preskoc :
          }
          
          b = 0; 
          Serial.print("AZ parse:");
          while(is_number(buffer[a]) && (a < BufferCnt)){
            Serial.print("*");
            data[b] = buffer[a]; // kopirujeme cislice do data[]
            b++;
            a++; //hladame poslednu cislicu azimut AZ:180.0
          }
          data[b] = 0; //ukonc string
          Serial.print(data);
          Serial.print("$\n");
          /*Get the absolute value of angle*/
          if (data[0] != 0 && a != BufferCnt && isNumber(data))
          {
            track_timer_reset_detect(); //zistime, ci trackujeme
            if(is_tracking){
              Serial.print("Tracking!\n");
              double angleAz_new = atof(data);
              angleAz = step2deg(AZstepper.currentPosition());
              angleAz = shortest_az(angleAz, angleAz_new); // ked trackujeme, skracujeme cestu
            }else{
              angleAz = atof(data);  
            }
            
            
            /*Calculate the steps*/
            stepAz = deg2step(angleAz);
          }
          
          while(buffer[a] != 'L' && a < BufferCnt){ // hladame koniec stringu EL
            a++;
            Serial.print("^");
          }
          a++; //preskoc L

          if(!is_number(buffer[a])){
            // orbitron v rezime Split out pise EL:
            a++; //preskoc :
          }
          
          b = 0;
          Serial.print("EL parse:"); 
          while(is_number(buffer[a]) && (a < BufferCnt)){
            Serial.print("*");
            data[b] = buffer[a]; // kopirujeme cislice do data[]
            b++;
            a++; //hladame poslednu cislicu elevacia
          }
          data[b] = 0; //ukonc string
          Serial.print(data);
          Serial.print("$\n");
          if (data[0] != 0 && isNumber(data))
          {
            angleEl = atof(data);
            /*Calculate the steps*/
            stepEl = deg2step(angleEl);
          }
        }
      }
      /*Stop Moving*/
      else if (buffer[0] == 'S' && buffer[1] == 'A' && buffer[2] == ' ' && buffer[3] == 'S' && buffer[4] == 'E')
      {
        /*Get position*/
        Serial.print("AZ");
        Serial.print(step2deg(AZstepper.currentPosition()), 1);
        Serial.print(" ");
        Serial.print("EL");
        Serial.println(step2deg(ELstepper.currentPosition()), 1);
        stepAz = AZstepper.currentPosition();
        stepEl = ELstepper.currentPosition();
      }
      /*Reset the rotator*/
      else if (buffer[0] == 'R' && buffer[1] == 'E' && buffer[2] == 'S' && buffer[3] == 'E' && buffer[4] == 'T')
      {
        /*Get position*/
        Serial.print("AZ");
        Serial.print(step2deg(AZstepper.currentPosition()), 1);
        Serial.print(" ");
        Serial.print("EL");
        Serial.println(step2deg(ELstepper.currentPosition()), 1);
        /*Move the steppers to initial position*/
        Homing(deg2step(-MAX_AZ_ANGLE), deg2step(-MAX_EL_ANGLE));
        /*Zero the steps*/
        stepAz = 0;
        stepEl = 0;
      }      
      BufferCnt = 0;
      /*Reset the disable motor time*/
      t_DIS = 0;
    }
    /*Fill the buffer with incoming data*/
    else if(BufferCnt == 0 && incomingByte != 'A'){
      // Nespravny prvy znak neukladaj do buffera
    }else{   
      buffer[BufferCnt] = incomingByte;
      if(BufferCnt < BufferSize-1){
       BufferCnt++; 
      }
    }
  }
}



/*Send pulses to stepper motor drivers*/
void stepper_move(long stepAz, long stepEl)
{
  AZstepper.moveTo(stepAz);
  ELstepper.moveTo(stepEl);
    
  AZstepper.run();
  ELstepper.run();
}



void setup()
{  

  AZstepper.setPinsInverted(true, true, true);
  ELstepper.setPinsInverted(true, true, true);
 // pinMode(27, OUTPUT);
  /*Change these to suit your stepper if you want*/
  AZstepper.setMaxSpeed(MAX_SPEED);
  AZstepper.setAcceleration(MAX_ACCELERATION);
  /*Change these to suit your stepper if you want*/
  ELstepper.setMaxSpeed(MAX_SPEED);
  ELstepper.setAcceleration(MAX_ACCELERATION);
  /*Set minimum pulse width*/
  AZstepper.setMinPulseWidth(MIN_PULSE_WIDTH);
  ELstepper.setMinPulseWidth(MIN_PULSE_WIDTH);
  /*Enable Motors*/
  pinMode(EN, OUTPUT);
  pinMode(EN2, OUTPUT);
  STEPPERS_ENABLE(); 
  STEPPERS_ENABLE2();

  /*Homing switch*/
  pinMode(HOME_AZ, INPUT_PULLUP);
  pinMode(HOME_EL, INPUT_PULLUP);
 // pinMode(Microstepping, OUTPUT);
  //digitalWrite(Microstepping, HIGH);
  
  /*Serial Communication*/
  //Serial.begin(BaudRate);
  rotator.init(rotator.comms, rotator.config);
  
  if(homingEnabled){
    /*Initial Homing*/
    Homing(deg2step(-MAX_AZ_ANGLE), deg2step(-MAX_EL_ANGLE));
  }else{
    AZstepper.setCurrentPosition(0);
    ELstepper.setCurrentPosition(0); 
  }
}
  const int LORA_MOSI = 10;
  const int LORA_MISO = 11;
  const int LORA_SCK = 9;
  const int LORA_NSS = 8;
  const int LORA_DIO1 = 14;
  const int LORA_NRST = 12;
  const int LORA_BUSY = 13;

  // LoRa object
  SX1262 lora = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
bool state = false;
 String str;
void loop(){
  ///state = rotator.comms.lora_receive(rotator.com
  //newData(rotator.comms,rotator.config);
   int sk = 0;
   String a = "";
   String b = "";
   /*Define the steps*/
   static long AZstep = 0;
   static long ELstep = 0;
   /*Time Check*/
   if (t_DIS == 0)
     t_DIS = millis();

   /*Disable Motors*/
   if (AZstep == AZstepper.currentPosition() && ELstep == ELstepper.currentPosition() && millis()-t_DIS > T_DELAY){
     STEPPERS_DISABLE(); 
     STEPPERS_DISABLE2();
    
   }else{
   /*Enable Motors*/
     STEPPERS_ENABLE(); 
     STEPPERS_ENABLE2();
   }

   //some notes. 
   //First off, for whatever reason, THIS DOESNT WORK IF YOU RUN IT THROUGH EXTERNAL FUNCTIONS (ie calling a function from a class). Only way to do it is to have the function here
   //dont change the pins, whatever you do.
   // Do not double up on converting the angle to steps. 
   //DO NOT PUT STEPPER_MOVE INTO AN IF CLAUSE, IT WILL NOT WORK, IT NEEDS TO BE CONSTANTLY CHECKING IF IT NEEDS TO MAKE A STEP.
   //dont use pin 27.
   //why is this so slow????
  
   if(Serial.available() > 0)
  {
    String msg = Serial.readString();
    Serial.println("Message is: " + msg);
    int msg_len = msg.length() + 1;
    char char_array[msg_len];
    msg.toCharArray(char_array, msg_len);

    char *token = strtok(char_array, ",");
    rotator.ballon_position.latitude = atof(token);
    token = strtok(NULL, ",");
    rotator.ballon_position.longitude = atof(token);
    token = strtok(NULL, ",");
    rotator.ballon_position.altitude = atof(token);
    
    rotator.update_angles(rotator.comms, rotator.ballon_position, rotator.config);
    AZstep = rotator.comms.data_to_rotator.azimuth;
    ELstep = rotator.comms.data_to_rotator.elevation;
     AZstep =  AZstep*(SPR*RATIO/360);
    ELstep = ELstep*(SPR*RATIO/360);
    Serial.println(String(AZstep)+ ","+ String(ELstep));

    
  }
  

 int state = lora.receive(str);
  if (state != RADIOLIB_ERR_NONE && state != -6)
  {
    Serial.println("LoRa receive error: " + String(state));
  }

  comms.received_data.msg = str;
  comms.received_data.rssi = lora.getRSSI();
  comms.received_data.snr = lora.getSNR();
  Serial.println("Received data: " + comms.received_data.msg);
  Serial.println("RSSI: " + String(comms.received_data.rssi));
  Serial.println("SNR: " + String(comms.received_data.snr));

  //Start receiving again
  lora.finishTransmit();
  state = lora.startReceive();
  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa receive restart failed: " + String(state));
  }

  
    stepper_move(AZstep, ELstep);
    // digitalWrite(27, HIGH);

  //this was already commented out in the original, with that precise wording. No clue whats going on here, albeit at a glance, alternate method for running the rotator ~ Grinums
  /*Read the steps from serial*/ 
 // cmd_proc(AZstep, ELstep);
  
  
  
  
}