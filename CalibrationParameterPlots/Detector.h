#ifndef DETECTOR_H
#define DETECTOR_H

#include <exception>
#include <iostream>
#include <map>

#include "TFile.h"
#include "TString.h"
#include "TTree.h"


enum Detector { UNKNOWN=-1, BPIX, FPIX, TIB, TID, TOB, TEC };

TString toStr(Detector det) {
  if( det == BPIX ) return "BPIX";
  if( det == FPIX ) return "FPIX";
  if( det == TIB  ) return "TIB";
  if( det == TID  ) return "TID";
  if( det == TOB  ) return "TOB";
  if( det == TEC  ) return "TEC";
  return "UNKNOWN"; 
}


class Tracker {
private:
  struct SensorInfo {
    SensorInfo()
      : det(UNKNOWN), layer(999999), ring(999999) {}
    SensorInfo(const Detector theDet, const unsigned int theLayer, const unsigned theRing)
      : det(theDet), layer(theLayer), ring(theRing) {}

    Detector det;
    unsigned int layer;
    unsigned int ring;
  };
  
  typedef std::map<unsigned int,SensorInfo> Sensors;
  typedef std::map<unsigned int,SensorInfo>::const_iterator SensorIt;


public:
  Tracker() {}
  Tracker(const TString& fileName) { initCMS(fileName); }

  Detector detector(const unsigned int id) const { return findSensor(id)->second.det; }

  // ring refers to units along global z
  unsigned int ring(const unsigned int id) const { return findSensor(id)->second.ring; }

  // layer refers to units along global r
  unsigned int layer(const unsigned int id) const { return findSensor(id)->second.layer; }


private:
  Sensors sensors_;

  void initCMS(const TString& fileName);
  SensorIt findSensor(const unsigned int id) const;
};


Tracker::SensorIt Tracker::findSensor(const unsigned int id) const {
  SensorIt it = sensors_.find(id);
  if( it == sensors_.end() ) {
    std::cerr << "\n\nERROR in Tracker: trying to access unknown sensor '" << id << "'\n" << std::endl;
    throw std::exception();
  }
  
  return it;
}


// dimensions in cm
void Tracker::initCMS(const TString& fileName) {
  std::cout << "Initialising CMS" << std::endl;
    
  // open file with tracker info
  TFile file(fileName,"READ");
  if( !file.IsOpen() ) {
    std::cerr << "\n\nERROR opening file '" << fileName << "'\n" << std::endl;
    throw std::exception();
  }

  TTree* tree = 0;
  const TString treeName = "TrackerTreeGenerator/TrackerTree/TrackerTree";
  file.GetObject(treeName,tree);
  if( tree == 0 ) {
    std::cerr << "\n\nERROR reading tree '" << treeName << "' from file '" << fileName << "'\n" << std::endl;
    throw std::exception();
  }
  
  // tree variables
  unsigned int theSensorId = 0;
  unsigned int theDetId = 0;
  unsigned int theLayer = 0;
  unsigned int theSide = 0;
  unsigned int theModule = 0;
  tree->SetBranchAddress("RawId",&theSensorId);
  tree->SetBranchAddress("SubdetId",&theDetId);
  tree->SetBranchAddress("Layer",&theLayer);
  tree->SetBranchAddress("Side",&theSide);
  tree->SetBranchAddress("Module",&theModule);

  for(int iE = 0; iE < tree->GetEntries(); ++iE) {
    tree->GetEntry(iE);
    
    // the detector encoding in tree
    // BPIX: 1
    // FPIX: 2
    // TIB:  3
    // TID:  4
    // TOB:  5
    // TEC:  6
    Detector theDet = UNKNOWN;
    if(      theDetId == 1 ) theDet = BPIX;
    else if( theDetId == 2 ) theDet = FPIX;
    else if( theDetId == 3 ) theDet = TIB;
    else if( theDetId == 5 ) theDet = TOB;

    // in this script, 'ring' refers to units along z
    unsigned int theRing = 999999;	// so far, only for BPIX, TIB and TOB
    if( theDet == BPIX ) {
      // translate 'module' index into rings
      theRing = theModule-1;
      theLayer = theLayer-1;

    } else if( theDet == FPIX ) {
      // translate 'side' index into rings
      theRing = theSide==1 ? 0 : 1;
      theLayer = 0;

    } else if( theDet == TIB ) {
      // translate 'side' and 'module' indices into rings
      // side   : 1 for -z, 2 for +x
      // module : 1,2,3 for rings from z=0 to +/-z
      if(       theSide == 1 ) theRing = 3-theModule;
      else if ( theSide == 2 ) theRing = 2+theModule;
      theLayer = theLayer-1;

    } else if( theDet == TOB ) {
      // translate 'side' and 'module' indices into rings
      // side   : 1 for -z, 2 for +x
      // module : 1,2,3,4,5,6 for rings from z=0 to +/-z
      if(       theSide == 1 ) theRing = 6-theModule;
      else if ( theSide == 2 ) theRing = 5+theModule;
      theLayer = theLayer-1;
    }    

    sensors_[theSensorId] = SensorInfo(theDet,theLayer,theRing);
  }

  delete tree;
  file.Close();
}
#endif
