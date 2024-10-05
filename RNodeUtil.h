#ifndef RNODEUTILS
#define RNODEUTILS

void BytesToASCII(byte Out[], byte In[], uint16_t Len){
  for(int i = 0; i < Len; i++){
    Out[2*i] = HexEnum[(In[i] & 0b11110000) >> 4];
    Out[(2*i)+1] = HexEnum[(In[i] & 0b00001111)];
  }
}

#endif
