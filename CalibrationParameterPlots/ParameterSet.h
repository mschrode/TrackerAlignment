#ifndef PARAMETER_SET_H
#define PARAMETER_SET_H

#include <exception>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "Detector.h"
#include "IOV.h"


enum CalibrationParameterType { NONE=-1, PixelLA, StripLADeco, StripLAPeak, StripBPDeco };


class GranularityBin {
public:
  GranularityBin()
    : firstUnit_(-1), lastUnit_(-1) {}
  GranularityBin(const int first, const int last)
    : firstUnit_(first), lastUnit_(last) {
    if( lastUnit_ < firstUnit_ ) {
      std::cerr << "\n\nERROR creating GranularityBin: last < first\n" << std::endl;
      throw std::exception();
    }
  }

  bool operator==(const GranularityBin& other) const {
    return firstUnit_ == other.firstUnit_ && lastUnit_ == other.lastUnit_;
  }
  bool operator<(const GranularityBin& other) const {
    return lastUnit_ < other.firstUnit_;
  }

  int firstUnit() const { return firstUnit_; }
  int lastUnit() const { return lastUnit_; }


private:
  int firstUnit_;
  int lastUnit_;
};


class GranularityElement {
public:
  GranularityElement()
    : zBin_(-1,-1), rBin_(-1,-1) {}
  GranularityElement(const GranularityBin& theZBin, const GranularityBin& theRBin)
    : zBin_(theZBin), rBin_(theRBin) {}

  bool operator<(const GranularityElement& other) const {
    if( zBin_ < other.zBin_ ) {
      return true;
    } else if( zBin_ == other.zBin_ ) {
      return rBin_ < other.rBin_;
    } else {
      return false;
    }
  }

  unsigned int zMin() const { return static_cast<unsigned int>(zBin_.firstUnit()); }
  unsigned int zMax() const { return static_cast<unsigned int>(zBin_.lastUnit()); }
  unsigned int rMin() const { return static_cast<unsigned int>(rBin_.firstUnit()); }
  unsigned int rMax() const { return static_cast<unsigned int>(rBin_.lastUnit()); }


private:
  GranularityBin zBin_;
  GranularityBin rBin_;
};


class Parameter {
public:
  Parameter()
    : origIndex_(-1) {}
  Parameter(const IOV& theIOV, const double theValue, const double theDelta, const double theError, const int theOrigIdx)
    : origIndex_(theOrigIdx) {
    std::vector<double> pars;
    pars.push_back(theValue);
    pars.push_back(theDelta);
    pars.push_back(theError);
    valuesPerIOV_[theIOV] = pars;
  }

  void addValue(const IOV& theIOV, const double theValue, const double theDelta, const double theError) {
    std::vector<double> pars;
    pars.push_back(theValue);
    pars.push_back(theDelta);
    pars.push_back(theError);
    valuesPerIOV_[theIOV] = pars;
  }

  unsigned int nIOVs() const { return valuesPerIOV_.size(); }
  double value(const IOV& theIOV) const;
  double delta(const IOV& theIOV) const;
  double error(const IOV& theIOV) const;


private:
  int origIndex_;
  std::map< IOV, std::vector<double> > valuesPerIOV_; // [value,delta,error]
};


double Parameter::value(const IOV& iov) const {
  std::map< IOV, std::vector<double> >::const_iterator it = valuesPerIOV_.find(iov);
  if( it == valuesPerIOV_.end() ) {
    std::cerr << "\n\nERROR no parameter stored for IOV " << iov() << "\n" << std::endl;
    throw std::exception();
  }

  return it->second.at(0);
}


double Parameter::delta(const IOV& iov) const {
  std::map< IOV, std::vector<double> >::const_iterator it = valuesPerIOV_.find(iov);
  if( it == valuesPerIOV_.end() ) {
    std::cerr << "\n\nERROR no parameter stored for IOV " << iov() << "\n" << std::endl;
    throw std::exception();
  }

  return it->second.at(1);
}


double Parameter::error(const IOV& iov) const {
  std::map< IOV, std::vector<double> >::const_iterator it = valuesPerIOV_.find(iov);
  if( it == valuesPerIOV_.end() ) {
    std::cerr << "\n\nERROR no parameter stored for IOV " << iov() << "\n" << std::endl;
    throw std::exception();
  }

  return it->second.at(2);
}


class ParameterSet {
public:
  ParameterSet()
    : type_(NONE), det_(UNKNOWN) {}
  ParameterSet(const CalibrationParameterType theType, const Detector theDetector)
    : type_(theType), det_(theDetector) {}

  void add(const unsigned int theZBinMin, const unsigned int theZBinMax,
	   const unsigned int theRBinMin, const unsigned int theRBinMax,
	   const IOV& iov,
	   const double value, const double delta, const double parDelta,
	   const int origParamIndex);

  CalibrationParameterType type() const { return type_; }
  Detector detector() const { return det_; }
  bool empty() const { return pars_.size() == 0; }
  unsigned int nZBins() const { return zBins_.size(); }
  unsigned int nRBins() const { return rBins_.size(); }
  unsigned int nIOVs() const { return iovs_.size(); }
  IOVIt IOVsBegin() const { return iovs_.begin(); }
  IOVIt IOVsEnd() const { return iovs_.end(); }
  double value(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const;
  double delta(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const;
  double error(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const;
  double zBinMin(const unsigned int zBin) const { return getBin(zBin,zBins_).firstUnit(); }
  double zBinMax(const unsigned int zBin) const { return getBin(zBin,zBins_).lastUnit(); }
  void print() const;


private:
  typedef std::map<GranularityElement,Parameter> ParMap;
  typedef std::map<GranularityElement,Parameter>::iterator ParMapIt;
  typedef std::map<GranularityElement,Parameter>::const_iterator ParMapConstIt;

  CalibrationParameterType type_;
  Detector det_;
  ParMap pars_;
  std::set<GranularityBin> zBins_;
  std::set<GranularityBin> rBins_;
  std::set<IOV> iovs_;

  GranularityElement getGranularityElement(const unsigned int zBin, const unsigned int rBin) const;
  GranularityBin getBin(const unsigned int idx, const std::set<GranularityBin>& bins) const;
  IOV getIOV(const unsigned int iov, const std::set<IOV>& iovs) const;
};


void ParameterSet::add(const unsigned int theZBinMin, const unsigned int theZBinMax,
		       const unsigned int theRBinMin, const unsigned int theRBinMax,
		       const IOV& iov,
		       const double parValue, const double parDelta, const double parError,
		       const int origParamIndex) {

  GranularityBin zBin(theZBinMin,theZBinMax);
  GranularityBin rBin(theRBinMin,theRBinMax);
  GranularityElement ge(zBin,rBin);
  ParMapIt it = pars_.find(ge);
  if( it != pars_.end() ) {	// update information
    it->second.addValue(iov,parValue,parDelta,parError);
    iovs_.insert(iov);
  } else {			// add new set
    zBins_.insert(zBin);
    rBins_.insert(rBin);
    iovs_.insert(iov);
    pars_[ge] = Parameter(iov,parValue,parDelta,parError,origParamIndex);
  }
}


double ParameterSet::value(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const {
  ParMapConstIt it = pars_.find(getGranularityElement(zBin,rBin));
  
  return it->second.value(getIOV(iov,iovs_));
}


double ParameterSet::delta(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const {
  ParMapConstIt it = pars_.find(getGranularityElement(zBin,rBin));
  
  return it->second.delta(getIOV(iov,iovs_));
}


double ParameterSet::error(const unsigned int zBin, const unsigned int rBin, const unsigned int iov) const {
  ParMapConstIt it = pars_.find(getGranularityElement(zBin,rBin));
  
  return it->second.error(getIOV(iov,iovs_));
}


GranularityElement ParameterSet::getGranularityElement(const unsigned int zBin, const unsigned int rBin) const {
  GranularityBin granularityZBin = getBin(zBin,zBins_);
  GranularityBin granularityRBin = getBin(rBin,rBins_);
  
  return GranularityElement(granularityZBin,granularityRBin);
}


GranularityBin ParameterSet::getBin(const unsigned int idx, const std::set<GranularityBin>& bins) const {
  unsigned int counter = 0;
  for(std::set<GranularityBin>::const_iterator it = bins.begin();
      it != bins.end(); ++it, ++counter) {
    if( counter == idx ) return *it;
  }
  std::cerr << "\n\nERROR trying to access bin outside range\n" << std::endl;
  throw std::exception();

  return GranularityBin();
}


IOV ParameterSet::getIOV(const unsigned int iov, const std::set<IOV>& iovs) const {
  unsigned int counter = 0;
  for(IOVIt it = iovs.begin(); it != iovs.end(); ++it, ++counter) {
    if( counter == iov ) return *it;
  }
  std::cerr << "\n\nERROR trying to access IOV outside range\n" << std::endl;
  throw std::exception();
  
  return IOV();
}


void ParameterSet::print() const {
  std::cout << pars_.size() << " " << toStr(det_) << " parameters:" << std::endl;
  for(ParMapConstIt it = pars_.begin(); it != pars_.end(); ++it) {
    const unsigned int zMin = it->first.zMin();
    const unsigned int zMax = it->first.zMax();
    const unsigned int rMin = it->first.rMin();
    const unsigned int rMax = it->first.rMax();
    // char txt[50];
    // printf(txt," %3u : %6.3f\n",i,pars_.at(i));
    std::cout << "  z: " << std::flush;
    if( zMin == zMax ) std::cout << zMin << std::flush;
    else               std::cout << zMin << "-" << zMax << std::flush;
    std::cout << ", r: " << std::flush;
    if( rMin == rMax ) std::cout << rMin << std::endl;
    else               std::cout << rMin << "-" << rMax << std::endl;
    for(size_t i = 0; i < it->second.nIOVs(); ++i) {
      std::cout << "    IOV " << i << ": " << it->second.value(getIOV(i,iovs_)) << std::endl;
    }
  }
}

#endif
