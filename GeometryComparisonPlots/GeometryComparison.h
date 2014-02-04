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
  ~GeometryComparison();

  void excludeModules(const TString& fileName);

  void draw(const TString &vars, double min = 1., double max = -1.) const;


private:
  typedef std::map< TString, TGraph* > Plots;
  typedef std::map< TString, TGraph* >::iterator PlotIt;  

  const int nSubDet_;

  TString id_;
  TFile* file_;
  TTree* tree_;
  std::set<int> exclAlignables_;

  Plots createPlots(const Variable &var1, const Variable &var2) const;
  void setStyle(Plots &plots) const;
  void getRange(Plots &plots, double &xMin, double &xMax, double &yMin, double &yMax) const;
  void getRange(const TGraph* g, double &xMin, double &xMax, double &yMin, double &yMax) const;
};


GeometryComparison::GeometryComparison(const TString &fileName, const TString &id)
  : nSubDet_(6) {
  id_ = id;
  id_.ReplaceAll(".root","");
  file_ = new TFile(fileName,"READ");
  tree_ = NULL;
  file_->GetObject("alignTree",tree_);
  if( tree_ == NULL ) {
    std::cerr << "\n\nERROR reading tree from file" << std::endl;
    throw std::exception();
  }
}


GeometryComparison::~GeometryComparison() {
  file_->Close();
  delete file_;
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
}


GeometryComparison::Plots GeometryComparison::createPlots(const Variable &var1, const Variable &var2) const {
  Plots plots;

  // Store coordinates
  std::vector< std::vector<float> > xs(nSubDet_);
  std::vector< std::vector<float> > ys(nSubDet_);

  int id = 0;
  int level = 0;
  int sublevel = 0;
  std::vector<float> yVals(var1.nTreeVariables(),0.);
  std::vector<float> xVals(var2.nTreeVariables(),0.);
  tree_->SetBranchAddress("id",&id);
  tree_->SetBranchAddress("level",&level);
  tree_->SetBranchAddress("sublevel",&sublevel);
  for(unsigned int i = 0; i < yVals.size(); ++i) {
    tree_->SetBranchAddress(var1.treeVariable(i),&yVals.at(i));
  }
  for(unsigned int i = 0; i < xVals.size(); ++i) {
    tree_->SetBranchAddress(var2.treeVariable(i),&xVals.at(i));
  }
  for(int i = 0; i < tree_->GetEntries(); ++i) {
    tree_->GetEntry(i);
    if( exclAlignables_.find( id ) != exclAlignables_.end() ) continue;
    if( level != 1 ) continue;
    if( sublevel > 0 && sublevel < nSubDet_+1 ) {
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

  return plots;
}


void GeometryComparison::setStyle(Plots &plots) const {
  int color = 1;
  for(PlotIt it = plots.begin(); it != plots.end(); ++it, ++color) {
    const TString det = it->first;
    it->second->SetMarkerStyle(7);
    if(      det == "PXB" ) it->second->SetMarkerColor(kBlack);
    else if( det == "PXF" ) it->second->SetMarkerColor(kRed);
    else if( det == "TIB" ) it->second->SetMarkerColor(kBlue);
    else if( det == "TID" ) it->second->SetMarkerColor(kGreen+2);
    else if( det == "TOB" ) it->second->SetMarkerColor(kMagenta);
    else if( det == "TEC" ) it->second->SetMarkerColor(kOrange);
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
