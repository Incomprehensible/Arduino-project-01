#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "ETHER_28J60.h"

// PIN NUMBERS
#define SS_1 10
#define SS_2 8
#define SS_3 7
#define SS_4 6
#define SS_5 5
#define SS_6 4
#define RST 9
#define SERVO 2
#define LOCK A4
#define ETH 3

#define READERS_N 6

bool access = false;

// array of all UID
// WARNING: random values for now!
byte rf_uid[][4] = {
  {0x4A, 0x87, 0xBE, 0xB9},
  {0x1B, 0x27, 0xAC, 0xC9},
  {0x5A, 0x16, 0xC8, 0x88},
  {0xAB, 0x77, 0x1C, 0x79},
  {0x4B, 0x17, 0xBC, 0x19},
  {0xA5, 0x12, 0xAC, 0x02}
}; 

// list of our slaves/readers
byte ss_n[6] = {SS_1, SS_2, SS_3, SS_4, SS_5, SS_6};

// using MFRC522 library -> creating instance of class
MFRC522 mfrc522[READERS_N];

ETHER_28J60 ethernet;

unsigned int servo_angle = 90; // current angle 
unsigned int N1 = 0; // sum of first half
unsigned int N2 = 0; // sum of second half

//num of rfid cards currently read and on the line
// reader + number assigned
uint8_t chosen[READERS_N][1] = {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};

// associative table with reader assigned to a number
// rf_uid[i] index -> assigned_number(i)
int uid_num[6] = {5, 10, 3, 9, 7, 2};
Servo servo1; //init servo

// initializing 
void setup() {
  servo1.attach(SERVO);
  
  Serial.begin(9600); // in case we need serial communication with PC
  //while (!Serial); //do nothing without serial port connection - leaving it optional now
  SPI.begin(); // init SPI bus
  for (uint8_t reader = 0; reader < READERS_N; reader++)
  {
    mfrc522[reader].PCD_Init(ss_n[reader], RST); //init each rfid
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

void loop() {

  while (!access)
  {
    if (ethernet.serviceRequest()) {
      //we got a signal to unlock the lock
      ethernet.respond();
      break;
    }
    analogWrite(LOCK, 255); // lock the lock
    //look for rfid
    delay(5000); //wait for people to put rfid in place and count ones in place
    for (uint8_t reader = 0; reader < READERS_N; reader++) {
      if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
        dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
        Serial.println();
      }
    }
    int i = 0;
    for (int x = 0; x < sizeof(rf_uid); x++) {
      while (i < mfrc522[reader].uid.size && mfrc522[reader].uid.uidByte[i] == rf_uid[x][i]))
        i++;
      if (i == mfrc522[reader].uid.size && !chosen[reader]) {
        chosen[reader] = 1; //we mark rfid as chosen if rfid is found
        chosen[reader][0] = uid_num[x]; // we assign a chosen number
        move_servo(reader);
        if (sides_equal())
        {
          access = true;
          break;
        }
      }
      else
        chosen[reader] = 0; //we mark rfid as yet unchosen
      i = 0;
    }
 } 
  analogWrite(LOCK, 0); //open the electromagnetic lock
  access = false;
}

bool sides_equal(void)
{
  if (all_chosen() && N1 == N2)
    return true;
  return false;
}

bool all_chosen(void)
{
  for (reader = 0; reader < READERS_N; reader++) {
    if (!chosen[reader])
      return (false);
  }
  return (true);
}

void  move_servo(uint8_t num)
{
  N1 = 0;
  N2 = 0;
  while (reader = 0; reader < 3; reader++) {
    if (chosen[reader])
      N1 += chosen[reader][0];
  }
  while (reader = 3; reader < READERS_N; reader++) {
    if (chosen[reader])
      N2 += chosen[reader][0];
  }
  angle -= N1 - N2;
  angle *= 3;
  if (angle > 180)
    angle %= 180; 
  servo1.write(angle);
}

// to print byte uid to the serial
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
