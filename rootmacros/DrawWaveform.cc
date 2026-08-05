#include <iostream>

#include "TCanvas.h"
#include "TFile.h"
#include "TH2D.h"
#include "TTree.h"

void DrawWaveform( const char *fileName = "RD_chain.root" )
{
    TFile *file = TFile::Open( fileName );
    if ( !file || file->IsZombie( ) ) {
        std::cerr << "Cannot open file." << std::endl;
        return;
    }

    TTree *tree = (TTree *)file->Get( "tree" );

    Int_t   clock_max;
    Short_t ch1[100000];
    Short_t ch2[100000];

    tree->SetBranchAddress( "clock_max", &clock_max );
    tree->SetBranchAddress( "ch1", ch1 );
    tree->SetBranchAddress( "ch2", ch2 );

    const int adcMin = -500;
    const int adcMax = 500;

    Int_t    maxClock = 0;
    Long64_t nEntries = tree->GetEntries( );

    for ( Long64_t i = 0; i < nEntries; ++i ) {
        tree->GetEntry( i );
        if ( clock_max > maxClock )
            maxClock = clock_max;
    }

    TH2D *hCh1 = new TH2D( "hCh1", "CH1;Clock;ADC", maxClock, 0, maxClock, adcMax - adcMin + 1, adcMin - 0.5, adcMax + 0.5 );

    TH2D *hCh2 = new TH2D( "hCh2", "CH2;Clock;ADC", maxClock, 0, maxClock, adcMax - adcMin + 1, adcMin - 0.5, adcMax + 0.5 );

    for ( Long64_t ev = 0; ev < nEntries; ++ev ) {
        tree->GetEntry( ev );

        for ( int clk = 0; clk < clock_max; ++clk ) {
            hCh1->Fill( clk, ch1[clk] );
            hCh2->Fill( clk, ch2[clk] );
        }
    }

    TCanvas *c = new TCanvas( "c", "Waveforms", 1200, 600 );
    c->Divide( 2, 1 );

    c->cd( 1 );
    hCh1->Draw( "COLZ" );
    c->SaveAs( "wf_ch1.png" );

    c->cd( 2 );
    hCh2->Draw( "COLZ" );
    c->SaveAs( "wf_ch2.png" );
}