// Plot the high-level structure parameters
//
// Runs on the treeFile_merge.root and plots the fitted alignment parameters
// of the high-level structures. The script knows which parameters and structures
// are fitted from the input.
//
// Run it standalone in ROOT, script needs to be compiled, e.g.
// root[0] .L plotHighLevelStructureParameters.C+
// root[1] plotHighLevelStructureParameters("..../treeFile_merge.root","great alignment")


#include <iostream>
#include <cmath>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TPaveText.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"

// declaration of main routine
void plotHighLevelStructureParameters(const TString& treeFileName, const TString& label, const bool plotErrors=false, const int iov=1);


TString detectorLabel(const int objId) {
  if( objId <  8 ) return "BPX";
  if( objId < 14 ) return "FPX";
  if( objId < 21 ) return "TIB";
  if( objId < 26 ) return "TID";
  if( objId < 31 ) return "TOB";
  if( objId < 37 ) return "TEC";

  return "Det";
}


void plotHighLevelStructureParameters(const TString& treeFileName, const TString& label, const bool plotErrors, const int iov) {
  // treeFileName : "<path/to/jobData/jobm/>treeFile_merge.root"
  // label        : a meaningful label of the campaign, printed on the canvas and put
  //                in the output file name
  // plotErrors   : if not run in inversion mode, 'Sigma' in the tree might be filled with
  //                some other numbers from the last column (nrcds) in millepede.res
  //                --> you want to suppress drawing those!
  // label        : labelling the alignment project, e.g. mp1234, printed in canvas and 
  //                output file name


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
  gStyle->SetFrameBorderSize(1);
  gStyle->SetFrameFillColor(kBlack);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetFrameLineColor(kBlack);
  gStyle->SetFrameLineStyle(0);
  gStyle->SetFrameLineWidth(1);
  
  //  For the Pad
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(kWhite);
  gStyle->SetPadGridX(false);
  gStyle->SetPadGridY(false);
  gStyle->SetGridColor(0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);
  
  //  Margins
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadLeftMargin(0.19);
  gStyle->SetPadRightMargin(0.04);
  
  //  For the histo:
  gStyle->SetHistLineColor(kBlack);
  gStyle->SetHistLineStyle(0);
  gStyle->SetHistLineWidth(2);
  gStyle->SetMarkerSize(1.4);
  gStyle->SetEndErrorSize(4);
  
  //  For the Global title:
  gStyle->SetOptTitle(1);
  gStyle->SetTitleFont(42,"");
  gStyle->SetTitleColor(1);
  gStyle->SetTitleTextColor(1);
  gStyle->SetTitleFillColor(0);
  gStyle->SetTitleFontSize(0.1);
  gStyle->SetTitleAlign(13);
  gStyle->SetTitleX(0.00);
  gStyle->SetTitleH(0.05);
  gStyle->SetTitleBorderSize(0);

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

  // For the axis labels:
  gStyle->SetLabelFont(42,"XYZ");
  gStyle->SetLabelOffset(0.007,"XYZ");
  gStyle->SetLabelSize(0.045,"XYZ");
  
  // For the axis titles:
  gStyle->SetTitleFont(42,"XYZ");
  gStyle->SetTitleSize(0.06,"XYZ");
  gStyle->SetTitleXOffset(1.2);
  gStyle->SetTitleYOffset(1.5);

  //  For the legend
  gStyle->SetLegendBorderSize(0);

  //  For the statistics box
  gStyle->SetOptStat("");


  std::cout << "Reading file" << std::endl;
  TFile* treeFile = new TFile(treeFileName,"READ");
  TTree* tree = 0;
  TString treeName = "MillePedeUser_";
  treeName += iov;
  treeFile->GetObject(treeName,tree);
  if( tree == 0 ) {
    std::cerr << "\n\nERROR reading tree '" << treeName << "' from file '" << treeFileName << "'\n" << std::endl;
    throw std::exception();
  }

  const size_t maxNPars = 18;	// max number of parameters per alignable
  const size_t maxNHLPars = 6;	// max number of parameters per high-level structure alignable
  std::vector< std::vector<TString> > detLabels(maxNHLPars); // [nPars]x[nAlignables] one element per alignable
  std::vector< std::vector<double> > vals(maxNHLPars);  // [nPars]x[nAlignables] we have max 6 parameters per alignable
  std::vector< std::vector<double> > errs(maxNHLPars);  // [nPars]x[nAlignables] we have max 6 parameters per alignable

  std::cout << "Setting branches" << std::endl;
  int objId = 0;
  unsigned int nPars = 0;
  float par[maxNPars];
  float sigma[maxNPars];
  float presigma[maxNPars];	// to identify fixed parameters (presigma = -1)
  tree->SetBranchAddress("ObjId",&objId);
  tree->SetBranchAddress("NumPar",&nPars);
  tree->SetBranchAddress("Par",par);
  tree->SetBranchAddress("Sigma",sigma);
  tree->SetBranchAddress("PreSigma",presigma);
  
  std::cout << "Reading parameters" << std::endl;
  for(int iAli = 0; iAli < tree->GetEntries(); ++iAli) {
    tree->GetEntry(iAli);
    if( objId > 1 ) {		// high-level structure alignable
      for(size_t iPar = 0; iPar < nPars && iPar < maxNHLPars; ++iPar) {
	if( presigma[iPar] > -1 ) { // is the parameter non-fixed?
	  vals.at(iPar).push_back(par[iPar]);
	  errs.at(iPar).push_back( (plotErrors?sigma[iPar]:0.) );
	  detLabels.at(iPar).push_back( detectorLabel(objId) );
	}
      }
    }
  }


  std::cout << "Creating plots" << std::endl;
  TCanvas* can = new TCanvas("can","high-level structure alignment",900,600);
  can->Divide(3,2);
  for(size_t iPar = 0; iPar < vals.size(); ++iPar) {
    TString name = "hFrame";
    name += iPar;
    TString parLabel = "#Deltax [#mum]";
    if(      iPar == 1 ) parLabel = "#Deltay [#mum]";
    else if( iPar == 2 ) parLabel = "#Deltaz [#mum]";
    else if( iPar == 3 ) parLabel = "#Delta#alpha [#murad]";
    else if( iPar == 4 ) parLabel = "#Delta#beta [#murad]";
    else if( iPar == 5 ) parLabel = "#Delta#gamma [#murad]";

    TH1* hFrame = new TH1D(name,";detector structure;"+parLabel,vals.at(iPar).size(),0,vals.at(iPar).size());
    for(int bin = 1; bin <= hFrame->GetNbinsX(); ++bin) {
      hFrame->GetXaxis()->SetBinLabel(bin,detLabels.at(iPar).at(bin-1));
      hFrame->SetBinContent(bin,0.);
    }
    hFrame->SetLineStyle(2);
    hFrame->SetLineColor(kBlack);
    hFrame->SetLineWidth(1);
    hFrame->GetXaxis()->LabelsOption("v");
    
    name.ReplaceAll("Frame","Par");
    TH1* hPars = static_cast<TH1*>(hFrame->Clone(name));
    hPars->SetMarkerStyle(20);
    hPars->SetMarkerSize(0.8);
    hPars->SetLineStyle(1);
    double max = 0.;
    const double scale = 1E4;	// millepede.res stores in [cm]. Convert here to [mum]
    std::cout << "\nFitted " << parLabel << std::endl;
    for(size_t iAli = 0; iAli < vals.at(iPar).size(); ++iAli) {
      const double val = scale*vals.at(iPar).at(iAli);
      const double err = scale*errs.at(iPar).at(iAli);
      printf("%5s: % 10.5f",hFrame->GetXaxis()->GetBinLabel(1+iAli),val);
      if( plotErrors ) printf(" +/- %8.5f",err);
      printf("\n");
      hPars->SetBinContent(1+iAli,val);
      if( plotErrors ) hPars->SetBinError(1+iAli,err);
      if( std::abs(val)+err > max ) max = std::abs(val)+err;
    }
    double maxCustom = iPar<3 ? 195. : 1.;
    if( 1.2*max > maxCustom ) maxCustom = 1.2*max;
    hFrame->GetYaxis()->SetRangeUser(-maxCustom,maxCustom);
    hPars->GetYaxis()->SetRangeUser(-maxCustom,maxCustom);

    can->cd(1+iPar);
    hFrame->Draw("HIST");
    if( plotErrors ) hPars->Draw("PEsame");
    else             hPars->Draw("Psame");

    if( iPar == 2 && label != "" ) {		// draw label in top-right corner
      TPaveText* info = new TPaveText(0.55,0.77,0.93,0.89,"NDC");
      info->SetBorderSize(0);
      info->SetFillColor(0);
      info->SetFillStyle(0);
      info->SetTextFont(42);
      info->SetTextAlign(22);	// horizontally and vertically centered
      info->SetMargin(0.);
      info->AddText(label);
      info->Draw("same");
    }
  }

  TString outFileName = "params.pdf";
  if( label != "") {
    outFileName = "params_"+label+".pdf";
    outFileName.ReplaceAll(" ","");
  }
  can->SaveAs(outFileName);
}
