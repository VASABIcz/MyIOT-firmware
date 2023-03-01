//
// Created by vasabi on 1.3.23.
//

#ifndef EMBEDED_XD_UTIL_H
#define EMBEDED_XD_UTIL_H

bool isBigEndian()   {
    unsigned int x = 1;
    return (bool)(((char *)&x)[0]);
}

int reverseBytes(int num) {
    int b = 0;
    ((byte*)&b)[3] = (uint8_t) (num >>  0u);
    ((byte*)&b)[2] = (uint8_t) (num >>  8u);
    ((byte*)&b)[1] = (uint8_t) (num >> 16u);
    ((byte*)&b)[0] = (uint8_t) (num >> 24u);
    return b;
}

#endif //EMBEDED_XD_UTIL_H
