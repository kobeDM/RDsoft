//////////////////////////////////////////////////////////////////
//
// Utility
//
// Satoshi Higashino
// satoshi.higashino@cern.ch
//
//////////////////////////////////////////////////////////////////

#ifndef NAP_UTIL_H
#define NAP_UTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>

const double eneBin = 40;
const double eneMin = 0;
const double eneMax = 400;
const double lenBin = 100;
const double lenMin = 0;
const double lenMax = 5;
const double totBin = 100;
const double totMin = 0;
const double totMax = 10;
const double betaBin = 80;
const double betaMin = 0;
const double betaMax = 1000;
const double roundBin = 120;
const double roundMin = 0.0;
const double roundMax = 0.3;
const double alpha = 2.3;

namespace NAPUtil {

    // ---------------------------------------------------------------
    // Check directry
    // return:
    //    true ...exist 
    //    false...NOT exist 
    // comment:
    //    faster to use S_ISDIR( ) instead of opendir( )?
    bool ExistDir( const std::string& dirPath )
    {
        bool exist = false;
        DIR* dp = opendir( dirPath.c_str( ) );
        if( dp != nullptr ) {
            exist = true;
            closedir( dp );
        }
        return exist;
    }

    // ---------------------------------------------------------------
    // Check directry: if the directory is not found, create it
    void ExistCreateDir( const std::string& dirPath )
    {
        if( ExistDir( dirPath ) == true ) return;

        // create directory
        if( mkdir( dirPath.c_str( ), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) == 0 ) {
            std::string output = "create directory : " + dirPath;
            std::cout << "NAPUtil::ExistCreateDir ... INFO: " << output << std::endl;
        }
        return;
    }



    // ---------------------------------------------------------------
    // Progress Bar
    void PrintProgressBar( const int& index, const int& total )
    {
        if( index % 10 == 0 ) {
            std::string printBar = " [";
            double progress = static_cast< double >( index ) / static_cast< double >( total );
            for( int bar = 0; bar < 20; ++bar ) {
                double currentFraction = static_cast< double >( bar ) * 0.05;
                if( progress > currentFraction ) printBar += "/";
                else printBar += ".";
            }
            printBar += "] ";
            double percent = 100.0 * progress;
            std::stringstream percentSS;
            percentSS << std::setprecision( 2 ) << percent;
            std::string text = printBar + " ";
            text += percentSS.str( );
            std::cout << std::flush; 
            std::cout << text << "%\r" << std::flush; 
        }
        return;
    }

    // ---------------------------------------------------------------
    // Create and draw text in your plots
    //   note: Detail is shown in : https://root.cern.ch/doc/master/classTLatex.html
    //
    TLatex* CreateDrawText( const double&       x,
                            const double&       y,
                            const std::string&  text,
                            const double&       size = 0.05,
                            const Color_t&      color = kBlack )
    {
        if( text.size( ) <= 0 ) return nullptr;
        TLatex l;
        l.SetNDC( );
        l.SetTextColor( color );
        l.SetTextSize( size );
        return l.DrawLatex( x, y, text.c_str( ) );
    }

    // ---------------------------------------------------------------
    // Create legend in your plots
    //   note: Detail is shown in : https://root.cern.ch/doc/master/classTLegend.html
    //
    TLegend* CreateLegend( const double& x1,
                           const double& y1,
                           const double& x2,
                           const double& y2 )
    {
        TLegend* pLegend = new TLegend( x1, y1, x2, y2 );
        pLegend->SetFillStyle( 0 );
        pLegend->SetBorderSize( 0 );
        pLegend->SetTextFont( 42 );
        return pLegend;
    }
    
    // ---------------------------------------------------------------
    // Get cutline parameter from 
    //   energy: actual parameter
    //   cutLine: 
    //
    double getCutLine( const double& energy, double* cutLine )
    {
        int eneBinWidth = (eneMax - eneMin) / eneBin;
        int energyBin = static_cast< int >( energy / eneBinWidth );

        return cutLine[ energyBin ];
    }

    
}

#endif // NAP_UTIL_H
