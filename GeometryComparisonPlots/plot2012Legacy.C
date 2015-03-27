{
  gROOT->ProcessLine(".x loadPlotter.C");

  const TString path = "/home/matsch/TrackerAlignment/studies/validation/geometryComparisons/2012Legacy/";

  const double drMin    = -100;
  const double drMax    =  100;
  const double dzMin    = -100;
  const double dzMax    =  100;
  const double dxyMin   = -100;
  const double dxyMax   =  100;
  const double rdphiMin =  -20;
  const double rdphiMax =   20;

  const double scale = 1.;

  const int nFiles = 4;
  TString fileNames[nFiles] = {
    path+"mp1535_vs_mp1511.Comparison_commonTracker_Images/mp1535_vs_mp1511.Comparison_commonTracker.root",
    path+"mp1511_vs_mp1510.Comparison_commonTracker_Images/mp1511_vs_mp1510.Comparison_commonTracker.root",
    path+"mp1510_vs_mp1509.Comparison_commonTracker_Images/mp1510_vs_mp1509.Comparison_commonTracker.root",
    path+"mp1509_vs_start.Comparison_commonTracker_Images/mp1509_vs_start.Comparison_commonTracker.root"    };
  TString ids[nFiles] = { "mp1535_vs_mp1511", "mp1511_vs_mp1510", "mp1510_vs_mp1509", "mp1509_vs_start" };
  for(int i = 0; i < nFiles; ++i ) {
    GeometryComparison gc(fileNames[i],ids[i]);
    gc.draw( "dr:r",   scale*drMin, scale*drMax );
    gc.draw( "dr:z",   scale*drMin, scale*drMax );
    gc.draw( "dr:phi", scale*drMin, scale*drMax );
    
    gc.draw( "dz:r",   scale*dzMin, scale*dzMax );
    gc.draw( "dz:z",   scale*dzMin, scale*dzMax );
    gc.draw( "dz:phi", scale*dzMin, scale*dzMax );
    
    gc.draw( "r*dphi:r",   scale*rdphiMin, scale*rdphiMax );
    gc.draw( "r*dphi:z",   scale*rdphiMin, scale*rdphiMax );
    gc.draw( "r*dphi:phi", scale*rdphiMin, scale*rdphiMax );
    
    gc.draw( "dx:r",   scale*dxyMin, scale*dxyMax );
    gc.draw( "dx:z",   scale*dxyMin, scale*dxyMax );
    gc.draw( "dx:phi", scale*dxyMin, scale*dxyMax );
    
    gc.draw( "dy:r",   scale*dxyMin, scale*dxyMax );
    gc.draw( "dy:z",   scale*dxyMin, scale*dxyMax );
    gc.draw( "dy:phi", scale*dxyMin, scale*dxyMax );
  }
}
