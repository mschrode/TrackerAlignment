#ifndef GEOMETRY_COMPARISON_H
#define GEOMETRY_COMPARISON_H

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TColor.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH1D.h"
#include "TString.h"
#include "TTree.h"

#include "Variable.h"


class GeometryComparison {
public:
  GeometryComparison(const TString &fileName, const TString &id);

  void excludeModules(const TString& fileName);

  void draw(const TString &vars, double min = 1., double max = -1.) const;


private:
  typedef std::map< TString, TGraph* > Plots;
  typedef std::map< TString, TGraph* >::iterator PlotIt;  

  const int nSubDet_;

  TString id_;
  TString fileName_;
  std::set<int> exclAlignables_;

  Plots createPlots(const Variable &var1, const Variable &var2) const;
  void setStyle(Plots &plots) const;
  void getRange(Plots &plots, double &xMin, double &xMax, double &yMin, double &yMax) const;
  void getRange(const TGraph* g, double &xMin, double &xMax, double &yMin, double &yMax) const;
};


GeometryComparison::GeometryComparison(const TString &fileName, const TString &id)
  : nSubDet_(6) {
  TH1::AddDirectory(true);
  id_ = id;
  id_.ReplaceAll(".root","");
  fileName_ = fileName;
}


// vars can be:
// - "<var1> : <var2>"; or
// - "<var1> * <var2> : <var3>"
void GeometryComparison::draw(const TString &expr, double min, double max) const {
  TString str(expr);
  str.ReplaceAll(" ","");
  const int posColon = str.First(":");
  const TString expr1 = str(0,posColon);
  const TString expr2 = str(posColon+1,str.Length()-posColon-1);
  Variable var1(expr1);
  Variable var2(expr2);
  TCanvas* can = new TCanvas("can_"+id_+"_"+var1()+":"+var2(),var1()+":"+var2(),500,500);
  can->cd();
  Plots plots = createPlots(var1,var2);
  setStyle(plots);
  double yMin = 0.;
  double yMax = 0.;
  double xMin = 0.;
  double xMax = 0.;
  getRange(plots,xMin,xMax,yMin,yMax);
  if( min < max ) {
    yMin = min;
    yMax = max;
  }
  TH1* hFrame = new TH1D("hFrame_"+id_+"_"+var1()+":"+var2(),"",1000,xMin,xMax);
  hFrame->GetXaxis()->SetTitle(var2());
  hFrame->GetYaxis()->SetTitle(var1());
  hFrame->GetYaxis()->SetRangeUser(yMin,yMax);
  for(int bin = 1; bin <= hFrame->GetNbinsX(); ++bin) {
    hFrame->SetBinContent(bin,0.);
    hFrame->SetBinError(bin,0.);
  }
  hFrame->SetLineStyle(2);
  hFrame->Draw("HIST");
  for(PlotIt it = plots.begin(); it != plots.end(); ++it) {
    it->second->Draw("Psame");
  }
  can->SaveAs(id_+"_"+var1.screenLabel()+"_vs_"+var2.screenLabel()+".pdf");

  for(PlotIt it = plots.begin(); it != plots.end(); ++it) {
    delete it->second;
  }
  plots.clear();
  delete hFrame;
  delete can;
}


GeometryComparison::Plots GeometryComparison::createPlots(const Variable &var1, const Variable &var2) const {
  Plots plots;

  // Store coordinates
  std::vector< std::vector<float> > xs(nSubDet_);
  std::vector< std::vector<float> > ys(nSubDet_);

  // names of used tree variables and their values
  // note: can use SetBranchAddress only to ONE variable!
  std::vector<TString> names;
  std::vector<float> vals;
  for(size_t i = 0; i < var1.nTreeVariables(); ++i) {
    const TString name = var1.treeVariable(i);
    bool nameExist = false;
    for(size_t j = 0; j < names.size(); ++j) {
      if( names.at(j) == name ) {
	nameExist = true;
	break;
      }
    }
    if( !nameExist ) {
      names.push_back(name);
      vals.push_back(0.);
    }
  }
  for(size_t i = 0; i < var2.nTreeVariables(); ++i) {
    const TString name = var2.treeVariable(i);
    bool nameExist = false;
    for(size_t j = 0; j < names.size(); ++j) {
      if( names.at(j) == name ) {
	nameExist = true;
	break;
      }
    }
    if( !nameExist ) {
      names.push_back(name);
      vals.push_back(0.);
    }
  }

  int id = 0;
  int level = 0;
  int sublevel = 0;

  TFile file(fileName_,"READ");
  TTree* tree = NULL;
  file.GetObject("alignTree",tree);
  if( tree == NULL ) {
    std::cerr << "\n\nERROR reading tree from file" << std::endl;
    throw std::exception();
  }


  tree->SetBranchAddress("id",&id);
  tree->SetBranchAddress("level",&level);
  tree->SetBranchAddress("sublevel",&sublevel);
  for(size_t i = 0; i < names.size(); ++i) {
    tree->SetBranchAddress(names.at(i),&vals.at(i));
  }


  // pointers to variables read from tree
  std::vector<float*> yVals(var1.nTreeVariables(),0);
  std::vector<float*> xVals(var2.nTreeVariables(),0);
  for(size_t i = 0; i < var1.nTreeVariables(); ++i) {
    const TString name = var1.treeVariable(i);
    for(size_t j = 0; j < names.size(); ++j) {
      if( names.at(j) == name ) {
	yVals.at(i) = &(vals.at(j));
	break;
      }
    }    
  }
  for(size_t i = 0; i < var2.nTreeVariables(); ++i) {
    const TString name = var2.treeVariable(i);
    for(size_t j = 0; j < names.size(); ++j) {
      if( names.at(j) == name ) {
	xVals.at(i) = &(vals.at(j));
	break;
      }
    }    
  }

  // loop over tree
  for(int i = 0; i < tree->GetEntries(); ++i) {
    tree->GetEntry(i);

    if( exclAlignables_.find( id ) != exclAlignables_.end() ) continue;
    if( level != 1 ) continue;	// Detector (DetId==1 is Tracker: DataFormats/DetId/interface/DetId.h)
    if( sublevel > 0 && sublevel < nSubDet_+1 ) { // Sub-Detector Id
      // std::cout << "\nyVals[0] = " << yVals.at(0) << std::endl;
      // std::cout << "yVals[1] = " << yVals.at(1) << std::endl;
      ys.at(sublevel-1).push_back( var1.eval(yVals) );
      xs.at(sublevel-1).push_back( var2.eval(xVals) );
    }
  }
  for(unsigned int l = 0; l < xs.size(); ++l) {
    TString det("PXB");		// sublevel 1
    if(      l == 1 ) det = "PXF"; // sublevel 2
    else if( l == 2 ) det = "TIB"; // sublevel 3
    else if( l == 3 ) det = "TID"; // sublevel 4
    else if( l == 4 ) det = "TOB"; // sublevel 5
    else if( l == 5 ) det = "TEC"; // sublevel 6
    plots[det] = new TGraph(xs.at(l).size(),&(xs.at(l).front()),&(ys.at(l).front()));
  }

  delete tree;
  file.Close();

  return plots;
}


void GeometryComparison::setStyle(Plots &plots) const {
  int color = 1;
  for(PlotIt it = plots.begin(); it != plots.end(); ++it, ++color) {
    const TString det = it->first;
    it->second->SetMarkerStyle(7);
    if(      det == "PXB" ) it->second->SetMarkerColor(kBlack);
    else if( det == "PXF" ) it->second->SetMarkerColor(kRed);
    else if( det == "TIB" ) it->second->SetMarkerColor(kGreen+1);
    else if( det == "TID" ) it->second->SetMarkerColor(kBlue);
    else if( det == "TOB" ) it->second->SetMarkerColor(kMagenta);
    else if( det == "TEC" ) it->second->SetMarkerColor(kCyan);
  }
}


void GeometryComparison::getRange(Plots &plots, double &xMin, double &xMax, double &yMin, double &yMax) const {
  yMin = 9999.;
  yMax = -9999.;
  xMin = 9999.;
  xMax = -9999.;
  for(PlotIt it = plots.begin(); it != plots.end(); ++it) {
    double minx = 9999.;
    double maxx = -9999.;
    double miny = 9999.;
    double maxy = -9999.;
    getRange(it->second,minx,maxx,miny,maxy);
    if( minx < xMin ) xMin = minx;
    if( maxx > xMax ) xMax = maxx;
    if( miny < yMin ) yMin = miny;
    if( maxy > yMax ) yMax = maxy;
  }
  const double deltaY = yMax - yMin;
  yMin -= 0.1*deltaY;
  yMax += 0.1*deltaY;
  const double deltaX = xMax - xMin;
  xMin -= 0.1*deltaX;
  xMax += 0.1*deltaX;
}


void GeometryComparison::getRange(const TGraph* g, double &xMin, double &xMax, double &yMin, double &yMax) const {
  yMin = 9999.;
  yMax = -9999.;
  xMin = 9999.;
  xMax = -9999.;
  for(int i = 0; i < g->GetN(); ++i) {
    const double xVal = g->GetX()[i];
    if( xVal < xMin ) xMin = xVal;
    if( xVal > xMax ) xMax = xVal;
    const double yVal = g->GetY()[i];
    if( yVal < yMin ) yMin = yVal;
    if( yVal > yMax ) yMax = yVal;
  }
}


// Expects .txt file with ids of excluded DetUnits
// One id per line. Empty lines and lines starting
// with '#' are ignored.
void GeometryComparison::excludeModules(const TString& fileName) {
  exclAlignables_.clear();

  // Open file for reading
  std::ifstream file( fileName.Data() );
  if( !file.is_open() ) {
    std::cerr << "\n\nERROR error opening file '" << fileName << "'\n";
    throw std::exception();
  }

  // Loop over lines and parse
  std::string line("");
  while( !file.eof() ) {
    std::getline(file,line);
    TString id(line);
    id.ReplaceAll(" ","");
    if( id.Length() > 0 ) {
      if( id[0] != '#' ) {
	if( !id.IsDigit() ) {
	  std::cerr << "\n\nERROR: unrecognised DetUnit Id '" << id << "'\n\n" << std::endl;
	  throw std::exception();
	}
	exclAlignables_.insert( id.Atoi() );
      }
    }
  }
}

#endif
