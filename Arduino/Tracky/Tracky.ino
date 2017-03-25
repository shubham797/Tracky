#include <SoftwareSerial.h>
#include "CurieIMU.h"

SoftwareSerial GPS(2, 3); // Rx, Tx
SoftwareSerial GSMmodule(4, 5); // Rx, Tx

int Gpsdata;             // for incoming serial data
unsigned int finish = 0; // indicate end of message
unsigned int pos_cnt = 0; // position counter
unsigned int lat_cnt = 0; // latitude data counter
unsigned int log_cnt = 0; // longitude data counter
unsigned int flg = 0;    // GPS flag
unsigned int com_cnt = 0; // comma counter
char lt[20];            // latitude array
char lg[20];             // longitude array

char m_num_my[] = {"+918871408244"}; // Enter your number here
char Rec_Data = 0;
int msg_flg = 0;
byte msg_flag_2 = 0;
byte call_flag = 0;
byte reply_flag = 0;
byte delete_flag = 0;
byte d_count = 0;
byte i = 0;
byte j = 0;
char number[15];
char message[160];

int avgX = 0;
int avgY = 0;
int avgZ = 0;
//int count = 0;

boolean alert = 0;
boolean theft = 0;

// Functions
void setReference();
void GPSData();
void modem_initialization(void);
void send_message(void);
void command_match(void);
void gsm_read_message();
//void ring();
/*******************************************************************************************
Function Name: Setup
*******************************************************************************************/
void setup() {
  delay(3000);
  //Serial.begin(9600);
  Serial1.begin(9600); // initialize serial communication for BT
  GPS.begin(9600); // initialize serial communication for GPS
  GSMmodule.begin(9600); // initialize serial communication for GSM
  
  Serial1.println("Initializing IMU device...");
  CurieIMU.begin(); // initialize device
  CurieIMU.setAccelerometerRange(3); // Set the accelerometer range to 3G

  Serial1.println("Modem initializing");
  modem_initialization(); // initialize GSM modem
  Serial1.println("Modem initialised");
  delay(1000);
}
/*******************************************************************************************
Function Name: Loop
*******************************************************************************************/
void loop() {
  gsm_read_message();//wait for message or call
  if ( reply_flag == 1 ) //only if valid message received
  {
    Serial1.println(message);
    command_match();
    reply_flag = 0;
  }
  //ring();
  if (Serial1.available() > 0) {
    char ch = Serial1.read();
    if (ch == '1' && alert == 0) {
      // Calculate reference
      setReference();
      alert = 1;
    } else if (ch == '0') {
      alert = 0;
      theft = 0;
      Serial1.println("Reseting. . .");
    } else if (ch == '2') {
      Serial1.println("GPS");
      GPSData();
      Serial1.print("G_");
      Serial1.print(lt);
      Serial1.print("_");
      Serial1.println(lg);
    }
  }

  if (alert == 1) {
    int x = abs(abs(CurieIMU.readAccelerometer(X_AXIS)) - avgX);
    int y = abs(abs(CurieIMU.readAccelerometer(Y_AXIS)) - avgY);
    int z = abs(abs(CurieIMU.readAccelerometer(Z_AXIS)) - avgZ);
    int s = (x + y + z) / 3;
    Serial1.print("Curret position: ");
    Serial1.print("\t");
    Serial1.println(s);
    if (s > 1000) {
      theft = 1;
      Serial1.println("ALERT");
      Serial1.println();     
    }
  }

  //long long lastMillis = 0;
  while (theft) {
    digitalWrite(13, HIGH);
    if (Serial1.available() > 0 && Serial1.read() == '0') {
      alert = 0;
      theft = 0;
      Serial1.println("Reseting. . .");
    }
  }
  digitalWrite(13, LOW);
  pos_cnt = 0; finish = 0;
  delay(50);
}
/*******************************************************************************************
Function Name: command_match
*******************************************************************************************/
void command_match(void)
{
  if (strcmp(message, "1") == 0)
  {
    send_message();
    delay(50);
  }
  /*else if ((strcmp(message, "off") == 0) || (strcmp(message, "OFF") == 0))
  {
    digitalWrite(light, LOW);
    Serial.println("Turning LED off");
    delay(50);  
  }*/
}
/*******************************************************************************************
Function Name: setReference
*******************************************************************************************/
void setReference() {
  long sumX = 0;
  long sumY = 0;
  long sumZ = 0;
  // Calculate reference position
  for (int i = 0; i < 1000; i++) {
    int x = abs(CurieIMU.readAccelerometer(X_AXIS));
    int y = abs(CurieIMU.readAccelerometer(Y_AXIS));
    int z = abs(CurieIMU.readAccelerometer(Z_AXIS));
    sumX += x;
    sumY += y;
    sumZ += z;
  }
  avgX = sumX / 1000;
  avgY = sumY / 1000;
  avgZ = sumZ / 1000;
  Serial1.print("Reference position: ");
  Serial1.print("\t");
  Serial1.print(avgX);
  Serial1.print("\t");
  Serial1.print(avgY);
  Serial1.print("\t");
  Serial1.println(avgZ);
  Serial1.println();
}
/*******************************************************************************************
Function Name: GPSData
*******************************************************************************************/
void GPSData() {
  //while (!finish) {
  //Serial1.println(GPS.available());
    while (GPS.available() > 0) {       // Check GPS data
      Gpsdata = GPS.read();
      //Serial1.write(Gpsdata);
      //Serial1.println("looping");
      flg = 1;
      if ( Gpsdata == '$' && pos_cnt == 0) // finding GPRMC header
        pos_cnt = 1;
      if ( Gpsdata == 'G' && pos_cnt == 1)
        pos_cnt = 2;
      if ( Gpsdata == 'P' && pos_cnt == 2)
        pos_cnt = 3;
      if ( Gpsdata == 'R' && pos_cnt == 3)
        pos_cnt = 4;
      if ( Gpsdata == 'M' && pos_cnt == 4)
        pos_cnt = 5;
      if ( Gpsdata == 'C' && pos_cnt == 5 )
        pos_cnt = 6;
      if (pos_cnt == 6 &&  Gpsdata == ',') { // count commas in message
        com_cnt++;
        flg = 0;
      } if (com_cnt == 3 && flg == 1) {
        lt[lat_cnt++] =  Gpsdata;         // latitude
        flg = 0;
      } if (com_cnt == 5 && flg == 1) {
        lg[log_cnt++] =  Gpsdata;         // Longitude
        flg = 0;
      } if ( Gpsdata == '*' && com_cnt >= 5) {
        com_cnt = 0;                      // end of GPRMC message
        lat_cnt = 0;
        log_cnt = 0;
        flg     = 0;
        finish  = 1;
      }
    }
  //}
}
/*******************************************************************************************
Function Name: modem_initialization
*******************************************************************************************/
void modem_initialization(void)
{
  char rec_data;
  byte network_status = 0 ;//network_status initialized as zero
  byte status_check = 0 ;//status_check initialized as zero
  byte gsm_cnt = 0;//gsm_cnt initialized as zero
  byte ok_flag = 0; //ok_flag initialized as zero
  //byte count = 0; //count initialized as zero
  clear_rx_buffer();//clearing receiving buffer
  while (gsm_cnt < 5) // repeat entire loop until gsm_cnt less than seven
  {
    switch (gsm_cnt)
    {
      case 0: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("AT");// Attention command to wake up GSM modem
        delay(1000);
        break;
      case 1: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("ATE0");//Command for disable echo
        delay(1000);
        break;
      case 2: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("ATV0");// Command for numeric response after this '0'(zero) will be recieved instead of "OK"
        delay(1000);
        break;
      case 3: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("AT&W");// Command TO SAVE SETTINGS
        delay(1000);
        break;
      case 4: gsm_cnt = 5;// exit from the loop
        break;
      default : break;
    }
    while (GSMmodule.available() > 0)
    {
      rec_data = GSMmodule.read();
      if (rec_data == 'O' ) // 'o' is recieved
        ok_flag = 1;
      else if (ok_flag == 1 && rec_data == 'K' ) // 'K' is recieved
      {
        gsm_cnt ++;
        ok_flag = 0;
      }
      else if (rec_data == '0' )// '0'(zero) is recieved (numeric response)
      {
        gsm_cnt ++;
        ok_flag = 0;
      }
      else if (rec_data == '+' )// '+' is recieved message
      {
        clear_rx_buffer();//clearing receiving buffer

        if ( gsm_cnt > 0 )
          gsm_cnt --;
      }
    }
  }
  gsm_cnt = 0;
  ok_flag = 0;
  while (network_status == 0 ) //wait for PIN READY
  {
    if (status_check == 0 )
    {
      delay(1000);
      status_check = 1 ;
      clear_rx_buffer();//clearing receiving buffer
      GSMmodule.println("AT+CPIN?"); //checkin PIN return ready(+CPIN: READY) with a valid simcard otherwise error
    }
    while (GSMmodule.available() > 0)
    {
      rec_data = GSMmodule.read();
      if ( rec_data == '+' && status_check == 1 ) // '+' is recieved
        status_check = 2 ;
      else if ( rec_data == 'C' && status_check == 2 ) // 'C' is recieved
        status_check = 3 ;
      else if ( rec_data == 'P' && status_check == 3 ) // 'P' is recieved
        status_check = 4 ;
      else if ( rec_data == 'I' && status_check == 4 ) // 'I' is recieved
        status_check = 5 ;
      else if ( rec_data == 'N' && status_check == 5 ) // 'N' is recieved
        status_check = 6 ;
      else if ( rec_data == ':' && status_check == 6 ) // ':' is recieved
        status_check = 7 ;
      else if ( rec_data == ' ' && status_check == 7 ) // ' ' is recieved
        status_check = 8 ;
      else if ( rec_data == 'R' && status_check == 8 ) // 'R' is recieved
        status_check = 9 ;
      else if ( rec_data == 'E' && status_check == 9 ) // 'E' is recieved
        status_check = 10 ;
      else if ( rec_data == 'A' && status_check == 10 ) // 'A' is recieved
        status_check = 11 ;
      else if ( rec_data == 'D' && status_check == 11 ) // 'D' is recieved
        status_check = 12 ;
      else if ( rec_data == 'Y' && status_check == 12 ) // 'Y' is recieved
        status_check = 13 ;
      else if ( rec_data == 0X0D && status_check == 13 ) //Carriage return
        status_check = 14 ;
      else if ( rec_data == 0X0A && status_check == 14 ) //Line Feed
        status_check = 15 ;
      else if ( rec_data == '0' && status_check == 15 ) // '0' is recieved
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 1; //goto next step
      }
      else if ( rec_data != 'R' && status_check == 8 ) //+CPIN: NOT READY
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 0; //repeat current step
      }
      else if ( rec_data == 'M' && status_check == 3 ) // in case of any message
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 0; //repeat current step
      }
    }
  }
  while (network_status == 1 ) //wait for SIM network registration
  {
    if (status_check == 0 )
    {
      delay(1000);
      status_check = 1 ;
      clear_rx_buffer();//clearing receiving buffer
      GSMmodule.println("AT+CREG?"); //checking for SIM card registration ,if registerd "+CREG: 0,1" will receive
    }
    while (GSMmodule.available() > 0)
    {
      rec_data = GSMmodule.read();
      if ( rec_data == '+' && status_check == 1 ) // '+' is recieved
        status_check = 2 ;
      else if ( rec_data == 'C' && status_check == 2 ) // 'C' is recieved
        status_check = 3 ;
      else if ( rec_data == 'R' && status_check == 3 ) // 'R' is recieved
        status_check = 4 ;
      else if ( rec_data == 'E' && status_check == 4 ) // 'E' is recieved
        status_check = 5 ;
      else if ( rec_data == 'G' && status_check == 5 ) // 'G' is recieved
        status_check = 6 ;
      else if ( rec_data == ':' && status_check == 6 ) // ':' is recieved
        status_check = 7 ;
      else if ( rec_data == ' ' && status_check == 7 ) // ' ' is recieved
        status_check = 8 ;
      else if ( rec_data == '0' && status_check == 8 ) // '0' is recieved
        status_check = 9 ;
      else if ( rec_data == ',' && status_check == 9 ) // ',' is recieved
        status_check = 10 ;
      else if ( rec_data == '1' && status_check == 10 ) // '1' is recieved
        status_check = 11 ;
      else if ( rec_data == 0X0D && status_check == 11 ) //Carriage return
        status_check = 12 ;
      else if ( rec_data == 0X0A && status_check == 12 ) //Line Feed
        status_check = 13 ;
      else if ( rec_data == '0' && status_check == 13 ) // '0' is recieved
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 2; //goto next step
      }
      else if ( rec_data != '1' && status_check == 10 ) // +CREG: 0,2 not registered
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 1; //repeat current step
      }
      else if ( rec_data == 'M' && status_check == 3 ) // in case of any message
      {
        clear_rx_buffer();//clearing receiving buffer
        status_check = 0 ;
        network_status = 1; //repeat current step
      }
    }
  }
  gsm_cnt = 0;
  while (gsm_cnt < 4) // repeat entire loop until gsm_cnt less than nine
  {
    switch (gsm_cnt)
    {
      case 0: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("AT+CMGF=1");// Attention command to wake up GSM modem
        delay(1000);
        break;
      case 1: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("AT+CNMI=2,1,0,0,0");//Command to configure new message indication
        delay(1000);
        break;
      case 2: clear_rx_buffer();//clearing receiving buffer
        GSMmodule.println("AT+CMGD=1,4");// Command to delete all received messages
        delay(1000);
        break;
      case 3: gsm_cnt = 4;// exit from the loop
        break;
      default : break;
    }
    while (GSMmodule.available() > 0)
    {
      rec_data = GSMmodule.read();
      if (rec_data == '0' )// '0'(zero) is recieved (numeric response)
      {
        gsm_cnt ++;
      }
      else if (rec_data == '+'  )// '+' recieved ,before "AT+CIICR" command,may be any message
      {
        clear_rx_buffer();//clearing receiving buffer
        if ( gsm_cnt > 0 )
          gsm_cnt --;
      }

      else if (rec_data == '4' && gsm_cnt > 2)// '4' recieved (error),in gprs initialisation commands
      {
        clear_rx_buffer();//clearing receiving buffer
        gsm_cnt = 4;
      }
    }
  }
  gsm_cnt = 0;
}
/*******************************************************************************************
Function Name: gsm_read_message
*******************************************************************************************/
void gsm_read_message(void)
{
  while (GSMmodule.available() > 0)
  {
    Rec_Data = GSMmodule.read();
    //Serial.write(Rec_Data);
    if ( Rec_Data == '+'  && msg_flg == 0 ) // '+' is recieved
      msg_flg = 1;
    else if ( Rec_Data == 'C' && msg_flg == 1 ) // 'C' is recieved
      msg_flg = 2;
    else if ( Rec_Data == 'M' && msg_flg == 2 ) // 'M' is recieved
      msg_flg = 3;
    else if ( Rec_Data == 'T' && msg_flg == 3 ) // 'T' is recieved
      msg_flg = 4;
    else if ( Rec_Data == 'I' && msg_flg == 4 ) // 'I' is recieved (INCOMING MESSAGE )
    {
      clear_rx_buffer();//clearing receiving buffer
      Rec_Data = 0;
      msg_flg = 0;
      delay(300);//Delay for GSM module become ready
      array_clear();//Clear both message and number arrays
      GSMmodule.println("AT+CMGR=1");//Command for message read from location one
      i = 0; j = 0;
    }
    else if ( Rec_Data == 'G' && msg_flg == 3 ) // 'G' is recieved
      msg_flg = 4;
    else if ( Rec_Data == 'R' && msg_flg == 4 ) // 'R' is recieved,Readind message
      msg_flg = 5;
    else if ( Rec_Data == ':' && msg_flg == 5 ) // ':' is recieved
      msg_flg = 6;
    else if ( Rec_Data == '"' && msg_flg == 6  ) // Counting double quotes
      d_count++;
    else if ( Rec_Data != '"' && d_count == 3 && msg_flg == 6) // Saving number to message array from between third and fourth double quotes
      number[i++] = Rec_Data;
    else if ( Rec_Data == 0X0D && msg_flg == 6) //Carriage return
      msg_flg = 7;
    else if ( Rec_Data == 0X0A && msg_flg == 7) //Line feed
      msg_flg = 8;
    else if ( Rec_Data == '*' && d_count >= 5 &&  msg_flg == 8 ) //Start symbol '*' is recieved
      msg_flag_2 = 1;
    else if ( Rec_Data != '#' && msg_flag_2 == 1 &&  msg_flg == 8 && Rec_Data != 0X0D && Rec_Data != 0X0A ) //Data between start and stop symbols were saved to message array
      message[j++] = Rec_Data;
    else if ( Rec_Data == '#' && msg_flag_2 == 1 &&  msg_flg == 8) //Stop symbol '#' is recieved
    {
      msg_flag_2 = 2;
      number[i] = '\0';
      message[j] = '\0';
      i = 0; j = 0;
    }
    else if (Rec_Data == 0X0D && msg_flg == 8) //Carriage return (end of message)
    {
      clear_rx_buffer();//clearing receiving buffer
      msg_flg = 0;
      delete_flag = 1;
      delay(300);//Delay for GSM module become ready
      GSMmodule.println("AT+CMGD=1,4");// Command for delete all messages in SIM card
    }
    else if ( Rec_Data == '0' && msg_flg == 0 && delete_flag == 1 ) //Response for delete command ,sucess all messages were deleted
    {
      if ( msg_flag_2 == 2 )
        reply_flag = 1;//If the message have start and stop symbols it must be checked otherwise no need to check the recieved message
      else
        reply_flag = 0;//No need to check recieved message
      msg_flg = 0;
      msg_flag_2 = 0;
      d_count = 0;
      delete_flag = 0;
    }
    else if ( Rec_Data == '2' && msg_flg == 0 ) // '2' is recieved,incoming call
    {
      call_flag = 1;
      delay(300);
      GSMmodule.println("ATH");//Command for hang up incomming call,all calls must be filtered no need to answer a call
    }
    else if ( Rec_Data == '0' && msg_flg == 0 && call_flag == 1 ) // '0' (zero) is recieved,call sucessfully rejected
    {
      msg_flg = 0;
      call_flag = 0;
      msg_flag_2 = 0;
      d_count = 0;
      reply_flag = 0;
    }
    else if ( (Rec_Data == '3' || Rec_Data == '4' || Rec_Data == '7'  ) && msg_flg == 0 ) //incase of any error,nocarrier and busy
    {
      msg_flg = 0;
      call_flag = 0;
      msg_flag_2 = 0;
      d_count = 0;
      reply_flag = 0;
    }
  }
}
/*******************************************************************************************
Function Name: send_message
*******************************************************************************************/
void send_message(void)
{
  byte msg_flg = 0;
  char rec_data = 0;
  GPSData();
  delay(1000);
  GSMmodule.print("AT+CMGS=");//Command to send message ,(AT+CMGS="+91phone number"<enter>)
  GSMmodule.print('"');
  GSMmodule.print(m_num_my);
  GSMmodule.println('"');
  delay(400);//Delay for GSM module become ready
  while (GSMmodule.available() > 0)
  {
    rec_data = GSMmodule.read();
    if ( rec_data == '>'  ) // '>'(greater than symbol) is recieved while message sending,after AT+CMGD="number" command
    {
      delay(400);//Delay for GSM module become ready
      GSMmodule.print("G_");
      GSMmodule.print(lt);
      GSMmodule.print("_");
      GSMmodule.println(lg);
      //GSMmodule.println("From - Tracky"); //Transmitting whole string to be sent as SMS
      GSMmodule.write(26);//'ctrl+z' command

      //delay(5000);//Delay for GSM module become ready
      msg_flg = 0;
    }
    else if ( rec_data == '+' && msg_flg == 0 ) // '+' is recieved
      msg_flg = 1 ;
    else if ( rec_data == 'C' && msg_flg == 1 ) // 'C' is recieved
      msg_flg = 2 ;
    else if ( rec_data == 'M' && msg_flg == 2 ) // 'M' is recieved
      msg_flg = 3 ;
    else if ( rec_data == 'G' && msg_flg == 3 ) // 'M' is recieved
      msg_flg = 4 ;
    else if ( rec_data == 'S' && msg_flg == 4 )//// 'S' is recieved after a sucessful message ("+CMSGS:  ")
      msg_flg = 5 ;
    else if ( rec_data == ':' && msg_flg == 5 )// ':' is recieved
      msg_flg = 6 ;
    else if ( rec_data == 0x0D && msg_flg == 6 )//Carriage return
      msg_flg = 7 ;
    else if ( rec_data == 0x0A && msg_flg == 7 )//Line feed
      msg_flg = 8 ;
    else if ( rec_data == '0' && msg_flg == 8 )// '0' (zero) is recieved
    {
      clear_rx_buffer();//clearing receiving buffer
      msg_flg = 0;
      msg_flag_2 = 0;
      reply_flag = 0;
    }
    else if ( rec_data == '4' && msg_flg == 0  )// '4' (Four) is recieved
    {
      clear_rx_buffer();//clearing receiving buffer
      msg_flg = 0;
      msg_flag_2 = 0;
      reply_flag = 0;
    }
    else if ( Rec_Data == '2' && msg_flg == 0 ) // '2' is recieved,incoming call
    {
      call_flag = 1;
      delay(300);
      GSMmodule.println("ATH");//Command for hang up incomming call,all calls must be filtered no need to answer a call
    }
    else if ( Rec_Data == '0' && msg_flg == 0 && call_flag == 1 ) // '0' (zero) is recieved,call sucessfully rejected
    {
      msg_flg = 0;
      call_flag = 0;
      msg_flag_2 = 0;
      d_count = 0;
      reply_flag = 0;
    }
  }
  finish = 0; pos_cnt = 0;
}
/*******************************************************************************************
Function Name: clear_rx_buffer
*******************************************************************************************/
void clear_rx_buffer(void)
{
  char rec_data = 0;
  while (GSMmodule.available() > 0)
    rec_data = GSMmodule.read();
}
/*******************************************************************************************
Function Name: array_clear
*******************************************************************************************/
void array_clear(void)
{
  byte k = 0;
  for ( k = 0; k < 15; k++)
    number[k] = '\0';
  for ( k = 0; k < 160; k++)
    message[k] = '\0';
}
/*
void ring()
{
  tone(8, 2000);
  delay(100);
  noTone(8);
  delay(100);
  tone(8, 1000);
  delay(100);
  noTone(8);
  delay(100);
}*/
