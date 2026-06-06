#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TTree.h"

const int PEDESTAL_SAMPLES = 100;
const int CLOCK_MIN        = 0;
const int CLOCK_MAX        = 1024;

int main( int argc, char *argv[] )
{
    std::cout << "### Dat2Root ###" << std::endl;
    if ( argc < 2 ) {
        std::cerr << "Usage: " << argv[0] << "[-i <input_file.dat>] [-o <output_file.root>] [-c <config_file>] [-v]" << std::endl;
        return 1;
    }

    int         opt;
    std::string config_file;
    std::string input_file;
    std::string output_file;
    bool        verbose = false;

    while ( ( opt = getopt( argc, argv, "i:o:c:v" ) ) != -1 ) {
        switch ( opt ) {
        case 'i':
            std::cout << "Using input file: " << optarg << std::endl;
            input_file = optarg;
            break;
        case 'o':
            std::cout << "Using output file: " << optarg << std::endl;
            output_file = optarg;
            break;
        case 'c':
            std::cout << "Using config file: " << optarg << std::endl;
            config_file = optarg;
            break;
        case 'v':
            std::cout << "Verbose mode enabled." << std::endl;
            verbose = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << "[-i <input_file.dat>] [-o <output_file.root>] [-c <config_file>] [-v]" << std::endl;
            return 1;
        }
    }

    TFile *output = new TFile( output_file.c_str( ), "RECREATE" );
    if ( !output->IsOpen( ) ) {
        std::cerr << "Error: Could not open output file " << output_file << std::endl;
        return 1;
    }
    TTree *tree = new TTree( "tree", "Data from .dat file" );
    if ( !tree ) {
        std::cerr << "Error: Could not create TTree" << std::endl;
        return 1;
    }

    ULong64_t t_event_id      = -1;
    ULong64_t t_timestamp     = 0;
    ULong64_t t_timestamp_end = 0;
    double    t_pedestal_ch1 = 0.0, t_pedestal_ch2 = 0.0;
    int       t_p_max_ch1 = -10000, t_p_max_ch2 = -10000;
    int       t_p_min_ch1 = 10000, t_p_min_ch2 = 10000;
    int       t_p_sum_ch1 = 0, t_p_sum_ch2 = 0;
    int       t_t_max_ch1 = -100, t_t_max_ch2 = -100;
    int       t_t_min_ch1 = -100, t_t_min_ch2 = -100;
    tree->Branch( "event_id", &t_event_id, "event_id/l" );
    tree->Branch( "timestamp", &t_timestamp, "timestamp/l" );
    tree->Branch( "timestamp_end", &t_timestamp_end, "timestamp_end/l" );
    tree->Branch( "pedestal_ch1", &t_pedestal_ch1, "pedestal_ch1/D" );
    tree->Branch( "pedestal_ch2", &t_pedestal_ch2, "pedestal_ch2/D" );
    tree->Branch( "p_max_ch1", &t_p_max_ch1, "p_max_ch1/I" );
    tree->Branch( "p_max_ch2", &t_p_max_ch2, "p_max_ch2/I" );
    tree->Branch( "p_min_ch1", &t_p_min_ch1, "p_min_ch1/I" );
    tree->Branch( "p_min_ch2", &t_p_min_ch2, "p_min_ch2/I" );
    tree->Branch( "p_sum_ch1", &t_p_sum_ch1, "p_sum_ch1/I" );
    tree->Branch( "p_sum_ch2", &t_p_sum_ch2, "p_sum_ch2/I" );
    tree->Branch( "t_max_ch1", &t_t_max_ch1, "t_max_ch1/I" );
    tree->Branch( "t_max_ch2", &t_t_max_ch2, "t_max_ch2/I" );
    tree->Branch( "t_min_ch1", &t_t_min_ch1, "t_min_ch1/I" );
    tree->Branch( "t_min_ch2", &t_t_min_ch2, "t_min_ch2/I" );

    std::ifstream infile( input_file );
    if ( !infile.is_open( ) ) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    std::string line;
    int         info_count = 0;
    int         clock      = 0;
    while ( std::getline( infile, line ) ) {
        std::string tmp;
        int         event_id;
        double      unixtime;
        double      unixtime_end;

        if ( line.empty( ) )
            continue;
        if ( line[0] == '#' )
            info_count++;

        std::istringstream iss( line );
        if ( info_count == 1 ) {  // event_id unixtime
            iss >> tmp >> event_id >> unixtime;
            t_event_id  = event_id;
            t_timestamp = static_cast<ULong64_t>( unixtime );
            continue;
        } else if ( info_count == 2 ) {  // header line
            info_count++;
            continue;
        } else if ( info_count > 3 ) {  // write unixtime
            iss >> tmp >> unixtime_end;
            t_timestamp_end = static_cast<ULong64_t>( unixtime_end );
            t_pedestal_ch1 /= static_cast<double>( PEDESTAL_SAMPLES );
            t_pedestal_ch2 /= static_cast<double>( PEDESTAL_SAMPLES );

            tree->Fill( );

            t_pedestal_ch1 = t_pedestal_ch2 = 0.0;
            t_p_max_ch1 = t_p_max_ch2 = -10000;
            t_p_min_ch1 = t_p_min_ch2 = 10000;
            t_p_sum_ch1 = t_p_sum_ch2 = 0;
            t_t_max_ch1 = t_t_max_ch2 = -100;
            t_t_min_ch1 = t_t_min_ch2 = -100;
            clock                     = 0;
            info_count                = 0;  // reset for next event
            continue;
        }

        int ch1, ch2;
        iss >> ch1 >> ch2;
        if ( clock < PEDESTAL_SAMPLES ) {
            t_pedestal_ch1 += ch1;
            t_pedestal_ch2 += ch2;
        }
        if ( ch1 > t_p_max_ch1 ) {
            t_p_max_ch1 = ch1;
            t_t_max_ch1 = clock;
        }
        if ( ch2 > t_p_max_ch2 ) {
            t_p_max_ch2 = ch2;
            t_t_max_ch2 = clock;
        }
        if ( ch1 < t_p_min_ch1 ) {
            t_p_min_ch1 = ch1;
            t_t_min_ch1 = clock;
        }
        if ( ch2 < t_p_min_ch2 ) {
            t_p_min_ch2 = ch2;
            t_t_min_ch2 = clock;
        }
        t_p_sum_ch1 += ch1;
        t_p_sum_ch2 += ch2;
        clock++;
    }

    tree->Write( );
    output->Close( );

    std::cout << "Data successfully converted to ROOT format and saved to " << output_file << std::endl;
    std::cout << "### Dat2Root Done ###" << std::endl;

    return 0;
}