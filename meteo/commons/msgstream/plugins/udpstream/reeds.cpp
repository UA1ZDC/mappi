#include "reeds.h"

#include <iostream>
#include <vector>

using namespace std;

//using namespace RS;

namespace RS {
vector<uint8_t> RS_code_array(const char * input, uint32_t input_size)
{
  uint32_t num_of_blocks = input_size/RS::PAYLOAD + 1;

  vector<uint8_t> message, buffer;

  for(uint32_t i = 0; i < num_of_blocks - 1; i++)
  {
    buffer.assign(&input[RS::PAYLOAD*i], &input[RS::PAYLOAD*(i+1)]);

    RS::rs.encode(buffer);

    message.insert(message.end(),buffer.begin(),buffer.end());

    //cout << "coded\t" << (float)i/num_of_blocks << "\r";
  }

  buffer.assign(&input[RS::PAYLOAD*(num_of_blocks - 1)], &input[input_size]);
  buffer.resize(RS::PAYLOAD, 0x00);

  RS::rs.encode(buffer);
  message.insert(message.end(),buffer.begin(),buffer.end());

  return message;
}

pair <vector<uint8_t>, RS::decode_result> RS_decode_array(char * input, uint32_t input_size)
{
  uint32_t num_of_blocks = input_size/RS::SIZE;

  vector<uint8_t> message, buffer;

  std::vector<int> erasures = { 1 }; // Report second symbol missing
  std::vector<int> position; // And get back corrected symbols here

  uint32_t total_corrected = 0;
  int16_t corrected;
  bool success = true;

  for(uint32_t i = 0; i < num_of_blocks - 1; i++)
  {
    buffer.assign(&input[RS::SIZE*i], &input[RS::SIZE*(i+1)]);

    //corrected = rs.decode(buffer, erasures, &position );
    corrected = RS::rs.decode(buffer);
    if(corrected < 0)
    {
      success = false;

      break;
    }

    total_corrected += corrected;

    buffer.resize(RS::PAYLOAD);

    message.insert(message.end(),buffer.begin(),buffer.end());
  }

  RS::decode_result result = {success, total_corrected};

  return make_pair(message, result);
}

pair <vector<uint8_t>, RS::decode_result> RS_decode_array(char * input, uint32_t input_size, vector<pair <uint32_t, uint16_t>> error_positions, \
                                                          pair <uint16_t, uint16_t> interleave_block_size)
{
  uint32_t num_of_blocks = input_size/RS::SIZE;

  vector<uint8_t> message, buffer;

  //    std::vector<int> erasures = { 1 }; // Report second symbol missing
  //    std::vector<int> position; // And get back corrected symbols here

  uint32_t total_corrected = 0;
  int16_t corrected;

  bool success = true;
  bool corrupted = true;

  uint32_t interleave_max_period = RS::search_NOK(interleave_block_size.first, interleave_block_size.second);

  vector<pair <uint32_t, uint32_t>> error_positions_bound(error_positions.size());
  for (uint32_t k = 0; k < error_positions_bound.size(); k++)
  {
    error_positions_bound[k].first = (error_positions[k].first > interleave_max_period) ? error_positions[k].first - interleave_max_period : 0;
    error_positions_bound[k].second = error_positions[k].second + 2*interleave_max_period;
  }

  for(uint32_t i = 0; i < num_of_blocks - 1; i++)
  {
    buffer.assign(&input[RS::SIZE*i], &input[RS::SIZE*(i+1)]);

    for (vector<pair <uint32_t, uint32_t>>::const_iterator it = error_positions_bound.begin() ; it != error_positions_bound.end(); it++)
    {
      if(((RS::SIZE*i <= (it->first + it->second)) && (RS::SIZE*i >= it->first)) \
         || (((RS::SIZE*(i+1) - 1) <= (it->first + it->second)) && ((RS::SIZE*(i+1) - 1) >= it->first)))
      {
        corrupted = true;
        break;
      }
      else corrupted = false;
    }

    //corrected = rs.decode(buffer, erasures, &position );
    if(corrupted) corrected = RS::rs.decode(buffer);
    else corrected = 0;

    if(corrected < 0)
    {
      success = false;

      break;
    }

    total_corrected += corrected;

    buffer.resize(RS::PAYLOAD);

    message.insert(message.end(),buffer.begin(),buffer.end());
  }

  RS::decode_result result = {success, total_corrected};

  return make_pair(message, result);
}

vector<uint8_t> RS_turbo_code(const char * input, uint32_t input_size)
{
  const uint32_t block_size = RS_turbo::PAYLOAD.first*RS_turbo::PAYLOAD.second;

  // const uint16_t t_row_count = RS_turbo::PAYLOAD.second;
  // const uint16_t t_col_count = RS_turbo::PAYLOAD.first;

  uint32_t num_of_blocks = input_size/block_size;

  if((input_size % block_size) != 0)
    num_of_blocks++;

  vector<uint8_t> message, buffer;
  vector<vector<uint8_t>> coded_block(RS_turbo::SIZE.second, vector<uint8_t>(RS_turbo::SIZE.first));

  vector<vector<uint8_t>> transposed_block(RS_turbo::SIZE.first, vector<uint8_t>(RS_turbo::PAYLOAD.second));

  for(uint32_t i = 0; i < num_of_blocks - 1; i++)
  {
    for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
    {
      buffer.assign(&input[block_size*i + RS_turbo::PAYLOAD.first*k], &input[block_size*i + RS_turbo::PAYLOAD.first*(k + 1)]);

      RS_turbo::rs.first.encode(buffer);

      //block.insert(block.end(), buffer.begin(), buffer.end());

      for(uint16_t m = 0; m < RS_turbo::SIZE.first; m++)
      {
        transposed_block[m][k] = buffer[m];
      }
    }

    for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
    {
      RS_turbo::rs.second.encode(transposed_block[k]);

      for(uint16_t m = 0; m < RS_turbo::SIZE.second; m++)
      {
        coded_block[m][k] = transposed_block[k][m];
      }

      transposed_block[k].resize(RS_turbo::PAYLOAD.second);
    }

    for(auto k = coded_block.begin(); k < coded_block.end(); k++)
    {
      message.insert(message.end(),k->begin(),k->end());
    }
    //message.insert(message.end(),buffer.begin(),buffer.end());
  }

  vector<uint8_t> last_block(&input[block_size*(num_of_blocks - 1)], &input[input_size]);
  last_block.resize(block_size, 0x00);



  for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
  {
    buffer.assign(last_block.begin() + RS_turbo::PAYLOAD.first*k, last_block.begin() + RS_turbo::PAYLOAD.first*(k + 1));

    RS_turbo::rs.first.encode(buffer);

    //block.insert(block.end(), buffer.begin(), buffer.end());

    for(uint16_t m = 0; m < RS_turbo::SIZE.first; m++)
    {
      transposed_block[m][k] = buffer[m];
    }
  }

  for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
  {
    RS_turbo::rs.second.encode(transposed_block[k]);

    for(uint16_t m = 0; m < RS_turbo::SIZE.second; m++)
    {
      coded_block[m][k] = transposed_block[k][m];
    }
  }


  for(auto k = coded_block.begin(); k < coded_block.end(); k++)
  {
    message.insert(message.end(),k->begin(),k->end());
  }

  return message;
}

pair <vector<uint8_t>, RS::decode_result> RS_turbo_decode(char * input, uint32_t input_size)
{
  const uint32_t block_size = RS_turbo::SIZE.first*RS_turbo::SIZE.second;

  uint32_t num_of_blocks = input_size/block_size;

  vector<uint8_t> message, buffer;

  uint32_t total_corrected = 0;
  int16_t corrected;
  bool success = true;
  vector<vector<uint8_t>> coded_block(RS_turbo::SIZE.second, vector<uint8_t>(RS_turbo::SIZE.first));
  vector<vector<uint8_t>> transposed_block(RS_turbo::SIZE.first, vector<uint8_t>(RS_turbo::SIZE.second));
  for(uint32_t i = 0; i < num_of_blocks; i++)
  {
    for(uint16_t k = 0; k < RS_turbo::SIZE.second; k++)
    {
      coded_block[k].assign(&input[block_size*i + RS_turbo::SIZE.first*k], &input[block_size*i + RS_turbo::SIZE.first*(k + 1)]);
    }

    for(uint16_t m = 0; m < RS_turbo::SIZE.first; m++)
    {
      for(uint16_t k = 0; k < RS_turbo::SIZE.second; k++)
      {
        transposed_block[k][m] = coded_block[m][k];
      }
    }

    for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
    {
      corrected = RS_turbo::rs.second.decode(transposed_block[k]);
      if(corrected > 0) total_corrected += corrected;
    }

    for(uint16_t m = 0; m < RS_turbo::SIZE.second; m++)
    {
      for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
      {
        coded_block[k][m] = transposed_block[m][k];
      }
    }

    for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
    {
      corrected = RS_turbo::rs.first.decode(coded_block[k]);
      if(corrected < 0)
      {
        success = false;
        break;
      }
      else
      {
        total_corrected += corrected;
      }

      coded_block[k].resize(RS_turbo::PAYLOAD.first);
    }

    for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
    {
      message.insert(message.end(),coded_block[k].begin(),coded_block[k].end());
    }
  }

  RS::decode_result result = {success, total_corrected};

  return make_pair(message, result);
}

pair <vector<uint8_t>, RS::decode_result> RS_turbo_decode(char * input, uint32_t input_size, vector<pair <uint32_t, uint16_t>> error_positions )
{
  const uint32_t block_size = RS_turbo::SIZE.first*RS_turbo::SIZE.second;

  uint32_t num_of_blocks = input_size/block_size;

  vector<uint8_t> message, buffer;

  uint32_t total_corrected = 0;
  int16_t corrected;

  bool success = true;
  bool corrupted = true;

  vector<vector<uint8_t>> coded_block(RS_turbo::SIZE.second, vector<uint8_t>(RS_turbo::SIZE.first));
  vector<vector<uint8_t>> transposed_block(RS_turbo::SIZE.first, vector<uint8_t>(RS_turbo::SIZE.second));


  for(uint32_t i = 0; i < num_of_blocks; i++)
  {
    for (vector<pair <uint32_t, uint16_t>>::const_iterator it = error_positions.begin() ; it != error_positions.end(); it++)
    {
      if(((block_size*i <= (it->first + it->second)) && (block_size*i >= it->first)) \
         || (((block_size*(i+1) - 1) <= (it->first + it->second)) && ((block_size*(i+1) - 1) >= it->first)))
      {
        corrupted = true;
        break;
      }
      else corrupted = false;

      if(((it->first < block_size*(i + 1)) && (it->first >= block_size*i)) \
         || (((it->first + it->second) < block_size*(i + 1)) && ((it->first + it->second) >= block_size*i)))
      {
        corrupted = true;
        break;
      }
      else corrupted = false;
    }

    for(uint16_t k = 0; k < RS_turbo::SIZE.second; k++)
    {
      coded_block[k].assign(&input[block_size*i + RS_turbo::SIZE.first*k], &input[block_size*i + RS_turbo::SIZE.first*(k + 1)]);
    }

    for(uint16_t m = 0; m < RS_turbo::SIZE.first; m++)
    {
      for(uint16_t k = 0; k < RS_turbo::SIZE.second; k++)
      {
        transposed_block[k][m] = coded_block[m][k];
      }
    }

    if(corrupted)
      for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
      {
        corrected = RS_turbo::rs.second.decode(transposed_block[k]);
        if(corrected > 0) total_corrected += corrected;
      }

    for(uint16_t m = 0; m < RS_turbo::SIZE.second; m++)
    {
      for(uint16_t k = 0; k < RS_turbo::SIZE.first; k++)
      {
        coded_block[k][m] = transposed_block[m][k];
      }
    }

    for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
    {
      if(corrupted)
      {
        corrected = RS_turbo::rs.first.decode(coded_block[k]);
        if(corrected < 0)
        {
          success = false;
          break;
        }
        else
        {
          total_corrected += corrected;
        }
      }

      coded_block[k].resize(RS_turbo::PAYLOAD.first);
    }

    for(uint16_t k = 0; k < RS_turbo::PAYLOAD.second; k++)
    {
      message.insert(message.end(),coded_block[k].begin(),coded_block[k].end());
    }
  }

  RS::decode_result result = {success, total_corrected};

  return make_pair(message, result);
}

uint32_t search_NOK (uint16_t x, uint16_t y)
{
  uint16_t count = (x > y) ? x : y;

  for (uint32_t i = count; i < uint32_t(x * y); i++)
    if (!(i % x) && !(i % y)) return i;

  return x * y;
}
}
