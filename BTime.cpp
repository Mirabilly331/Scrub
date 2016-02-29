#include "BTime.h"

using namespace mirabella;

int BTime::dazes[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 335};

BTime::BTime(int year, int month, int date, double seconds) : t_seconds(seconds)
{
    // Convert y/m/d to days since 01-01-2001
    t_days = (year - 2001) * 365 + (year - 2001) / 4 + dazes[month] + date - 1;
}

BTime::BTime(const BTime& b) : t_seconds(b.t_seconds), t_days(b.t_days) {  }

BTime& BTime::operator=(const BTime& rhs)
{
    if(this != &rhs)
    {
        t_seconds = rhs.t_seconds;
        t_days = rhs.t_days;
    }
    return *this;
}

bool BTime::operator==(const BTime& rhs) const
{
    return((t_days == rhs.t_days) && (t_seconds == rhs.t_seconds));
}

bool BTime::operator!=(const BTime& rhs) const
{
    return !(*this == rhs);
}

bool BTime::operator<(const BTime& rhs) const
{
    if(t_days < rhs.t_days)
        return true;

    if((t_days == rhs.t_days) && (t_seconds < rhs.t_seconds))
        return true;

    return false;
}

bool BTime::operator<=(const BTime& rhs) const
{
    return ((*this == rhs) || (*this < rhs));
}

bool BTime::operator>(const BTime& rhs) const
{
    return (rhs < *this);
}

bool BTime::operator>=(const BTime& rhs) const
{
    return (rhs <= *this);
}

int BTime::get_days() const { return t_days; }

double BTime::get_seconds() const { return t_seconds; }
