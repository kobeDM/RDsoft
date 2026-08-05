#include <iostream>

#include "TROOT.h"
#include "TStyle.h"

//  ---------------------------------------------------------------------//
//  ███╗   ██╗ █████╗ ██████╗ ███████╗████████╗██╗   ██╗██╗     ███████╗ //
//  ████╗  ██║██╔══██╗██╔══██╗██╔════╝╚══██╔══╝╚██╗ ██╔╝██║     ██╔════╝ //
//  ██╔██╗ ██║███████║██████╔╝███████╗   ██║    ╚████╔╝ ██║     █████╗   //
//  ██║╚██╗██║██╔══██║██╔═══╝ ╚════██║   ██║     ╚██╔╝  ██║     ██╔══╝   //
//  ██║ ╚████║██║  ██║██║     ███████║   ██║      ██║   ███████╗███████╗ //
//  ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚══════╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝ //
// ----------------------------------------------------------------------//

TStyle *NAPStyle( )
{
    TStyle *pNAPStyle = new TStyle( "NAP", "NAP style" );

    // use plain black on white colors
    Int_t icol = 0;  // WHITE
    pNAPStyle->SetFrameBorderMode( icol );
    pNAPStyle->SetFrameFillColor( icol );
    pNAPStyle->SetCanvasBorderMode( icol );
    pNAPStyle->SetCanvasColor( icol );
    pNAPStyle->SetPadBorderMode( icol );
    pNAPStyle->SetPadColor( icol );
    pNAPStyle->SetStatColor( icol );
    //pNAPStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

    // set the paper & margin sizes
    pNAPStyle->SetPaperSize( 20, 26 );

    // set margin sizes
    pNAPStyle->SetPadTopMargin( 0.05 );
    pNAPStyle->SetPadRightMargin( 0.05 );
    pNAPStyle->SetPadBottomMargin( 0.12 );
    pNAPStyle->SetPadLeftMargin( 0.12 );

    // set title offsets (for axis label)
    pNAPStyle->SetTitleXOffset( 1.3 );
    pNAPStyle->SetTitleYOffset( 1.3 );

    // use large fonts
    //Int_t font=72; // Helvetica italics
    Int_t    font  = 42;  // Helvetica
    Double_t tsize = 0.04;
    pNAPStyle->SetTextFont( font );

    pNAPStyle->SetTextSize( tsize );
    pNAPStyle->SetLabelFont( font, "x" );
    pNAPStyle->SetTitleFont( font, "x" );
    pNAPStyle->SetLabelFont( font, "y" );
    pNAPStyle->SetTitleFont( font, "y" );
    pNAPStyle->SetLabelFont( font, "z" );
    pNAPStyle->SetTitleFont( font, "z" );

    pNAPStyle->SetLabelSize( tsize, "x" );
    pNAPStyle->SetTitleSize( tsize, "x" );
    pNAPStyle->SetLabelSize( tsize, "y" );
    pNAPStyle->SetTitleSize( tsize, "y" );
    pNAPStyle->SetLabelSize( tsize, "z" );
    pNAPStyle->SetTitleSize( tsize, "z" );

    // use bold lines and markers
    // pNAPStyle->SetMarkerStyle(20);
    pNAPStyle->SetMarkerSize( 1.2 );
    // pNAPStyle->SetHistLineWidth( 1.5 );
    // pNAPStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

    // get rid of X error bars
    //pNAPStyle->SetErrorX(0.001);
    // get rid of error bar caps
    pNAPStyle->SetEndErrorSize( 0. );

    // do not display any of the standard histogram decorations
    pNAPStyle->SetOptTitle( 0 );
    //pNAPStyle->SetOptStat(1111);
    pNAPStyle->SetOptStat( 0 );
    //pNAPStyle->SetOptFit(1111);
    pNAPStyle->SetOptFit( 0 );

    // put tick marks on top and RHS of plots
    pNAPStyle->SetPadTickX( 1 );
    pNAPStyle->SetPadTickY( 1 );

    return pNAPStyle;
}

void SetNAPStyle( )
{
    static TStyle *pNAPStyle = nullptr;
    std::cout << "\nApplying NAP style settings...\n" << std::endl;
    if ( pNAPStyle == 0 )
        pNAPStyle = NAPStyle( );
    gROOT->SetStyle( "NAP" );
    gROOT->ForceStyle( );

    return;
}

//  -----------------------------------------------------------//
//  ███████╗██╗  ██╗███████╗████████╗██╗   ██╗██╗     ███████╗ //
//  ██╔════╝██║  ██║██╔════╝╚══██╔══╝╚██╗ ██╔╝██║     ██╔════╝ //
//  ███████╗███████║███████╗   ██║    ╚████╔╝ ██║     █████╗   //
//  ╚════██║██╔══██║╚════██║   ██║     ╚██╔╝  ██║     ██╔══╝   //
//  ███████║██║  ██║███████║   ██║      ██║   ███████╗███████╗ //
//  ╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝ //
// ------------------------------------------------------------//

void SetColorBar( )
{
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 1.00 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.80, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable( NRGBs, stops, red, green, blue, NCont );
    gStyle->SetNumberContours( NCont );
}

TStyle *ShStyle( const int colerbar_flag = 0 )
{
    // const int colerbar_flag = 0;

    TStyle *shStyle = new TStyle( "ATLAS", "Sh style" );

    // use plain black on white colors
    Int_t icol = 0;  // WHITE
    shStyle->SetFrameBorderMode( icol );
    shStyle->SetFrameFillColor( icol );
    shStyle->SetCanvasBorderMode( icol );
    shStyle->SetCanvasColor( icol );
    shStyle->SetPadBorderMode( icol );
    shStyle->SetPadColor( icol );
    shStyle->SetStatColor( icol );
    // shStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

    // set the paper & margin sizes
    shStyle->SetPaperSize( 20, 26 );

    // set margin sizes
    if ( colerbar_flag ) {
        shStyle->SetPadTopMargin( 0.16 );
        shStyle->SetPadRightMargin( 0.16 );
    } else {
        shStyle->SetPadTopMargin( 0.05 );
        shStyle->SetPadRightMargin( 0.05 );
    }
    shStyle->SetPadBottomMargin( 0.16 );
    shStyle->SetPadLeftMargin( 0.16 );

    // set title offsets (for axis label)
    shStyle->SetTitleXOffset( 1.4 );
    shStyle->SetTitleYOffset( 1.4 );

    // use large fonts
    // Int_t font=72; // Helvetica italics
    Int_t    font  = 42;  // Helvetica
    Double_t tsize = 0.05;
    shStyle->SetTextFont( font );

    shStyle->SetTextSize( tsize );
    shStyle->SetLabelFont( font, "x" );
    shStyle->SetTitleFont( font, "x" );
    shStyle->SetLabelFont( font, "y" );
    shStyle->SetTitleFont( font, "y" );
    shStyle->SetLabelFont( font, "z" );
    shStyle->SetTitleFont( font, "z" );

    shStyle->SetLabelSize( tsize, "x" );
    shStyle->SetTitleSize( tsize, "x" );
    shStyle->SetLabelSize( tsize, "y" );
    shStyle->SetTitleSize( tsize, "y" );
    shStyle->SetLabelSize( tsize, "z" );
    shStyle->SetTitleSize( tsize, "z" );

    // use bold lines and markers
    shStyle->SetMarkerStyle( 20 );
    shStyle->SetMarkerSize( 1.2 );
    shStyle->SetHistLineWidth( 2. );
    shStyle->SetLineStyleString( 2, "[12 12]" );  // postscript dashes

    // get rid of X error bars
    // shStyle->SetErrorX(0.001);
    // get rid of error bar caps
    shStyle->SetEndErrorSize( 0. );

    // do not display any of the standard histogram decorations
    shStyle->SetOptTitle( 0 );
    // shStyle->SetOptStat(1111);
    shStyle->SetOptStat( 0 );
    // shStyle->SetOptFit(1111);
    shStyle->SetOptFit( 0 );

    // put tick marks on top and RHS of plots
    shStyle->SetPadTickX( 1 );
    shStyle->SetPadTickY( 1 );

    return shStyle;
}

void SetShStyle( int colerbar_flag = 0 )
{
    static TStyle *shStyle = 0;
    std::cout << "Applying SH style settings.." << std::endl;
    if ( shStyle == 0 ) {
        if ( colerbar_flag ) {
            shStyle = ShStyle( colerbar_flag );
            std::cout << "Color bar sytly is applied." << std::endl;
        } else {
            shStyle = ShStyle( );
        }
        gROOT->SetStyle( "ATLAS" );
        gROOT->ForceStyle( );
        SetColorBar( );
    }
}