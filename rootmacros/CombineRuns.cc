#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "TAxis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TTree.h"

#include "../include/NAPStyle.h"

void CombineRuns( const std::string &run_list_file, const std::string &output_dir )
{
    SetShStyle( );

    std::vector<std::string> root_files;
    std::vector<std::string> config_files;

    std::ifstream infile( run_list_file );
    if ( !infile.is_open( ) ) {
        std::cerr << "Error: Could not open file " << run_list_file << std::endl;
        return;
    }

    std::string line;
    while ( std::getline( infile, line ) ) {
        if ( line.empty( ) || line[0] == '#' )
            continue;
        std::string root_file   = line + "/plots.root";
        std::string config_file = line + "/RD-anaconfig.json";

        root_files.push_back( root_file );
        config_files.push_back( config_file );
    }

    int                         fileIdx          = 0;
    Long64_t                    initial_unixtime = 0;
    std::vector<TGraphErrors *> tg_po214_vec;
    std::vector<TGraphErrors *> tg_po218_vec;
    std::vector<TGraphErrors *> tg_po212_vec;
    std::vector<ULong64_t>      elapsed_days_vec;

    for ( const auto &file : root_files ) {
        std::cout << "Processing file: " << file << std::endl;
        std::cout << "  Corresponding config file: " << config_files[fileIdx] << std::endl;

        TFile *f = TFile::Open( file.c_str( ) );
        if ( !f || f->IsZombie( ) ) {
            std::cerr << "Error: Could not open ROOT file " << file << std::endl;
            continue;
        }
        boost::property_tree::ptree pt;
        read_json( config_files[fileIdx], pt );
        Long64_t elapsed_time = 0;
        Long64_t unixtime     = static_cast<Long64_t>( pt.get<double>( "DAQ.RUN START" ) );
        if ( initial_unixtime == 0 ) {
            initial_unixtime = unixtime;
        } else {
            elapsed_time = unixtime - initial_unixtime;
        }
        elapsed_days_vec.push_back( elapsed_time / 24.0 / 60.0 / 60.0 );
        TGraphErrors *tg_po214 = dynamic_cast<TGraphErrors *>( f->Get( "tg_po214" ) );
        TGraphErrors *tg_po218 = dynamic_cast<TGraphErrors *>( f->Get( "tg_po218" ) );
        TGraphErrors *tg_po212 = dynamic_cast<TGraphErrors *>( f->Get( "tg_po212" ) );

        tg_po214_vec.push_back( tg_po214 );
        tg_po218_vec.push_back( tg_po218 );
        tg_po212_vec.push_back( tg_po212 );
        fileIdx++;
    }

    TGraphErrors *tg_po214_combined = new TGraphErrors( );
    TGraphErrors *tg_po218_combined = new TGraphErrors( );
    TGraphErrors *tg_po212_combined = new TGraphErrors( );

    for ( size_t i = 0; i < tg_po214_vec.size( ); ++i ) {
        for ( int j = 0; j < tg_po214_vec[i]->GetN( ); ++j ) {
            double x  = tg_po214_vec[i]->GetX( )[j] + elapsed_days_vec[i];
            double y  = tg_po214_vec[i]->GetY( )[j];
            double ex = tg_po214_vec[i]->GetEX( )[j];
            double ey = tg_po214_vec[i]->GetEY( )[j];
            int    n  = tg_po214_combined->GetN( );
            tg_po214_combined->SetPoint( n, x, y );
            tg_po214_combined->SetPointError( n, ex, ey );
        }
        for ( int j = 0; j < tg_po218_vec[i]->GetN( ); ++j ) {
            double x  = tg_po218_vec[i]->GetX( )[j] + elapsed_days_vec[i];
            double y  = tg_po218_vec[i]->GetY( )[j];
            double ex = tg_po218_vec[i]->GetEX( )[j];
            double ey = tg_po218_vec[i]->GetEY( )[j];
            int    n  = tg_po218_combined->GetN( );
            tg_po218_combined->SetPoint( n, x, y );
            tg_po218_combined->SetPointError( n, ex, ey );
        }
        for ( int j = 0; j < tg_po212_vec[i]->GetN( ); ++j ) {
            double x  = tg_po212_vec[i]->GetX( )[j] + elapsed_days_vec[i];
            double y  = tg_po212_vec[i]->GetY( )[j];
            double ex = tg_po212_vec[i]->GetEX( )[j];
            double ey = tg_po212_vec[i]->GetEY( )[j];
            int    n  = tg_po212_combined->GetN( );
            tg_po212_combined->SetPoint( n, x, y );
            tg_po212_combined->SetPointError( n, ex, ey );
        }
    }

    TCanvas *c_rnrate = new TCanvas( "c_rnrate", "Rn rate", 800, 600 );
    tg_po214_combined->SetLineColor( kMagenta + 2 );
    tg_po218_combined->SetLineColor( kCyan + 2 );
    tg_po212_combined->SetLineColor( kGreen + 2 );
    tg_po214_combined->SetMarkerColor( kMagenta + 2 );
    tg_po218_combined->SetMarkerColor( kCyan + 2 );
    tg_po212_combined->SetMarkerColor( kGreen + 2 );
    tg_po214_combined->GetXaxis( )->SetTitle( "Elapsed time (days)" );
    tg_po214_combined->GetYaxis( )->SetTitle( "Rn rate (counts/day)" );
    tg_po214_combined->SetMaximum( 10 );
    tg_po214_combined->Draw( "AP" );
    tg_po218_combined->Draw( "P SAME" );
    tg_po212_combined->Draw( "P SAME" );

    TLegend *leg = new TLegend( 0.7, 0.7, 0.9, 0.9 );
    leg->AddEntry( tg_po218_combined, "Po-218", "p" );
    leg->AddEntry( tg_po214_combined, "Po-214", "p" );
    leg->AddEntry( tg_po212_combined, "Po-212", "p" );
    leg->SetBorderSize( 0 );
    leg->SetFillStyle( 0 );
    leg->Draw( "SAME" );

    c_rnrate->SaveAs( ( output_dir + "/Rn_rate_combined.png" ).c_str( ) );

    return;
}