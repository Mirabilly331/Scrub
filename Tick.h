#pragma once
#include <iostream>
#include <vector>
#include "BTime.h"

namespace mirabella{

class Tick;

class TickParserFunction
{
public:
    TickParserFunction();
    ~TickParserFunction();
    void operator()(Tick& tick);
private:
    const char* extract_part(const char* s, char* d, char token);
    BTime convert_time(char* s);
    bool validate_characters(const char* line);
    void yyyymmdd(char* buffer, int& year, int& month, int& day);

    char* _buffer;
};

struct TickTimeLessThan
{
    bool operator()(const Tick& lhs, const Tick& rhs);
};

//============================================================================================
//           VALIDATORS
//============================================================================================

class TickTimeValidator
{
public:
    TickTimeValidator();
    void operator()(Tick& tick);
private:
    BTime lasttime;
};

class TickPriceValidator
{
public:
    TickPriceValidator();
    void operator()(Tick& tick);
private:
    double lastprice;
};

class TickVolumeValidator
{
public:
    void operator()(Tick& tick);
};

class TickValidator
{
public:

    TickValidator(TickTimeValidator timeValidator,
               TickPriceValidator priceValidator,
               TickVolumeValidator volumeValidator);
    
    void operator()(Tick& T);

private:
    TickTimeValidator    _timeValidator;
    TickPriceValidator   _priceValidator;
    TickVolumeValidator  _volumeValidator;
};

//============================================================================================
//           TICK CLASS
//============================================================================================

enum class TickState
{
    TICK_BAD_FORMAT,
    TICK_VALID,
    TICK_INVALID_TIME,
    TICK_INVALID_PRICE,
    TICK_INVALID_VOLUME,
};

class Tick
{
public:

    Tick(char* data);

    Tick() : _price(0.0), _volume(0), _tickstate(TickState::TICK_VALID), _data(nullptr) {  };

    BTime get_timestamp() const;
    void   set_timestamp(const BTime ts);

    double get_price() const;
    void   set_price(const double p);  

    int    get_volume() const;
    int    set_volume(const int vol);

    TickState    get_tickstate() const;
    std::string  get_tickstate_str() const;
    void         set_tickstate(const TickState& tickstate);

    const char*  get_data() const;
    std::string string_data() const;

private:
    BTime      _timestamp;
    double      _price;
    int         _volume;
    TickState    _tickstate;
    char*       _data;

};


}; // namespace mirabella
