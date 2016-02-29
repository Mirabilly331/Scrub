#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <unordered_set> 
#include <algorithm>
#include <cstring>

#include "Tick.h"

using namespace mirabella;


//=================================================================
//     TickParserFunction Class
//=================================================================

void TickParserFunction::operator()(Tick& tick)
{
    const char* data = tick.get_data();
    const char* b = nullptr;

    if( false == validate_characters(data) )   
    {
        tick.set_tickstate(TickState::TICK_BAD_FORMAT);
    }
    else
    {
        b = extract_part(data, _buffer, ',');
        tick.set_timestamp(convert_time(_buffer));
        b = extract_part(b, _buffer, ',');
        tick.set_price(atof(_buffer));
        b = extract_part(b, _buffer, ',');
        tick.set_volume(atof(_buffer)); 

        tick.set_tickstate(TickState::TICK_VALID);
    }
}

TickParserFunction::TickParserFunction() : _buffer(nullptr)
{
    constexpr int BufferSize = 64;
    _buffer = new char[BufferSize];
}


TickParserFunction::~TickParserFunction()
{
    if(_buffer != nullptr)
    {
        delete [] _buffer;
        _buffer = nullptr;
    }
}

/**
  extract_part: A fast function to extract part of a buffer given a token as delimeter.

  arguments:
  s : a pointer to the buffer containing the text to be parsed.
  d : a pointer to a user allocated buffer that will contain the text on return
      this buffer will be null terminated
  token: a character that acts as delimeter

  returns:
  a pointer to the location in the text to be parsed buffer one char after the token
  this pointer may be fed back into extract_parts to repeatedly parse the buffer
  
  Buffers before call to: b = extract_parts(s, d, ":");
  _____________________________________________
 |                                             |
 |_____________________________________________|
 ^
 d
  ___________________________________________________________
  |At the tone, the time will be: 11:46 and 30 seconds.      |
  |__________________________________________________________|
   ^
   s

   Buffers before call to: b = extract_parts(s, d, ":");
  _____________________________________________
 |At the tone, the time will be'\0'            |
 |_____________________________________________|
 ^
 d
  ___________________________________________________________
  |At the tone, the time will be: 11:46 and 30 seconds.      |
  |__________________________________________________________|
  ^                              ^
  s                              b 

  The next call might be b = extract_parts(b, d, ".")
                                
**/
const char* TickParserFunction::extract_part(const char* s, char* d, char token)
{
    while(*s != '\0'){
      if(*s != token){
        *d = *s;
        d++;
        s++;
      }else{
        break;
      } 
    }
    *d = '\0';
    s++;
  
    return s;
}


void TickParserFunction::yyyymmdd(char* buffer, int& year, int& month, int& day)
{
    year = 0;
    month = 0;
    day = 0;
    char work[10];

    if(strlen(buffer) == 8)
    {
        memset(work, 0, 10);
        strncpy(work, buffer, 4);
        year = atoi(work);
        memset(work, 0, 10);
        strncpy(work, buffer + 4, 2);
        month = atoi(work);
        memset(work, 0, 10);
        strncpy(work, buffer + 6, 2);
        day = atoi(work);
    }
}


BTime TickParserFunction::convert_time(char* s)
{
    double seconds = 0.0;
    char buffer[24]; 
    int year, month, day;
    
    const char* b;

    // get the date
    b = extract_part(s, buffer, ':');
    yyyymmdd(buffer, year, month, day);

    // get the hours and convert to seconds
    b = extract_part(b, buffer, ':');
    seconds = atof(buffer) * 60 * 60; 

    // get the minutes and convert to seconds
    b = extract_part(b, buffer, ':');
    seconds += atof(buffer) * 60;

    // get the actual seconds with the fractional
    b = extract_part(b, buffer, ':');
    seconds += atof(buffer);
    
    BTime BT(year, month, day, seconds);
  
    return BT;
}

bool TickParserFunction::validate_characters(const char* line)
{
    static std::unordered_set<char> characters = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', ',', '\r'};
    const char* p = line;
    bool valid = true; 
    int commas = 0;
    int dots = 0;
    int colons = 0;

    while((*p != '\n') && (valid == true))
    {
        if( characters.end() == characters.find(*p) ) {   
            valid = false;
        }
        if(*p == ':') ++colons;
        if(*p == '.') ++dots;
        if(*p == ',') ++commas;
        ++p;
    }

    if(valid == true)
    {
        if( (dots != 2) || (commas != 2) || (colons != 3) ){
            valid = false;
        }
    }
     
    return valid;
}

bool TickTimeLessThan::operator()(const Tick& lhs, const Tick& rhs)
{
    bool rc = true;
    if( rhs.get_tickstate() != TickState::TICK_VALID) {
        rc = false;
    }
    if( lhs.get_tickstate() != TickState::TICK_VALID) {
        rc = true;
    }
    rc = ( lhs.get_timestamp() < rhs.get_timestamp());
    return rc;
}

//=================================================================
//  VALIDATORS 
//=================================================================

TickTimeValidator::TickTimeValidator()  {  }

void TickTimeValidator::operator()(Tick& tick)
{
    if(lasttime.get_days() == 0){
        lasttime = tick.get_timestamp();
        return;
    }    
    if(lasttime == tick.get_timestamp()) {
        tick.set_tickstate(TickState::TICK_INVALID_TIME);
    } else {
        lasttime = tick.get_timestamp();
    } 
}

TickPriceValidator::TickPriceValidator() : lastprice(0.0) {  };

void TickPriceValidator::operator()(Tick& tick)
{
    if(lastprice == 0.0) {
        lastprice = tick.get_price();
        return;
    }
    if(abs(lastprice - tick.get_price()) > (0.40 * lastprice)) {
        tick.set_tickstate(TickState::TICK_INVALID_PRICE);
    } else {
        lastprice = (tick.get_price());
    }
}

void TickVolumeValidator::operator()(Tick& tick)
{

}

TickValidator::TickValidator(TickTimeValidator timeValidator,
                             TickPriceValidator priceValidator,
                             TickVolumeValidator volumeValidator)
    : _timeValidator(timeValidator),
      _priceValidator(priceValidator),
      _volumeValidator(volumeValidator)
{



}

void TickValidator::operator()(Tick& T)
{
    if(T.get_tickstate() != TickState::TICK_VALID) 
        return;
    _timeValidator(T);
    if(T.get_tickstate() != TickState::TICK_VALID) 
        return;
    _priceValidator(T);

}

//=================================================================
//  Tick Class
//=================================================================
Tick::Tick(char* data)
    : _data(data), _price(0.0), _volume(0), _tickstate(TickState::TICK_VALID)
{
   TickParserFunction tpf;
   tpf(*this);
}


BTime Tick::get_timestamp() const { return _timestamp; }
void   Tick::set_timestamp(const BTime timestamp) { _timestamp = timestamp; }

double Tick::get_price() const { return _price; }
void   Tick::set_price(const double price) { _price = price; }

int    Tick::get_volume() const { return _volume; }
int    Tick::set_volume(const int volume) { _volume = volume; }

TickState    Tick::get_tickstate() const { return _tickstate; } 
void        Tick::set_tickstate(const TickState& tickstate) { _tickstate = tickstate; }

std::string  Tick::get_tickstate_str() const
{
    std::string state;
    switch(_tickstate)
    {
    case(TickState::TICK_BAD_FORMAT) : state = "TICK_BAD_FORMAT";
        break;
    case(TickState::TICK_VALID) : state = "TICK_VALID";
        break;
    case(TickState::TICK_INVALID_TIME) : state = "TICK_INVALID_TIME";
        break;
    case(TickState::TICK_INVALID_PRICE) : state = "TICK_INVALID_PRICE";
        break;
    case(TickState::TICK_INVALID_VOLUME) : state = "TICK_INVALID_VOLUME";
        break;
    };

    return state;
}

const char* Tick::get_data() const { return _data; }

std::string Tick::string_data() const 
{ 
    std::stringstream ss;
    char* p = _data;
   
    while(isprint(*p))
    {
        ss << *p;
        ++p;
    }
    ss << '\n';

    return ss.str();
}

