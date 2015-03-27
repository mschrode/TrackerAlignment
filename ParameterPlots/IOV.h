#ifndef IOV_H
#define IOV_H

#include <exception>
#include <iostream>
#include <set>


class IOV {
public:
  IOV()
    : runMin_(0), runMax_(0) {}
  IOV(const unsigned int theMinRun, const unsigned int theMaxRun)
    : runMin_(theMinRun), runMax_(theMaxRun) {
    if( runMin_ > runMax_ ) {
      std::cerr << "\n\nERROR initialising IOV with MIN(=" << runMin_ << ") > MAX(=" << runMax_ << ")\n" << std::endl;
      throw std::exception();
    }
   }

  bool operator==(const IOV& other) const {
    return runMin_ == other.runMin_ && runMax_ == other.runMax_;
  }
  bool operator!=(const IOV& other) const {
    return !( (*this) == other );
  }

  // for orderint in STL containers
  bool operator<(const IOV& other) const {
    return runMax_ < other.runMin_;
  }
  bool operator>(const IOV& other) const {
    return other < (*this);
  }

  unsigned int minRun() const { return runMin_; }
  unsigned int maxRun() const { return runMax_; }

  TString operator()() const {
    TString str("");
    str += runMin_;
    str += "-";
    str += runMax_;

    return str;
  }


private:
  unsigned int runMin_;
  unsigned int runMax_;
};

typedef std::set<IOV>::const_iterator IOVIt;
#endif
