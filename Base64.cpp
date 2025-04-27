#include "Base64.h"

const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

String Base64::encode(uint8_t * data, size_t length) {
  size_t i = 0, j = 0;
  size_t enc_len = 0;
  unsigned char a3[3];
  unsigned char a4[4];
  String encoded = "";

  enc_len = (length + 2) / 3 * 4;
  for(i=0; i<length; i++) {
    a3[i % 3] = data[i];
    if(i % 3 == 2) {
      a4[0] = (a3[0] & 0xfc) >> 2;
      a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
      a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
      a4[3] = (a3[2] & 0x3f);

      for(j = 0; j < 4; j++) {
        encoded += pgm_read_byte(&b64_alphabet[a4[j]]);
      }
    }
  }

  i = i % 3;
  if(i) {
    for(j = i; j < 3; j++) {
      a3[j] = '\0';
    }
    
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = (a3[2] & 0x3f);

    for(j = 0; j < i + 1; j++) {
      encoded += pgm_read_byte(&b64_alphabet[a4[j]]);
    }

    while((i++ < 3)) {
      encoded += '=';
    }
  }
  return encoded;
}

String Base64::encode(String text) {
  return encode((uint8_t*)text.c_str(), text.length());
}

// For compatibility with HTTPClient
namespace base64 {
  String encode(String text) {
    return Base64::encode(text);
  }
}