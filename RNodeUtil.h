#ifndef RNODEUTILS
#define RNODEUTILS

#include "RNSFraming.h"

void BytesToASCII(byte Out[], byte In[], uint16_t Len){
  for(int i = 0; i < Len; i++){
    Out[2*i] = HexEnum[(In[i] & 0b11110000) >> 4];
    Out[(2*i)+1] = HexEnum[(In[i] & 0b00001111)];
  }
  Out[(2*Len)] = 0;
}

void ShowSerial(char In) {
  char Buffer[5];
  char High = In >> 4;
  char Low = In & 0b00001111;
  Buffer[0] = '0';
  Buffer[1] = 'x';
  Buffer[2] = HexEnum[High];
  Buffer[3] = HexEnum[Low];
  Buffer[4] = 0x00;
  printf(Buffer);
  printf(" ");

}

void ShowSerialNeat(char In) {
  char Buffer[2];
  char High = In >> 4;
  char Low = In & 0b00001111;
  Buffer[0] = HexEnum[High];
  Buffer[1] = HexEnum[Low];
  printf("%c%c", Buffer[0], Buffer[1]);
}


uint32_t ParseCharInt(char *payload, int payloadlength) {
  uint32_t Buffer = 0;
  uint32_t A;
  for (int i = 0; i < payloadlength; i++) {
    A =  int(payload[i] - '0');
    A = A * (pow(10, payloadlength - i - 1));
    //printf("%i",A);
    Buffer += A;
  }
  return Buffer;
}


byte IntToByte(uint32_t I, int pos) {
  return (byte)(I >> (8 * pos));
}

//void ClearSerialBuffer() {
//  memset(SerialBuffer, 0x00, 500);
//}

int SetEcho(int argc, char **argv)
{

}



char CBGet() {
  char C = SerialBuffer[SerialBufferReadPointer];
  //printf("Get: 0x%02x \n",C);
  SerialBufferReadPointer++;
  if (SerialBufferReadPointer > 1023) {
    SerialBufferReadPointer = 0;
  }
  return C;
}

char CBGetE() {
  //  #define FEND            0xC0
  //  #define FESC            0xDB
  //  #define TFEND           0xDC
  //  #define TFESC           0xDD
  char C = SerialBuffer[SerialBufferReadPointer];
  if (C == FESC) {
    //printf("Escape! ");
    SerialBufferReadPointer++;
    if (SerialBufferReadPointer > 1023) {
      SerialBufferReadPointer = 0;
    }
    C = SerialBuffer[SerialBufferReadPointer];
    if (C == TFEND) {
      C = FEND;
      //printf("FEND\n");
    }

    if (C == TFESC) {
      C = FESC;
      //printf("FESC\n");
    }
  }
  //printf("Get: 0x%02x \n",C);
  SerialBufferReadPointer++;
  if (SerialBufferReadPointer > 1023) {
    SerialBufferReadPointer = 0;
  }
  return C;
}

void CBSet(char C) {
  if ((SerialBufferWritePointer + 1) % 1023 == SerialBufferReadPointer) {
    printf("ERROR: Circular Buffer Full!");
    return;
  }
  //printf("0x%02x ",C);
  SerialBuffer[SerialBufferWritePointer] = C;
  SerialBufferWritePointer++;
  if (SerialBufferWritePointer > 1023)SerialBufferWritePointer = 0;
}



#endif
