#pragma once

namespace mirabella {

//stores time in days + seconds since 12:00:00 AM Jan 1st 2001
class BTime {
public:
    BTime() : t_days(0), t_seconds(0) {} ;
    BTime(int year, int month, int date, double seconds);
    BTime(const BTime& b);
    BTime& operator=(const BTime& rhs);

    bool operator==(const BTime& rhs) const;
    bool operator!=(const BTime& rhs) const;
    bool operator<(const BTime& rhs) const;
    bool operator<=(const BTime& rhs) const;
    bool operator>(const BTime& rhs) const;
    bool operator>=(const BTime& rhs) const;

    int get_days() const;
    double get_seconds() const;

private:
    int    t_days;
    double t_seconds;
    static int dazes[];


};
 

}  // namespace mirabella
