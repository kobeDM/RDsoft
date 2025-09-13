#include "../include/NAPStyle.h"
#include "../include/NAPUtil.h"

void drawPublicPlot( const std::string& inputFile, const std::string& detName )
{
    SetNAPStyle( );

    TFile file( inputFile.c_str( ) );
    TH1D* pHistEneRaw = dynamic_cast< TH1D* >( file.Get( "h_poraw" ) );
    if( pHistEneRaw == nullptr ) return;

    TH2D* pHist2DPhQ = dynamic_cast< TH2D* >( file.Get( "h_ph_q" ) );
    if( pHist2DPhQ == nullptr ) return;

    TGraphErrors* pGraph_212Po = dynamic_cast< TGraphErrors* >( file.Get( "g_po212_rate" ) );
    TGraphErrors* pGraph_214Po = dynamic_cast< TGraphErrors* >( file.Get( "g_po214_rate" ) );
    if( pGraph_212Po == nullptr || pGraph_214Po == nullptr ) return;

    pGraph_212Po->SetLineColor( kGreen + 1 );
    pGraph_214Po->SetLineColor( kMagenta + 1 );
    pGraph_212Po->SetMarkerColor( kGreen + 1 );
    pGraph_214Po->SetMarkerColor( kMagenta + 1 );
    
    TMultiGraph mg;
    mg.Add( pGraph_212Po, "P" );
    mg.Add( pGraph_214Po, "P" );
    
    TCanvas cvs( "cvs", "cvs", 800, 600 );
    mg.Draw( "A" );

    mg.GetXaxis( )->SetTitle( "Elapsed day (started from 2024/10/23 15:57)" );
    mg.GetYaxis( )->SetTitle( "Events / day" );
    
    // double maxRate =  pGraph_212Po->GetMaximum( ) > pGraph_214Po->GetMaximum( ) ? pGraph_212Po->GetMaximum( ) : pGraph_214Po->GetMaximum( );
    double maxRate = 0.0;
    for( int i = 0; i < pGraph_212Po->GetN( ); ++i ) {
        if( maxRate < pGraph_212Po->GetPointY( i ) ) maxRate = pGraph_212Po->GetPointY( i );
    }
    for( int i = 0; i < pGraph_214Po->GetN( ); ++i ) {
        if( maxRate < pGraph_214Po->GetPointY( i ) ) maxRate = pGraph_212Po->GetPointY( i );
    }
    mg.SetMaximum( maxRate * 1.7 );

    mg.Draw( "A" );

    if( pGraph_212Po->GetFunction( "func_po212" ) == nullptr ||
        pGraph_214Po->GetFunction( "func_po214" ) == nullptr ) return;

    double po212_rate     = pGraph_212Po->GetFunction( "func_po212" )->GetParameter( 0 );
    double po212_rate_err = pGraph_212Po->GetFunction( "func_po212" )->GetParError( 0 );
    double po214_rate     = pGraph_214Po->GetFunction( "func_po214" )->GetParameter( 0 );
    double po214_rate_err = pGraph_214Po->GetFunction( "func_po214" )->GetParError( 0 );
    
    TLegend* pLeg = NAPUtil::CreateLegend( 0.4, 0.75, 0.95, 0.92 );
    pLeg->AddEntry( pGraph_212Po, Form("^{212}Po (Th chain): %0.1lf #pm %0.1lf", po212_rate, po212_rate_err ), "lep" );
    pLeg->AddEntry( pGraph_214Po, Form("^{214}Po (U chain): %0.1lf #pm %0.1lf", po214_rate, po214_rate_err ), "lep" );
    pLeg->Draw( );    

    NAPUtil::CreateDrawText( 0.22, 0.85, "#bf{NEWAGE}" );
    NAPUtil::CreateDrawText( 0.22, 0.78, detName.c_str( ) );
    
    cvs.SaveAs( "rate_time.png" );
    cvs.SaveAs( "rate_time.pdf" );

    pHistEneRaw->GetXaxis( )->SetTitle( "Energy [MeV]" );
    pHistEneRaw->GetYaxis( )->SetTitle( "Entries / 0.06 MeV" );
    pHistEneRaw->GetYaxis( )->SetRangeUser( 0.0, pHistEneRaw->GetMaximum( ) * 1.5 );
    pHistEneRaw->SetMarkerStyle( 20 );
    pHistEneRaw->SetLineWidth( 3 );

    
    pHistEneRaw->Draw( "histe" );
    NAPUtil::CreateDrawText( 0.22, 0.85, "#bf{NEWAGE}" );
    NAPUtil::CreateDrawText( 0.22, 0.78, detName.c_str( ) );
    
    cvs.SaveAs( "energy.png" );
    cvs.SaveAs( "energy.pdf" );
        
    return;
}
