#include <iostream>
#include <string>
#include <cmath>
#include <fstream>

#include "TCanvas.h"
#include "TFile.h"
#include "TF1.h"

const double DX = 1e-3;
const double CONFIDENCE_LEVEL = 0.90; // %
const double RANGE = 10.0; // Range for integration in terms of sigma

const std::string input_bg_file = "/mnt/nadb/nadb55/namai/newage/radon/RD3/ana/20251007_2/plots.root";
const std::string input_sample_file = "/mnt/nadb/nadb55/namai/newage/radon/RD3/ana/20250806_1/plots.root";

double gaussian(double x, double mu, double sigma)
{
    return exp(-0.5 * pow((x - mu) / sigma, 2)) / (sigma * sqrt(2 * M_PI));
}

double CalcUpperLimit(double mean, double sigma)
{
    const double x_min = mean - RANGE * sigma;
    const double x_max = mean + RANGE * sigma;

    double sum = 0.0;
    double gaus = 0.0;
    for (double x = x_min; x < x_max; x += DX)
    {
        gaus = gaussian(x, mean, sigma);
        if (x > 0)
        {
            sum += gaus * DX;
        }
    }

    double upper_limit = 0.0;
    for (double x = x_min; x < x_max; x += DX)
    {
        gaus = gaussian(x, mean, sigma);
        if (x > 0)
        {
            upper_limit += gaus * DX;
            if (upper_limit / sum >= CONFIDENCE_LEVEL)
            {
                return x;
            }
        }
    }

    return -1.0;
}

std::string GetDirFromPath(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        return path.substr(0, pos);

    }
    return path;
}

void SubtractBgRate()
{    
    TFile *bg_file = TFile::Open( input_bg_file.c_str(), "READ" );
    if ( !bg_file || bg_file->IsZombie() ) {
        std::cerr << "Error: Could not open input background file " << input_bg_file << std::endl;
        return;
    }
    TFile *sample_file = TFile::Open( input_sample_file.c_str(), "READ" );
    if ( !sample_file || sample_file->IsZombie() ) {
        std::cerr << "Error: Could not open input sample file " << input_sample_file << std::endl;
        return;
    }

    std::string output_dir = GetDirFromPath(input_sample_file);
    std::string output_path = output_dir + "/subtracted_rates.txt";
    std::cout << "Output file path: " << output_path << std::endl;
    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open output file for writing." << std::endl;
        return;
    }
    
    std::cout << "Opened background file: " << input_bg_file << std::endl;
    std::cout << "Opened sample file: " << input_sample_file << std::endl;
    output_file << "Opened background file: " << input_bg_file << std::endl;
    output_file << "Opened sample file: " << input_sample_file << std::endl;

    TF1 *f_bg_po218 = dynamic_cast<TF1 *>( bg_file->Get( "f_po218" ) );
    TF1 *f_bg_po214 = dynamic_cast<TF1 *>( bg_file->Get( "f_po214" ) );
    TF1 *f_bg_po212 = dynamic_cast<TF1 *>( bg_file->Get( "f_po212" ) );
    if ( !f_bg_po214 || !f_bg_po218 || !f_bg_po212 ) {
        std::cerr << "Error: Could not find background fit functions in background file " << input_bg_file << std::endl;
        return;
    }

    TF1 *f_sample_po218 = dynamic_cast<TF1 *>( sample_file->Get( "f_po218" ) );
    TF1 *f_sample_po214 = dynamic_cast<TF1 *>( sample_file->Get( "f_po214" ) );
    TF1 *f_sample_po212 = dynamic_cast<TF1 *>( sample_file->Get( "f_po212" ) );
    if ( !f_sample_po214 || !f_sample_po218 || !f_sample_po212 ) {
        std::cerr << "Error: Could not find sample fit functions in sample file " << input_sample_file << std::endl;
        return;
    }
    
    double bg_rate_po218 = f_bg_po218->GetParameter( 0 );
    double bg_rate_po214 = f_bg_po214->GetParameter( 0 );
    double bg_rate_po212 = f_bg_po212->GetParameter( 0 );
    double bg_rate_po218_err = f_bg_po218->GetParError( 0 );
    double bg_rate_po214_err = f_bg_po214->GetParError( 0 );
    double bg_rate_po212_err = f_bg_po212->GetParError( 0 );

    double sample_rate_po218 = f_sample_po218->GetParameter( 0 );
    double sample_rate_po214 = f_sample_po214->GetParameter( 0 );
    double sample_rate_po212 = f_sample_po212->GetParameter( 0 );
    double sample_rate_po218_err = f_sample_po218->GetParError( 0 );
    double sample_rate_po214_err = f_sample_po214->GetParError( 0 );
    double sample_rate_po212_err = f_sample_po212->GetParError( 0 );

    std::cout << "--------------------------" << std::endl;
    std::cout << "Background Po-218 rate: " << bg_rate_po218 << " +/- " << bg_rate_po218_err << " cpd" << std::endl;
    std::cout << "Background Po-214 rate: " << bg_rate_po214 << " +/- " << bg_rate_po214_err << " cpd" << std::endl;
    std::cout << "Background Po-212 rate: " << bg_rate_po212 << " +/- " << bg_rate_po212_err << " cpd" << std::endl;
    std::cout << "Sample Po-218 rate: " << sample_rate_po218 << " +/- " << sample_rate_po218_err << " cpd" << std::endl;
    std::cout << "Sample Po-214 rate: " << sample_rate_po214 << " +/- " << sample_rate_po214_err << " cpd" << std::endl;
    std::cout << "Sample Po-212 rate: " << sample_rate_po212 << " +/- " << sample_rate_po212_err << " cpd" << std::endl;
    output_file << "--------------------------" << std::endl;
    output_file << "Background Po-218 rate: " << bg_rate_po218 << " +/- " << bg_rate_po218_err << " cpd" << std::endl;
    output_file << "Background Po-214 rate: " << bg_rate_po214 << " +/- " << bg_rate_po214_err << " cpd" << std::endl;
    output_file << "Background Po-212 rate: " << bg_rate_po212 << " +/- " << bg_rate_po212_err << " cpd" << std::endl;
    output_file << "Sample Po-218 rate: " << sample_rate_po218 << " +/- " << sample_rate_po218_err << " cpd" << std::endl;
    output_file << "Sample Po-214 rate: " << sample_rate_po214 << " +/- " << sample_rate_po214_err << " cpd" << std::endl;
    output_file << "Sample Po-212 rate: " << sample_rate_po212 << " +/- " << sample_rate_po212_err << " cpd" << std::endl;

    double subtracted_rate_po218 = sample_rate_po218 - bg_rate_po218;
    double subtracted_rate_po214 = sample_rate_po214 - bg_rate_po214;
    double subtracted_rate_po212 = sample_rate_po212 - bg_rate_po212;
    double subtracted_rate_po218_err = sqrt( sample_rate_po218_err * sample_rate_po218_err + bg_rate_po218_err * bg_rate_po218_err );
    double subtracted_rate_po214_err = sqrt( sample_rate_po214_err * sample_rate_po214_err + bg_rate_po214_err * bg_rate_po214_err );
    double subtracted_rate_po212_err = sqrt( sample_rate_po212_err * sample_rate_po212_err + bg_rate_po212_err * bg_rate_po212_err );

    std::cout << "--------------------------" << std::endl;
    std::cout << "Subtracted rates (Sample - Background):" << std::endl;
    std::cout << "Subtracted Po-218 rate: " << subtracted_rate_po218 << " +/- " << subtracted_rate_po218_err << " cpd" << std::endl;
    std::cout << "Subtracted Po-214 rate: " << subtracted_rate_po214 << " +/- " << subtracted_rate_po214_err << " cpd" << std::endl;
    std::cout << "Subtracted Po-212 rate: " << subtracted_rate_po212 << " +/- " << subtracted_rate_po212_err << " cpd" << std::endl;
    output_file << "--------------------------" << std::endl;
    output_file << "Subtracted rates (Sample - Background):" << std::endl;
    output_file << "Subtracted Po-218 rate: " << subtracted_rate_po218 << " +/- " << subtracted_rate_po218_err << " cpd" << std::endl;
    output_file << "Subtracted Po-214 rate: " << subtracted_rate_po214 << " +/- " << subtracted_rate_po214_err << " cpd" << std::endl;
    output_file << "Subtracted Po-212 rate: " << subtracted_rate_po212 << " +/- " << subtracted_rate_po212_err << " cpd" << std::endl;

    double upper_limit_po218 = CalcUpperLimit(subtracted_rate_po218, subtracted_rate_po218_err);
    double upper_limit_po214 = CalcUpperLimit(subtracted_rate_po214, subtracted_rate_po214_err);
    double upper_limit_po212 = CalcUpperLimit(subtracted_rate_po212, subtracted_rate_po212_err);
    std::cout << "--------------------------" << std::endl;
    std::cout << "Upper limits at "<< CONFIDENCE_LEVEL * 100 << "% confidence level:" << std::endl;
    std::cout << "Upper limit Po-218: " << upper_limit_po218 << " cpd" << std::endl;
    std::cout << "Upper limit Po-214: " << upper_limit_po214 << " cpd" << std::endl;
    std::cout << "Upper limit Po-212: " << upper_limit_po212 << " cpd" << std::endl;
    output_file << "--------------------------" << std::endl;
    output_file << "Upper limits at "<< CONFIDENCE_LEVEL * 100 << "% confidence level:" << std::endl;
    output_file << "Upper limit Po-218: " << upper_limit_po218 << " cpd" << std::endl;
    output_file << "Upper limit Po-214: " << upper_limit_po214 << " cpd" << std::endl;
    output_file << "Upper limit Po-212: " << upper_limit_po212 << " cpd" << std::endl;

    output_file.close();
    std::cout << "---------------------------" << std::endl;
    std::cout << "Results written to " << output_path << std::endl;

    return;
}