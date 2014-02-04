{
  gROOT->ProcessLine(".x loadPlotter.C");

  const TString path = "/home/matsch/Documents/TrackerAlignment/studies/validation/weakModes2012/";

  const double dphiMin = -7E-4;
  const double dphiMax = 7E-4;
  const double dyMin = -0.032;
  const double dyMax = 0.032;

  // // control path (no misalignment)
  // GeometryComparison c1(path+"mp1337_vs_mp1385.Comparison_commonTracker.root","control_mp1337_vs_mp1385");
  // c1.draw("dphi:z",dphiMin,dphiMax);
  // c1.draw("dy:r",dyMin,dyMax);
  // GeometryComparison c2(path+"mp1337_vs_mp1400.Comparison_commonTracker.root","control_mp1337_vs_mp1400");
  // c2.draw("dphi:z",dphiMin,dphiMax);
  // c2.draw("dy:r",dyMin,dyMax);

  // // twist misalignment wrt to starting geometry
  // GeometryComparison t0(path+"mp1337_vs_twist5.Comparison_commonTracker.root","twist_on_mp1337");
  // t0.draw("dphi:z",dphiMin,dphiMax);
  // GeometryComparison t1(path+"mp1337_vs_mp1392.Comparison_commonTracker.root","twist_mp1337_vs_mp1392");
  // t1.draw("dphi:z",dphiMin,dphiMax);
  // GeometryComparison t2(path+"mp1337_vs_mp1397.Comparison_commonTracker.root","twist_mp1337_vs_mp1397");
  // t2.draw("dphi:z",dphiMin,dphiMax);

  // // sagitta misalignment wrt to starting geometry
  // GeometryComparison s0(path+"mp1337_vs_sagitta2.Comparison_commonTracker.root","sagitta_on_mp1337");
  // s0.draw("dy:r",dyMin,dyMax);
  GeometryComparison s1(path+"mp1337_vs_mp1393.Comparison_commonTracker.root","sagitta_mp1337_vs_mp1393");
  //  s1.excludeModules("/home/matsch/tmp/mp1337ExclAlignables.txt");
  s1.draw("dy:r",dyMin,dyMax);
  // GeometryComparison s2(path+"mp1337_vs_mp1394.Comparison_commonTracker.root","sagitta_mp1337_vs_mp1394");
  // s2.draw("dy:r",dyMin,dyMax);
  // GeometryComparison s11(path+"mp1337_vs_mp1399.Comparison_commonTracker.root","sagitta_mp1337_vs_mp1399");
  // s11.draw("dy:r",dyMin,dyMax);

  // twist misalignment wrt to iterated starting geometry
  // GeometryComparison tt1(path+"mp1385_vs_mp1392.Comparison_commonTracker.root","twist_mp1385_vs_mp1392");
  // tt1.draw("dphi:z",dphiMin,dphiMax);
  // GeometryComparison tt2(path+"mp1400_vs_mp1397.Comparison_commonTracker.root","twist_mp1400_vs_mp1397");
  // tt2.draw("dphi:z",dphiMin,dphiMax);

  // // sagitta misalignment wrt to iterated starting geometry
  // GeometryComparison ss1(path+"mp1385_vs_mp1393.Comparison_commonTracker.root","sagitta_mp1385_vs_mp1393");
  // ss1.draw("dy:r",dyMin,dyMax);
  // GeometryComparison ss2(path+"mp1400_vs_mp1394.Comparison_commonTracker.root","sagitta_mp1400_vs_mp1394");
  // ss2.draw("dy:r",dyMin,dyMax);

  // // Stability of mp1337
  // GeometryComparison c1(path+"mp1337_vs_mp1385.Comparison_commonTracker.root","mp1337_vs_mp1385");
  // c1.draw("dphi:z",dphiMin,dphiMax);
  // c1.draw("dy:r",dyMin,dyMax);
  // GeometryComparison c2(path+"mp1385_vs_mp1400.Comparison_commonTracker.root","mp1385_vs_mp1400");
  // c2.draw("dphi:z",dphiMin,dphiMax);
  // c2.draw("dy:r",dyMin,dyMax);



}
