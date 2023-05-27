#ifndef REEDS_H
#define REEDS_H


#include <iostream>
#include <vector>

#include "rs.h"

using namespace std;

namespace RS {
const uint32_t SIZE = 255;    //2^symbol_size - 1;
const uint16_t PARITY = 48;

const uint32_t PAYLOAD = SIZE - PARITY;

const ezpwd::RS<SIZE,PAYLOAD> rs;

struct decode_result {
    bool success;
    uint32_t total_corrected;
};
}

namespace RS_turbo {
const pair<uint32_t, uint32_t> SIZE = {255, 255};    //2^symbol_size - 1;
const pair<uint32_t, uint32_t> PARITY = {48, 64};

const pair<uint32_t, uint32_t> PAYLOAD = {SIZE.first - PARITY.first, SIZE.second - PARITY.second};

const pair<ezpwd::RS<255,207>, ezpwd::RS<255,191>> rs;

struct decode_result {
    bool success;
    uint32_t total_corrected;
};
}

namespace RS {
vector<uint8_t> RS_code_array(const char *input, uint32_t input_size);

pair <vector<uint8_t>, RS::decode_result> RS_decode_array(char * input, uint32_t input_size);

pair <vector<uint8_t>, RS::decode_result> RS_decode_array(char * input, uint32_t input_size, vector<pair <uint32_t, uint16_t>> error_positions, \
                                                          pair <uint16_t, uint16_t> interleave_block_size);

vector<uint8_t> RS_turbo_code(const char * input, uint32_t input_size);

pair <vector<uint8_t>, RS::decode_result> RS_turbo_decode(char * input, uint32_t input_size);

pair <vector<uint8_t>, RS::decode_result> RS_turbo_decode(char * input, uint32_t input_size, vector<pair <uint32_t, uint16_t>> error_positions);

uint32_t search_NOK (uint16_t x, uint16_t y);
}
#endif // RS_H
