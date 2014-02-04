#include <cmath>
#include <exception>
#include <iostream>
#include <vector>

#include "TFile.h"
#include "TTree.h"


std::vector<UInt_t> getList(const TString& fileName, const unsigned int iov) {
  if( iov == 0 ) {
    std::cerr << "\n\nERROR: IOV numbering starts with 1\n\n" << std::endl;
    throw std::exception();
  }
  TFile file(fileName,"READ");
  TString treeName("MillePedeUser_");
  treeName += iov;
  TTree* mpt = NULL;
  file.GetObject(treeName,mpt);
  if( mpt == NULL ) {
    std::cerr << "\n\nERROR reading TTree '" << treeName << "' from file '" << fileName << "'\n\n" << std::endl;
    throw std::exception();
  }
  
  std::vector<UInt_t> list;

  // define variables in TTree
  const UInt_t numParMax = 20;
  UInt_t Id = 0;
  Int_t ObjId = 0;
  UInt_t HitsX = 0;
  UInt_t NumPar = 0;
  Float_t Par[numParMax];
  Float_t DiffBefore[numParMax];

  mpt->SetBranchAddress("Id",&Id);
  mpt->SetBranchAddress("ObjId",&ObjId);
  mpt->SetBranchAddress("HitsX",&HitsX);
  mpt->SetBranchAddress("NumPar",&NumPar);
  mpt->SetBranchAddress("Par",Par);
  mpt->SetBranchAddress("DiffBefore",DiffBefore);

  // Loop over tree entries
  for(int entry = 0; entry < mpt->GetEntries(); ++entry) {
    mpt->GetEntry(entry);
    if( NumPar > numParMax ) {
      std::cerr << "\n\nERROR NumPar = " << NumPar << " > " << numParMax << "\n\n" << std::endl;
      throw std::exception();
    }

    // consider only DetUnits
    if( ObjId != 1 ) continue;

    // check parameter value returned by mille-pede
    bool isUnchangedPar = true;
    for(UInt_t i = 0; i < NumPar; ++i) {
      if( !( std::abs(Par[i]) < 1E-12 || Par[i] < -999990 ) ) {
	isUnchangedPar = false;
	break;
      }
    }

    if( isUnchangedPar ) {
      list.push_back( Id );
      //std::cout << Id << ":" << HitsX << std::endl;
    }
    
  } // End of loop over tree entries

  return list;
}


void getListOfExcludedAlignables(const TString& fileName) {
  std::vector<UInt_t> list = getList(fileName,1);
  std::cout << "Ids of unchanged alignables:" << std::endl;
  for(std::vector<UInt_t>::const_iterator it = list.begin();
      it != list.end(); ++it) {
    std::cout << "  " << *it << std::endl;
  }
}
