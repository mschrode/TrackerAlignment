#include <cmath>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH1D.h"
#include "TLegend.h"

#include "Detector.h"
#include "ParameterSet.h"
#include "CalibrationParameterReader.h"

int marker(unsigned int idx) {
  if( idx == 0 ) return 20;
  if( idx == 1 ) return 21;
  if( idx == 2 ) return 22;
  if( idx == 3 ) return 24;
  if( idx == 4 ) return 25;
  if( idx == 5 ) return 26;
  else return 21+idx;
}

void test(const TString& detector, const TString& treeFile) {
  std::cout << "Initialising tracker" << std::endl;
  const Tracker tracker(detector);

  std::cout << "Reading fitted calibration parameters" << std::endl;
  CalibrationParameterReader pr(&tracker);
  std::map<Detector,ParameterSet> pars = pr.read(PixelLA,treeFile);
  for(std::map<Detector,ParameterSet>::const_iterator it = pars.begin();
      it != pars.end(); ++it) {
    it->second.print();
  }

  std::cout << "Creating parameter plots" << std::endl;
  TCanvas* can = new TCanvas("can","calibration parameters",500,500);
  can->cd();
  for(std::map<Detector,ParameterSet>::const_iterator it = pars.begin();
      it != pars.end(); ++it) {
    const Detector det = it->first;
    const ParameterSet ps = it->second;

    std::cout << "  " << toStr(det) << std::endl;
    
    // plot value vs IOV:
    // - one canvas per layer
    // - plots for different rings overlayed
    const int nIOVs = static_cast<int>(ps.nIOVs());
    TH1* frame = new TH1D("frame",";IOV;#DeltaW^{shift}_{BP}",nIOVs,0.5,nIOVs+0.5);
    frame->SetLineStyle(2);
    frame->GetYaxis()->SetRangeUser(-0.05,0.15);
    frame->GetYaxis()->SetRangeUser(0.35,0.46);
    //frame->GetYaxis()->SetRangeUser(0.060,0.082);

    for(unsigned int iLayer = 0; iLayer < ps.nRBins(); ++iLayer) {
      TString title = toStr(det)+" layer ";
      title += iLayer+1;
      TString name = "CalibPars_"+toStr(det)+"_Layer";
      name += iLayer+1;

      std::vector<TGraph*> graphs;
      std::vector<TH1*> starts;
      TLegend* leg = new TLegend(0.5,0.5,0.9,0.9);
      for(unsigned int iRing = 0; iRing < ps.nZBins(); ++iRing) {
	std::vector<double> iovs;
	std::vector<double> values;

	TString tmpname = "start";
	tmpname += iRing;
	TH1* start = static_cast<TH1*>(frame->Clone(tmpname));
	for(unsigned int iov = 0; iov < ps.nIOVs(); ++iov) {
	  const double finalval = 3.8*ps.value(iRing,iLayer,iov);
	  const double startval = finalval - ps.delta(iRing,iLayer,iov);

	  iovs.push_back(iov+1);
	  values.push_back(finalval);

	  start->SetBinContent(1+iov,startval);
	}
	TGraph* graph = new TGraph(iovs.size(),&(iovs.front()),&(values.front()));
	graph->SetMarkerStyle(marker(iRing));
	graph->SetMarkerColor( 1 + iRing%3 );
	graph->SetLineColor(graph->GetLineColor());
	graphs.push_back(graph);

	start->SetLineColor(graph->GetLineColor());
	starts.push_back(start);
	
	const unsigned int rMin = ps.zBinMin(iRing);
	const unsigned int rMax = ps.zBinMax(iRing);
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
      
      frame->SetTitle(title);
      frame->Draw("HIST");
      for(std::vector<TGraph*>::iterator git = graphs.begin();
	  git != graphs.end(); ++git) {
	(*git)->Draw("Psame");
      }
      for(std::vector<TH1*>::iterator hit = starts.begin();
	  hit != starts.end(); ++hit) {
	(*hit)->Draw("Hsame");
      }
      leg->Draw("same");
      can->SaveAs(name+".pdf");

      for(std::vector<TGraph*>::iterator git = graphs.begin();
	  git != graphs.end(); ++git) {
	delete *git;
      }
      delete leg;
      
    } // end of loop over layers

    delete frame;

  } //  end of loop over detectors
  delete can;
}
