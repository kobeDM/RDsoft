//
// ATLAS Style, based on a style file from BaBar
//

#include <iostream>

#include "inc/shStyle.h"

#include <TROOT.h>

void SetShStyle ()
{
  static TStyle* shStyle = 0;
  std::cout << "\nApplying SH style settings...\n" << std::endl ;
  if ( shStyle==0 ) shStyle = ShStyle();
  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
}

TStyle* ShStyle() 
{
  TStyle *shStyle = new TStyle("ATLAS","Sh style");

  // use plain black on white colors
  Int_t icol=0; // WHITE
  shStyle->SetFrameBorderMode(icol);
  shStyle->SetFrameFillColor(icol);
  shStyle->SetCanvasBorderMode(icol);
  shStyle->SetCanvasColor(icol);
  shStyle->SetPadBorderMode(icol);
  shStyle->SetPadColor(icol);
  shStyle->SetStatColor(icol);
  //shStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

  // set the paper & margin sizes
  shStyle->SetPaperSize(20,26);

  // set margin sizes
  shStyle->SetPadTopMargin(0.05);
  shStyle->SetPadRightMargin(0.05);
  shStyle->SetPadBottomMargin(0.16);
  shStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  shStyle->SetTitleXOffset(1.4);
  shStyle->SetTitleYOffset(1.4);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font=42; // Helvetica
  Double_t tsize=0.05;
  shStyle->SetTextFont(font);

  shStyle->SetTextSize(tsize);
  shStyle->SetLabelFont(font,"x");
  shStyle->SetTitleFont(font,"x");
  shStyle->SetLabelFont(font,"y");
  shStyle->SetTitleFont(font,"y");
  shStyle->SetLabelFont(font,"z");
  shStyle->SetTitleFont(font,"z");
  
  shStyle->SetLabelSize(tsize,"x");
  shStyle->SetTitleSize(tsize,"x");
  shStyle->SetLabelSize(tsize,"y");
  shStyle->SetTitleSize(tsize,"y");
  shStyle->SetLabelSize(tsize,"z");
  shStyle->SetTitleSize(tsize,"z");

  // use bold lines and markers
  shStyle->SetMarkerStyle(20);
  shStyle->SetMarkerSize(1.2);
  shStyle->SetHistLineWidth(2.);
  shStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

  // get rid of X error bars 
  //shStyle->SetErrorX(0.001);
  // get rid of error bar caps
  shStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  shStyle->SetOptTitle(0);
  //shStyle->SetOptStat(1111);
  shStyle->SetOptStat(0);
  //shStyle->SetOptFit(1111);
  shStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  shStyle->SetPadTickX(1);
  shStyle->SetPadTickY(1);

  return shStyle;

}

