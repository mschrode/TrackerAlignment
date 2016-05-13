#ifndef CALIBRATION_PARAMETER_PLOTTER_H
#define CALIBRATION_PARAMETER_PLOTTER_H

#include <cmath>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TError.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TString.h"
#include "TStyle.h"

#include "Detector.h"
#include "ParameterSet.h"
#include "CalibrationParameterReader.h"


class CalibrationParameterPlotter {
public:
  CalibrationParameterPlotter(const TString& geometryFile);

  void plot(const TString& treeFile, const TString& outNamePrefix="CalibPars") const;


private:
  Tracker tracker_;

  // plots for one detector
  void plot(const ParameterSet& pars, const TString& outNamePrefix) const;

  // little helpers
  TString yTitle(const CalibrationParameterType type) const;
  int color(const unsigned int ring, const unsigned int nRings) const;
  int markerStyle(const unsigned int ring, const unsigned int nRings) const;
  TLegend* createLegend(const unsigned int nRings) const;
  TPaveText* createTitle(const TString& txt) const;
};


CalibrationParameterPlotter::CalibrationParameterPlotter(const TString& geometryFile) 
  : tracker_(Tracker(geometryFile)) {

  // Suppress message when canvas has been saved
  gErrorIgnoreLevel = 1001;

  // Zero horizontal error bars
  gStyle->SetErrorX(0);

  //  For the canvas
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetCanvasDefH(800); //Height of canvas
  gStyle->SetCanvasDefW(800); //Width of canvas
  gStyle->SetCanvasDefX(0);   //Position on screen
  gStyle->SetCanvasDefY(0);
  
  //  For the frame
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameFillColor(kBlack);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(kBlack);
  gStyle->SetFrameLineStyle(0);
  gStyle->SetFrameLineWidth(1);
  gStyle->SetLineWidth(2);
    
  //  For the Pad
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);
  
  //  Margins
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.18);
  gStyle->SetPadRightMargin(0.05);

  //  For the histo:
  gStyle->SetHistLineColor(kBlack);
  gStyle->SetHistLineStyle(0);
  gStyle->SetHistLineWidth(1);
  gStyle->SetMarkerSize(1);
  gStyle->SetEndErrorSize(4);
  gStyle->SetHatchesLineWidth(1);

  //  For the statistics box:
  gStyle->SetOptStat(0);
  
  //  For the axis
  gStyle->SetAxisColor(1,"XYZ");
  gStyle->SetTickLength(0.03,"XYZ");
  gStyle->SetNdivisions(510,"XYZ");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetStripDecimals(kFALSE);
  
  //  For the axis labels and titles
  gStyle->SetTitleColor(1,"XYZ");
  gStyle->SetLabelColor(1,"XYZ");
  gStyle->SetLabelFont(42,"XYZ");
  gStyle->SetLabelOffset(0.007,"XYZ");
  gStyle->SetLabelSize(0.04,"XYZ");
  gStyle->SetTitleFont(42,"XYZ");
  gStyle->SetTitleSize(0.047,"XYZ");
  gStyle->SetTitleXOffset(1.5);
  gStyle->SetTitleYOffset(1.9);

  //  For the legend
  gStyle->SetLegendBorderSize(0);
}


TString CalibrationParameterPlotter::yTitle(const CalibrationParameterType type) const {
  if(      type == StripBPDeco ) return "#DeltaW^{shift}_{BP} [% of module thickness]";
  else if( type == StripLADeco ) return "deco-mode tan(#theta^{shift}_{LA})";
  else if( type == StripLAPeak ) return "peak-mode tan(#theta^{shift}_{LA})";
  else if( type == PixelLA     ) return "tan(#theta^{shift}_{LA})";
  else                           return "";
}


int CalibrationParameterPlotter::color(const unsigned int ring, const unsigned int nRings) const {
  const int modulo = nRings/2;
  const int idx = nRings > 4 ? ring%modulo : ring;
  if( idx == 0 ) return kBlack;
  if( idx == 1 ) return kRed;
  if( idx == 2 ) return kBlue;
  if( idx == 3 ) return kGreen+2;
  if( idx == 4 ) return kOrange;
  if( idx == 5 ) return kCyan;
  if( idx == 6 ) return kMagenta;

  else           return kBlack;
}


int CalibrationParameterPlotter::markerStyle(const unsigned int ring, const unsigned int nRings) const {
  const int modulo = nRings/2;
  int idx = ring;
  if( nRings > 4 ) {
    idx = ring%modulo;
    idx += 10*(ring/modulo);
  }

  if( idx ==  0 ) return 20;
  if( idx ==  1 ) return 21;
  if( idx ==  2 ) return 22;
  if( idx ==  3 ) return 23;
  if( idx ==  4 ) return 29;
  if( idx ==  5 ) return 33;
  if( idx ==  6 ) return 34;

  if( idx == 10 ) return 24;
  if( idx == 11 ) return 25;
  if( idx == 12 ) return 26;
  if( idx == 13 ) return 32;
  if( idx == 14 ) return 30;
  if( idx == 15 ) return 27;
  if( idx == 16 ) return 28;

  else            return 7;
}


TLegend* CalibrationParameterPlotter::createLegend(const unsigned int nRings) const {
  int nCols = 1;
  int nRows = nRings;
  if( nRings > 4 ) {
    nCols = 2;
    nRows = nRings%2 == 0 ? nRings/2 : nRings/2+1;
  }

  const double lineHeight = 0.04;
  const double margin = 0.05;
  const double colWidth = 0.25;

  const double x0 = 1.-gStyle->GetPadRightMargin()-margin-nCols*colWidth*(1.-gStyle->GetPadLeftMargin()-gStyle->GetPadRightMargin()-2.*margin);
  const double x1 = 1.-gStyle->GetPadRightMargin()-margin;
  const double y1 = 1.-gStyle->GetPadTopMargin()-margin;
  const double y0 = y1-nRows*lineHeight;

  TLegend* leg = new TLegend(x0,y0,x1,y1);
  leg->SetNColumns(nCols);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->SetTextFont(42);
  leg->SetTextSize(0.038);

  return leg;
}


TPaveText* CalibrationParameterPlotter::createTitle(const TString& txt) const {
  double x0 = gStyle->GetPadLeftMargin();
  double x1 = 1.-gStyle->GetPadRightMargin();
  double y0 = 1.-gStyle->GetPadTopMargin()+0.01;
  double y1 = 1.;
  TPaveText* theTitle = new TPaveText(x0,y0,x1,y1,"NDC");
  theTitle->SetBorderSize(0);
  theTitle->SetFillColor(10);
  theTitle->SetFillStyle(1001);
  theTitle->SetTextFont(42);
  theTitle->SetTextAlign(12);	// left adjusted and vertically centered
  theTitle->SetTextSize(0.040);
  theTitle->SetMargin(0.);
  theTitle->AddText(txt);
  
  return theTitle;
}



void CalibrationParameterPlotter::plot(const ParameterSet& pars, const TString& outNamePrefix) const {
  const Detector det = pars.detector();

  std::cout << "Creating parameter plots" << std::endl;
  std::cout << "  " << toStr(det) << std::endl;

  // In case of LA calibration, multiply By=3.8 to parameter values
  // Alignment determines mobility mu, where tan(theta_LA) = mu*By
  // and dx = d/2 * tan(theta_LA)
  const double scale = pars.type() == StripBPDeco ? 1. : 3.8;

  TCanvas* can = new TCanvas("can","calibration parameters",500,500);
  can->cd();

  // plot value vs IOV:
  // - one canvas per layer
  // - plots for different rings overlayed
  const int nIOVs = static_cast<int>(pars.nIOVs());
  TH1* frame = new TH1D("frame",";IOV;"+yTitle(pars.type()),nIOVs,0.5,nIOVs+0.5);
  frame->SetLineStyle(0);
  for(int bin = 1; bin <= frame->GetNbinsX(); ++bin) {
    frame->SetBinContent(bin,-100);
  }

  double yMin =  1000.;
  double yMax = -1000.;

  // loop over layers = units in r
  for(unsigned int iLayer = 0; iLayer < pars.nRBins(); ++iLayer) {
    TString titletxt = toStr(det)+" layer ";
    titletxt += iLayer+1;
    TPaveText* title = createTitle(titletxt);

    TString outName = outNamePrefix+"_"+toStr(det)+"_Layer";
    outName += iLayer+1;

    std::vector<TGraph*> graphs;
    std::vector<TH1*> starts;
    TLegend* leg = createLegend(pars.nZBins());
    // loop over rings = units in z
    const unsigned int nRings = pars.nZBins();
    for(unsigned int iRingCounter = 0; iRingCounter < nRings; ++iRingCounter) {
      // want the legend get filled column-wise: need to re-order
      // sequence to first even entries then odd in case of more
      // than one column
      unsigned int iRing = iRingCounter;
      if( nRings > 4 ) {
	if( iRingCounter%2 == 0 ) {
	  iRing = iRingCounter/2;
	} else {
	  unsigned int offset = nRings/2;
	  if( nRings%2 != 0 ) offset += 1;
	  iRing = offset + (iRingCounter-1)/2;
	}
      }

      std::vector<double> iovs;
      std::vector<double> zeros;
      std::vector<double> values;
      std::vector<double> errors;

      TString hname = "start";
      hname += iRing;
      TH1* start = static_cast<TH1*>(frame->Clone(hname));
      for(unsigned int iov = 0; iov < pars.nIOVs(); ++iov) {
	const double finalval = scale*pars.value(iRing,iLayer,iov);
	const double delta = scale*pars.delta(iRing,iLayer,iov);
	const double startval = finalval - delta;
	const double error = scale*pars.error(iRing,iLayer,iov);
	iovs.push_back(iov+1);
	values.push_back(finalval);
	start->SetBinContent(1+iov,startval);
	zeros.push_back(0);
	errors.push_back(error);
	if( std::min(startval,finalval) < yMin ) yMin = std::min(startval,finalval);
	if( std::max(startval,finalval) > yMax ) yMax = std::max(startval,finalval);
      }
      TGraph* graph = new TGraphErrors(iovs.size(),&(iovs.front()),&(values.front()),
				       &(zeros.front()),&(errors.front()));
      graph->SetMarkerStyle( markerStyle(iRing,nRings) );
      graph->SetMarkerColor( color(iRing,nRings) );
      graph->SetLineColor(graph->GetMarkerColor());
      graphs.push_back(graph);

      start->SetLineWidth(2);
      start->SetLineStyle(2);
      start->SetLineColor(graph->GetLineColor());
      starts.push_back(start);
	
      const unsigned int rMin = pars.zBinMin(iRing);
      const unsigned int rMax = pars.zBinMax(iRing);
      TString entry = "ring ";
      if( rMin == rMax ) {
	entry += rMin+1;
      } else {
	entry += rMin+1;
	entry += "-";
	entry += rMax+1;
      }
      leg->AddEntry(graph,entry,"P");

    }	// end of loop over rings

    const double deltaY = yMax-yMin;
    frame->GetYaxis()->SetRangeUser(yMin-0.4*deltaY,yMax+deltaY);

    frame->Draw("HIST");
    // for(std::vector<TH1*>::reverse_iterator hit = starts.rbegin();
    // 	hit != starts.rend(); ++hit) {
    //   // loop backwards to have funny colors hidden in case
    //   // lines are on top of each other
    //   (*hit)->Draw("Hsame");
    // }
    for(std::vector<TGraph*>::iterator git = graphs.begin();
	git != graphs.end(); ++git) {
      (*git)->Draw("PEsame");
    }
    leg->Draw("same");
    title->Draw("same");
    can->SaveAs(outName+".pdf");
    
    for(std::vector<TGraph*>::iterator git = graphs.begin();
	git != graphs.end(); ++git) {
      delete *git;
    }
    for(std::vector<TH1*>::iterator hit = starts.begin();
	hit != starts.end(); ++hit) {
      delete *hit;
    }
    delete leg;
    delete title;
      
  } // end of loop over layers

  delete frame;
  delete can;
}


void CalibrationParameterPlotter::plot(const TString& treeFile, const TString& outNamePrefix) const {
  std::cout << "Reading fitted calibration parameters" << std::endl;
  const CalibrationParameterReader reader(&tracker_);
  CalibrationParameterType types[4] = { PixelLA, StripLADeco, StripLAPeak, StripBPDeco };
  for(int t = 0; t < 4; ++t) {
    std::map<Detector,ParameterSet> parsPerDet = reader.read(types[t],treeFile);
    for(std::map<Detector,ParameterSet>::const_iterator it = parsPerDet.begin();
	it != parsPerDet.end(); ++it) {
      it->second.print();
      TString outName = outNamePrefix;
      if(      types[t] == PixelLA     ) outName += "_LA";
      else if( types[t] == StripLADeco ) outName += "_LA-Deco";
      else if( types[t] == StripLAPeak ) outName += "_LA-Peak";
      else if( types[t] == StripBPDeco ) outName += "_BP";
      
      plot(it->second,outName);
    }
  }
}

#endif
