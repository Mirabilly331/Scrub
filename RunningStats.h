#pragma once

/**
  Note of Attributions:
     This code was created by John D. Cook and enhanced to calculate skewness
  and kurtosis but John Myles White.  It has been used verbatum.  The original
  code can be found at http://www.johndcook.com/blog/skewness_kurtosis.

  Quoting the description from the website:
  The code is an extension of the method of Knuth and Welford for computing
  standard deviation, skewness and kurtosis in one pass.  
  In addition to only requiring one pass through the data, the algorithm 
  is numerically stable and accurate.

  It should be clearly understood that Billy makes no claim to the
  authorship of this code.
**/


class RunningStats
{
public:
    RunningStats();
    void Clear();
    void Push(double x);
    long long NumDataValues() const;
    double Mean() const;
    double Variance() const;
    double StandardDeviation() const;
    double Skewness() const;
    double Kurtosis() const;
 
    friend RunningStats operator+(const RunningStats a, const RunningStats b);
    RunningStats& operator+=(const RunningStats &rhs);
 
private:
    long long n;
    double M1, M2, M3, M4;
};
 
