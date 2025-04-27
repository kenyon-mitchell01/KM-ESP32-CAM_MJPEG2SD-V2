#ifndef _BASE64_H
#define _BASE64_H

#include <Arduino.h>

class Base64 {
  public:
    static String encode(uint8_t * data, size_t length);
    static String encode(String text);
};

#endif