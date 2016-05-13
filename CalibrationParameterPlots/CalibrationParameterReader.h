#ifndef CALIBRATION_PARAMETER_READER_H
#define CALIBRATION_PARAMETER_READER_H

#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "TDirectory.h"
#include "TFile.h"
#include "TKey.h"
#include "TList.h"
#include "TString.h"
#include "TTree.h"

#include "Detector.h"
#include "IOV.h"
#include "ParameterSet.h"


class CalibrationParameterReader {
public:
  CalibrationParameterReader(const Tracker* tracker)
    : tracker_(tracker) { }

  std::map<Detector,ParameterSet> read(const CalibrationParameterType type, const TString& fileName) const;


private:
  struct TreeInfo {
    TreeInfo()
      : name(""), iov(IOV()) {}
    TreeInfo(const TString& theName, const unsigned int min, const unsigned int max)
      : name(theName), iov(IOV(min,max)) {}

    TString name;
    IOV iov;
  };

  struct ParInfo {
    ParInfo()
      : det(UNKNOWN), value(9999999.), delta(999999.), error(9999999.) {}
    ParInfo(const Detector detector, const double val, const double delt, const double err,
	    const unsigned int ring, const unsigned int layer)
      : det(detector), value(val), delta(delt), error(err) {
      rings.insert(ring);
      layers.insert(layer);
    }
    
    Detector det;
    double value;
    double delta;
    double error;
    std::set<unsigned int> rings;
    std::set<unsigned int> layers;
  };

  const Tracker* tracker_;

  std::vector<TreeInfo> getTreeInfo(const CalibrationParameterType type, TFile& file) const;
};


std::vector<CalibrationParameterReader::TreeInfo> CalibrationParameterReader::getTreeInfo(const CalibrationParameterType type, TFile& file) const {

  if( type == NONE ) return std::vector<TreeInfo>(0);


  TString treeBaseName("");
  if(      type == PixelLA     )
    treeBaseName = "SiPixelLorentzAngleCalibration_result_";
  else if( type == StripLADeco )
    treeBaseName = "SiStripLorentzAngleCalibration_deconvolution_result_";
  else if( type == StripLAPeak )
    treeBaseName = "SiStripLorentzAngleCalibration_peak_result_";
  else if( type == StripBPDeco )
    treeBaseName = "SiStripBackplaneCalibration_deconvolution_result_";

  std::vector<TreeInfo> treeInfos;
  TIter nextkey( file.GetListOfKeys() );
  TKey* key = 0;
  while( ( key = (TKey*)nextkey() ) ) {
    TString name( key->GetName() );
    if( name.BeginsWith(treeBaseName) ) {
      name.ReplaceAll(treeBaseName,"");
      if( name.IsDigit() && name.Atoi() > 0 ) {
	const unsigned int min = static_cast<unsigned int>(name.Atoi());
	treeInfos.push_back(TreeInfo(treeBaseName+name,min,9999999));
      } else {
	std::cerr << "\n\nERROR reading tree '" << key->GetName() << std::endl;
	std::cout << "when looking for all IOVs of '" << treeBaseName << "'\n" << std::endl;
	throw std::exception();
      }
    }
  }
  for(size_t i = 0; i < treeInfos.size()-1; ++i) {
    treeInfos.at(i).iov = IOV(treeInfos.at(i).iov.minRun(),treeInfos.at(i+1).iov.minRun()-1);
  }
  treeInfos.pop_back();		// don't need last tree (I think)

  std::cout << "Found the following IOVs" << std::endl;
  for(size_t i = 0; i < treeInfos.size(); ++i) {
    std::cout << treeInfos.at(i).name << ": " << treeInfos.at(i).iov.minRun() << " - " << treeInfos.at(i).iov.maxRun() << std::endl;
  }


  return treeInfos;
}
  

std::map<Detector,ParameterSet> CalibrationParameterReader::read(const CalibrationParameterType type, const TString& fileName) const {

  // open file with alignment results
  TFile file(fileName,"READ");
  if( !file.IsOpen() ) {
    std::cerr << "\n\nERROR opening file '" << fileName << "'\n" << std::endl;
    throw std::exception();
  }

  // get name of all trees of this base name for different IOVs
  std::vector<TreeInfo> treeInfoPerIOV = getTreeInfo(type,file);

  // the result: parameters for all detectors and IOVs
  std::map<Detector,ParameterSet> result;

  // loop over IOVs
  for(std::vector<TreeInfo>::const_iterator iovIt = treeInfoPerIOV.begin();
      iovIt != treeInfoPerIOV.end(); ++iovIt) {
    
    // get tree for this IOV
    TTree* tree = 0;
    file.GetObject(iovIt->name,tree);
    if( tree == 0 ) {
      std::cerr << "\n\nERROR reading tree '" << iovIt->name << "' from file\n" << std::endl;
      throw std::exception();
    }

    // helper object to temporarily store parameters and granularity info
    std::map<int,ParInfo> values;

    // tree variables
    unsigned int id = 0;
    float value = 0.;
    struct treeStruct {
      float delta;
      float error;
      int parIdx;
    } results;
    tree->SetBranchAddress("detId",&id);
    tree->SetBranchAddress("value",&value);
    tree->SetBranchAddress("treeStruct",&results);

    // loop over tree (modules)
    for(int iE = 0; iE < tree->GetEntries(); ++iE) {
      tree->GetEntry(iE);

      if( results.parIdx > -1 ) {	// parIdx == -1 for modules withouth LA/BP calibration parameters

	// detector and module information
	const Detector det = tracker_->detector(id);
	const unsigned int ring = tracker_->ring(id);
	const unsigned int layer = tracker_->layer(id);

	// store in temporary map
	std::map<int,ParInfo>::iterator valueIt = values.find(results.parIdx);
	if( valueIt == values.end() ) {
	  values[results.parIdx] = ParInfo(det,value,results.delta,results.error,ring,layer);
	} else {
	  valueIt->second.rings.insert(ring);
	  valueIt->second.layers.insert(layer);
	}

	// create an entry in result for this detector
	if( result.find(det) == result.end() ) result[det] = ParameterSet(type,det);

      }
    
    } // end of loop over tree
    delete tree;

    // store results
    for(std::map<int,ParInfo>::const_iterator valIt = values.begin();
	valIt != values.end(); ++valIt) {
      const int origParIdx = valIt->first;
      const ParInfo& pi = valIt->second;
      const unsigned int minZIdx = *std::min_element(pi.rings.begin(),pi.rings.end());
      const unsigned int maxZIdx = *std::max_element(pi.rings.begin(),pi.rings.end());
      const unsigned int minRIdx = *std::min_element(pi.layers.begin(),pi.layers.end());
      const unsigned int maxRIdx = *std::max_element(pi.layers.begin(),pi.layers.end());

      // std::cout << "Adding result:" << std::endl;
      // std::cout << "  zIdx: " << minZIdx << " - " << maxZIdx << std::endl;
      // std::cout << "  rIdx: " << minRIdx << " - " << maxRIdx << std::endl;
      // std::cout << "   val: " << pi.value << std::endl;
      // std::cout << "  orig: " << origParIdx << std::endl;

      result[pi.det].add(minZIdx,maxZIdx,minRIdx,maxRIdx,iovIt->iov,pi.value,pi.delta,pi.error,origParIdx);
    } // end of loop over stored values

  } // end of loop over IOVs

  file.Close();

  return result;
}
#endif
