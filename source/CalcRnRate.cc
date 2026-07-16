#include <algorithm>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "TAxis.h"
#include "TBox.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TTree.h"

#include "../include/NAPStyle.h"

const int PEDESTAL_SAMPLES = 100;
const int ADC_MAX          = 4096;
const int CLOCK_MAX        = 1024;

const int WF_WIN_START = 480;
const int WF_WIN_END   = 580;

const double ENE_PO218 = 6.00235;
const double ENE_PO214 = 7.68682;
const double ENE_PO212 = 8.785;

const double T_RADON220 = 3.824 / log( 2.0 );

using UnixTimeRange = std::pair<ULong64_t, ULong64_t>;

bool IsExcludedTimeRange( ULong64_t start_time, ULong64_t end_time, const std::vector<UnixTimeRange> &exclude_ranges )
{
    for ( const auto &range : exclude_ranges ) {
        if ( range.first == 0 && range.second == 0 ) {
            continue;
        }
        if ( range.second < range.first ) {
            continue;
        }
        if ( end_time < range.first || start_time > range.second ) {
            continue;
        }
        return true;
    }
    return false;
}

double GetEffectiveExposureDays( double start_days, double end_days, ULong64_t run_start_time, const std::vector<UnixTimeRange> &exclude_ranges )
{
    if ( end_days <= start_days ) {
        return 0.0;
    }

    const double    seconds_per_day = 24.0 * 60.0 * 60.0;
    const ULong64_t start_abs       = static_cast<ULong64_t>( run_start_time + start_days * seconds_per_day );
    const ULong64_t end_abs         = static_cast<ULong64_t>( run_start_time + end_days * seconds_per_day );

    double effective_days = end_days - start_days;
    for ( const auto &range : exclude_ranges ) {
        if ( range.first == 0 && range.second == 0 ) {
            continue;
        }
        if ( range.second < range.first ) {
            continue;
        }

        const ULong64_t overlap_start = std::max( start_abs, range.first );
        const ULong64_t overlap_end   = std::min( end_abs, range.second );
        if ( overlap_end > overlap_start ) {
            effective_days -= static_cast<double>( overlap_end - overlap_start ) / seconds_per_day;
        }
    }

    return std::max( 0.0, effective_days );
}

int main( int argc, char *argv[] )
{
    SetShStyle( );

    std::cout << "### CalcRnRate ###" << std::endl;
    if ( argc < 2 ) {
        std::cerr << "Usage: " << argv[0] << "[-i <input_file>] [-o <output_directory>] [-c <config_file>] [-d <detector_name>] [-r <rate_directory>] [-v]" << std::endl;
        return 1;
    }

    int         opt;
    std::string input_file;
    std::string daq_config_file;
    std::string config_file;
    std::string output_directory;
    std::string detector_name;
    std::string monitor_directory;
    bool        verbose = false;
    while ( ( opt = getopt( argc, argv, "i:o:c:d:m:v" ) ) != -1 ) {
        switch ( opt ) {
        case 'i':
            std::cout << "Using input file: " << optarg << std::endl;
            input_file = optarg;
            break;
        case 'o':
            std::cout << "Using output directory: " << optarg << std::endl;
            output_directory = optarg;
            break;
        case 'c':
            std::cout << "Using analysis config file: " << optarg << std::endl;
            config_file = optarg;
            break;
        case 'v':
            std::cout << "Verbose mode enabled." << std::endl;
            verbose = true;
            break;
        case 'd':
            std::cout << "Using detector name: " << optarg << std::endl;
            detector_name = optarg;
            break;
        case 'm':
            std::cout << "Using monitor directory: " << optarg << std::endl;
            monitor_directory = optarg;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << "[-i <input_file>] [-o <output_directory>] [-c <config_file>] [-d <detector_name>] [-m <monitor_directory>] [-v]" << std::endl;
            return 1;
        }
    }

    boost::property_tree::ptree pt;
    read_json( config_file, pt );
    double dynamic_range   = pt.get<double>( "DAQ.DYNAMIC RANGE" );
    double sampling_hertz  = pt.get<double>( "DAQ.SAMPLING RATE" );
    int    sampling_number = pt.get<int>( "DAQ.SAMPLING NUMBER" );
    double daq_Vth         = pt.get<double>( "DAQ.TRIGGER THRESHOLD CH1" );

    double ene_reso                   = pt.get<double>( "ana.ene_reso" );
    double ene_win_upper              = pt.get<double>( "ana.ene_win_upper" );
    double ene_win_lower              = pt.get<double>( "ana.ene_win_lower" );
    double neg_veto_factor            = pt.get<double>( "ana.neg_veto_factor" );
    double twin_avg_start_us          = pt.get<double>( "ana.twin_avg_start_us" );
    double twin_avg_window_us         = pt.get<double>( "ana.twin_avg_window_us" );
    double time_win_hour              = pt.get<double>( "ana.time_win_hour" );
    double integ_win_start_in_days    = pt.get<double>( "ana.integ_win_start_in_days" );
    double integ_win_end_in_days      = pt.get<double>( "ana.integ_win_end_in_days" );
    double measurement_offset_in_days = pt.get<double>( "ana.measurement_offset_in_days" );
    double fit_win_start_in_days      = pt.get<double>( "ana.fit_win_start_in_days" );
    double fit_win_end_in_days        = pt.get<double>( "ana.fit_win_end_in_days" );
    double area_threshold             = pt.get<double>( "ana.area_threshold" );
    double pulse_height_threshold     = pt.get<double>( "ana.pulse_height_threshold" );
    double cal_a, cal_b;
    cal_a = pt.get<double>( "ana.cal_factor_a_" + detector_name );
    cal_b = pt.get<double>( "ana.cal_factor_b_" + detector_name );

    // Parse optional exclusion windows from the analysis config.
    // The implementation accepts either the new list-based format or the
    // legacy single-range fields for backward compatibility.
    std::vector<UnixTimeRange> exclude_ranges;
    const auto                 exclude_ranges_node = pt.get_child_optional( "ana.exclude_unixtime_ranges" );
    if ( exclude_ranges_node ) {
        for ( const auto &item : *exclude_ranges_node ) {
            const auto     &range_node  = item.second;
            const ULong64_t range_start = static_cast<ULong64_t>( range_node.get<double>( "start", 0.0 ) );
            const ULong64_t range_end   = static_cast<ULong64_t>( range_node.get<double>( "end", 0.0 ) );
            if ( range_start != 0 || range_end != 0 ) {
                exclude_ranges.emplace_back( range_start, range_end );
            }
        }
    }

    double show_rate_max = pt.get<double>( "view.show_rate_max" );

    double    Toffset      = 0;
    int       show_Po214   = pt.get<int>( "view.show_Po214" );
    int       show_Po218   = pt.get<int>( "view.show_Po218" );
    int       show_Po212   = pt.get<int>( "view.show_Po212" );
    int       spbin        = pt.get<int>( "view.show_sp_bin" );
    double    spmax        = pt.get<double>( "view.show_sp_max" );
    double    spmin        = pt.get<double>( "view.show_sp_min" );
    int       spMbin       = pt.get<int>( "view.show_sp_bin_MeV" );
    double    spMmax       = pt.get<double>( "view.show_sp_max_MeV" );
    double    spMmin       = pt.get<double>( "view.show_sp_min_MeV" );
    ULong64_t runstarttime = pt.get<double>( "DAQ.RUN START" );

    if ( verbose ) {

        std::cout << "=== Configuration ===" << std::endl;
        std::cout << "DYNAMIC RANGE          : " << dynamic_range << " (0=+/-25V,1=+/-2.5V)" << std::endl;
        std::cout << "SAMPLING RATE          : " << sampling_hertz << " (Hz)" << std::endl;
        std::cout << "SAMPLING NUMBER        : " << sampling_number << std::endl;
        std::cout << "TRIGGER THRESHOLD CH1  : " << daq_Vth << " (V)" << std::endl;
        std::cout << "ENERGY RESOLUTION      : " << ene_reso << " (%)" << std::endl;
        std::cout << "ENERGY WINDOW UPPER    : " << ene_win_upper << " (sigma)" << std::endl;
        std::cout << "ENERGY WINDOW LOWER    : " << ene_win_lower << " (sigma)" << std::endl;
        std::cout << "NEGATIVE VETO FACTOR   : " << neg_veto_factor << std::endl;
        std::cout << "TWIN AVG START         : " << twin_avg_start_us << " (us)" << std::endl;
        std::cout << "TWIN AVG WINDOW        : " << twin_avg_window_us << " (us)" << std::endl;
        std::cout << "TIME WINDOW HOUR       : " << time_win_hour << " (hour)" << std::endl;
        std::cout << "INTEG WIN START        : " << integ_win_start_in_days << " (days)" << std::endl;
        std::cout << "INTEG WIN END          : " << integ_win_end_in_days << " (days)" << std::endl;
        std::cout << "MEASUREMENT OFFSET     : " << measurement_offset_in_days << " (days)" << std::endl;
        std::cout << "FIT WIN START          : " << fit_win_start_in_days << " (days)" << std::endl;
        std::cout << "FIT WIN END            : " << fit_win_end_in_days << " (days)" << std::endl;
        std::cout << "AREA THRESHOLD         : " << area_threshold << std::endl;
        std::cout << "PULSE HEIGHT THRESHOLD : " << pulse_height_threshold << std::endl;
        std::cout << "EXCLUDE UNIXTIME RANGES: " << exclude_ranges.size( ) << std::endl;
        for ( std::size_t i = 0; i < exclude_ranges.size( ); ++i ) {
            std::cout << "  RANGE " << i << " : " << exclude_ranges[i].first << " - " << exclude_ranges[i].second << std::endl;
        }
        std::cout << "CAL FACTOR A " << detector_name << "      : " << cal_a << std::endl;
        std::cout << "CAL FACTOR B " << detector_name << "      : " << cal_b << std::endl;
        std::cout << "SHOW RATE MAX          : " << show_rate_max << std::endl;
        std::cout << "SHOW Po214             : " << show_Po214 << std::endl;
        std::cout << "SHOW Po218             : " << show_Po218 << std::endl;
        std::cout << "SHOW Po212             : " << show_Po212 << std::endl;
        std::cout << "SHOW SPECTRUM BIN      : " << spbin << std::endl;
        std::cout << "SHOW SPECTRUM MAX      : " << spmax << std::endl;
        std::cout << "SHOW SPECTRUM MIN      : " << spmin << std::endl;
        std::cout << "SHOW SPECTRUM BIN (MeV) : " << spMbin << std::endl;
        std::cout << "SHOW SPECTRUM MAX (MeV) : " << spMmax << std::endl;
        std::cout << "SHOW SPECTRUM MIN (MeV) : " << spMmin << std::endl;
        std::cout << "RUN START TIME         : " << runstarttime << std::endl;
        std::cout << "=====================" << std::endl;
    }

    TFile *input = TFile::Open( input_file.c_str( ), "READ" );
    if ( !input || input->IsZombie( ) ) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    TTree *tree = dynamic_cast<TTree *>( input->Get( "tree" ) );
    if ( !tree ) {
        std::cerr << "Error: Could not find TTree in input file " << input_file << std::endl;
        return 1;
    }

    Long64_t nentries = tree->GetEntries( );
    std::cout << "Number of entries in the tree: " << nentries << std::endl;

    ULong64_t t_event_id;
    ULong64_t t_timestamp;
    ULong64_t t_timestamp_end;
    double    t_pedestal;
    int       t_p_max;
    int       t_p_min;
    int       t_p_sum;
    int       t_t_max;
    int       t_t_min;
    short     t_ch[CLOCK_MAX];
    tree->SetBranchAddress( "event_id", &t_event_id );
    tree->SetBranchAddress( "timestamp", &t_timestamp );
    tree->SetBranchAddress( "timestamp_end", &t_timestamp_end );
    tree->SetBranchAddress( "pedestal_ch1", &t_pedestal );
    tree->SetBranchAddress( "p_max_ch1", &t_p_max );
    tree->SetBranchAddress( "p_min_ch1", &t_p_min );
    tree->SetBranchAddress( "p_sum_ch1", &t_p_sum );
    tree->SetBranchAddress( "t_max_ch1", &t_t_max );
    tree->SetBranchAddress( "t_min_ch1", &t_t_min );
    tree->SetBranchAddress( "ch1", t_ch );

    TH1D         *h_spectrum          = new TH1D( "h_spectrum", "h_spectrum", spMbin, spMmin, spMmax );
    TH1D         *h_po214             = new TH1D( "h_po214", "h_po214", spMbin, spMmin, spMmax );
    TH1D         *h_po218             = new TH1D( "h_po218", "h_po218", spMbin, spMmin, spMmax );
    TH1D         *h_po212             = new TH1D( "h_po212", "h_po212", spMbin, spMmin, spMmax );
    TH1D         *h_po214_roi         = new TH1D( "h_po214_roi", "h_po214_roi", spMbin, spMmin, spMmax );
    TH1D         *h_po218_roi         = new TH1D( "h_po218_roi", "h_po218_roi", spMbin, spMmin, spMmax );
    TH1D         *h_po212_roi         = new TH1D( "h_po212_roi", "h_po212_roi", spMbin, spMmin, spMmax );
    TH1D         *h_spectrum_ph_all   = new TH1D( "h_spectrum_ph_all", "h_spectrum_ph_all", spbin, spmin, spmax );
    TH1D         *h_spectrum_ph       = new TH1D( "h_spectrum_ph", "h_spectrum_ph", spbin, spmin, spmax );
    TH1D         *h_spectrum_area_all = new TH1D( "h_spectrum_area_all", "h_spectrum_area_all", spbin, 0, 100000 );
    TH1D         *h_spectrum_area     = new TH1D( "h_spectrum_area", "h_spectrum_area", spbin, 0, 100000 );
    TH2D         *h_pmin_pmax_all     = new TH2D( "h_pmin_pmax_all", "h_pmin_pmax_all", spbin, spmin, spmax, spbin, spmin, spmax );
    TH2D         *h_pmin_pmax         = new TH2D( "h_pmin_pmax", "h_pmin_pmax", spbin, spmin, spmax, spbin, spmin, spmax );
    TH2D         *h_ph_area_all       = new TH2D( "h_ph_area_all", "h_ph_area_all", spbin, spmin, spmax, spbin, 0, 100000 );
    TH2D         *h_ph_area           = new TH2D( "h_ph_area", "h_ph_area", spbin, spmin, spmax, spbin, 0, 100000 );
    TH1D         *h_ped               = new TH1D( "h_ped", "h_ped", spbin, -spmax * 0.1, spmax * 0.1 );
    TH2D         *h_waveform_all      = new TH2D( "h_waveform_all", "h_waveform_all", CLOCK_MAX, 0, CLOCK_MAX, ADC_MAX, -ADC_MAX * 0.5, ADC_MAX * 0.5 );
    TH2D         *h_waveform          = new TH2D( "h_waveform", "h_waveform", CLOCK_MAX, 0, CLOCK_MAX, ADC_MAX, -ADC_MAX * 0.5, ADC_MAX * 0.5 );
    TH2D         *h_waveform_us       = new TH2D( "h_waveform_us", "h_waveform_us", CLOCK_MAX, 0, CLOCK_MAX / sampling_hertz * 1e6, ADC_MAX, -1.0, 1.0 );
    TGraphErrors *tg_po214            = new TGraphErrors( );
    TGraphErrors *tg_po218            = new TGraphErrors( );
    TGraphErrors *tg_po212            = new TGraphErrors( );

    h_spectrum->Sumw2( );
    h_po214->Sumw2( );
    h_po218->Sumw2( );
    h_po212->Sumw2( );
    h_po214_roi->Sumw2( );
    h_po218_roi->Sumw2( );
    h_po212_roi->Sumw2( );
    h_spectrum_ph_all->Sumw2( );
    h_spectrum_ph->Sumw2( );
    h_spectrum_area_all->Sumw2( );

    const double po214_roi_min = ENE_PO214 * ( 1 - ene_win_lower * ene_reso / 100 );
    const double po218_roi_min = ENE_PO218 * ( 1 - ene_win_lower * ene_reso / 100 );
    const double po212_roi_min = ENE_PO212 * ( 1 - ene_win_lower * ene_reso / 100 );
    const double po214_roi_max = ENE_PO214 * ( 1 + ene_win_upper * ene_reso / 100 );
    const double po218_roi_max = ENE_PO218 * ( 1 + ene_win_upper * ene_reso / 100 );
    const double po212_roi_max = ENE_PO212 * ( 1 + ene_win_upper * ene_reso / 100 );

    const int n_bin_max                  = 65536;
    int       po214_count[n_bin_max]     = { 0 };
    int       po218_count[n_bin_max]     = { 0 };
    int       po212_count[n_bin_max]     = { 0 };
    int       po214_count_day[n_bin_max] = { 0 };
    int       po218_count_day[n_bin_max] = { 0 };
    int       po212_count_day[n_bin_max] = { 0 };

    double last_time_in_days = 0;
    for ( Long64_t i = 0; i < nentries; i++ ) {
        tree->GetEntry( i );
        double cal_factor = 0.001;  // Why not use dynamic range and ADC bits ... ?
        double area       = static_cast<double>( t_p_sum ) - static_cast<double>( PEDESTAL_SAMPLES ) * t_pedestal;
        double ph         = ( static_cast<double>( t_p_max ) - t_pedestal ) * cal_factor;
        double pmin       = ( static_cast<double>( t_p_min ) - t_pedestal ) * cal_factor;
        double pedestal   = t_pedestal * cal_factor;
        double ene        = cal_a * ph + cal_b;

        for ( int clock = 0; clock < CLOCK_MAX; clock++ )
            h_waveform_all->Fill( clock, static_cast<double>( t_ch[clock] ) - t_pedestal );

        // Reject events whose time span overlaps any excluded interval.
        // This keeps the selected sample consistent with the intended live time.
        const ULong64_t event_start = t_timestamp;
        const ULong64_t event_end   = t_timestamp_end > t_timestamp ? t_timestamp_end : t_timestamp;
        if ( IsExcludedTimeRange( event_start, event_end, exclude_ranges ) ) {
            continue;
        }

        // event selection
        const double cal_ph = dynamic_range / static_cast<double>( ADC_MAX ) * 0.5 * 1000.0;  // (V / (ADC * 0.001))
        h_spectrum_ph_all->Fill( ph * cal_ph );
        h_spectrum_area_all->Fill( area );
        h_pmin_pmax_all->Fill( pmin * cal_ph, ph * cal_ph );
        h_ped->Fill( pedestal * cal_ph );
        if ( area < area_threshold ) {
            continue;
        }
        if ( ph < pulse_height_threshold ) {
            continue;
        }
        if ( pmin < daq_Vth * neg_veto_factor ) {
            continue;
        }

        // time calculation
        double time_in_days = ( t_timestamp - runstarttime ) / ( 24. * 60. * 60. );
        int    bin_idx      = static_cast<int>( time_in_days / time_win_hour * 24. );
        int    day_idx      = static_cast<int>( time_in_days );

        // fill
        h_spectrum->Fill( ene );
        h_spectrum_ph->Fill( ph * cal_ph );
        h_spectrum_area->Fill( area );
        h_ph_area->Fill( ph * cal_ph, area );
        h_pmin_pmax->Fill( pmin * cal_ph, ph * cal_ph );
        for ( int clock = 0; clock < CLOCK_MAX; clock++ ) {
            double wf_ph   = ( static_cast<double>( t_ch[clock] ) - t_pedestal );
            double time_us = static_cast<double>( clock ) / sampling_hertz * 1e6;
            double wf_v    = wf_ph * cal_ph * cal_factor;
            h_waveform->Fill( clock, wf_ph );
            h_waveform_us->Fill( time_us, wf_v );
        }

        if ( ene > po214_roi_min && ene < po214_roi_max ) {
            h_po214->Fill( ene );
            if ( time_in_days > integ_win_start_in_days && time_in_days < integ_win_end_in_days ) {
                h_po214_roi->Fill( ene );
            }
            po214_count[bin_idx]++;
            po214_count_day[day_idx]++;
        }
        if ( ene > po218_roi_min && ene < po218_roi_max ) {
            h_po218->Fill( ene );
            if ( time_in_days > integ_win_start_in_days && time_in_days < integ_win_end_in_days ) {
                h_po218_roi->Fill( ene );
            }
            po218_count[bin_idx]++;
            po218_count_day[day_idx]++;
        }
        if ( ene > po212_roi_min && ene < po212_roi_max ) {
            h_po212->Fill( ene );
            if ( time_in_days > integ_win_start_in_days && time_in_days < integ_win_end_in_days ) {
                h_po212_roi->Fill( ene );
            }
            po212_count[bin_idx]++;
            po212_count_day[day_idx]++;
        }

        last_time_in_days = time_in_days;
    }

    std::cout << "Last time in days: " << last_time_in_days << std::endl;
    const double total_live_time_days = GetEffectiveExposureDays( 0.0, last_time_in_days, runstarttime, exclude_ranges );
    std::cout << "Effective livetime in days: " << total_live_time_days << std::endl;

    // Compute the effective exposure for each time bin by subtracting any
    // excluded intervals from the nominal bin width. The same correction is
    // applied to the rate errors so that both the values and uncertainties
    // are normalized to the remaining live time.
    int n_last_bin = static_cast<int>( last_time_in_days / time_win_hour * 24. ) + 1;
    for ( int bin = 0; bin < n_last_bin; bin++ ) {
        const double bin_start_days = static_cast<double>( bin ) * time_win_hour / 24.;
        const double bin_end_days   = ( bin == n_last_bin - 1 ) ? last_time_in_days : static_cast<double>( bin + 1 ) * time_win_hour / 24.;
        const double effective_time = GetEffectiveExposureDays( bin_start_days, bin_end_days, runstarttime, exclude_ranges );

        double time           = 0.5 * ( bin_start_days + bin_end_days );
        double time_err       = 0.5 * ( bin_end_days - bin_start_days );
        double rate_po214     = 0.0;
        double rate_po218     = 0.0;
        double rate_po212     = 0.0;
        double rate_po214_err = 0.0;
        double rate_po218_err = 0.0;
        double rate_po212_err = 0.0;

        if ( effective_time > 0.0 ) {
            rate_po214     = po214_count[bin] / effective_time;
            rate_po218     = po218_count[bin] / effective_time;
            rate_po212     = po212_count[bin] / effective_time;
            rate_po214_err = sqrt( po214_count[bin] ) / effective_time;
            rate_po218_err = sqrt( po218_count[bin] ) / effective_time;
            rate_po212_err = sqrt( po212_count[bin] ) / effective_time;
        }

        tg_po214->SetPoint( bin, time, rate_po214 );
        tg_po218->SetPoint( bin, time, rate_po218 );
        tg_po212->SetPoint( bin, time, rate_po212 );
        tg_po214->SetPointError( bin, time_err, rate_po214_err );
        tg_po218->SetPointError( bin, time_err, rate_po218_err );
        tg_po212->SetPointError( bin, time_err, rate_po212_err );
    }

    // For rate file
    const std::string rate_file_footer_po214 = "_po214.dat";
    const std::string rate_file_footer_po218 = "_po218.dat";
    const std::string rate_file_footer_po212 = "_po212.dat";

    // Write daily monitor files using the same effective exposure logic as the
    // time-bin rates so that the per-day outputs remain consistent with the
    // plotted rate values.
    int n_last_day_bin = static_cast<int>( last_time_in_days ) + 1;
    for ( int day = 0; day < n_last_day_bin; day++ ) {
        const double day_start_days = static_cast<double>( day );
        const double day_end_days   = ( day == n_last_day_bin - 1 ) ? last_time_in_days : static_cast<double>( day + 1 );
        const double effective_time = GetEffectiveExposureDays( day_start_days, day_end_days, runstarttime, exclude_ranges );

        double rate_po214     = 0.0;
        double rate_po218     = 0.0;
        double rate_po212     = 0.0;
        double rate_po214_err = 0.0;
        double rate_po218_err = 0.0;
        double rate_po212_err = 0.0;

        if ( effective_time > 0.0 ) {
            rate_po214     = po214_count_day[day] / effective_time;
            rate_po218     = po218_count_day[day] / effective_time;
            rate_po212     = po212_count_day[day] / effective_time;
            rate_po214_err = sqrt( po214_count_day[day] ) / effective_time;
            rate_po218_err = sqrt( po218_count_day[day] ) / effective_time;
            rate_po212_err = sqrt( po212_count_day[day] ) / effective_time;
        }

        std::time_t start_time = static_cast<std::time_t>( runstarttime + static_cast<ULong64_t>( day ) * 24 * 60 * 60 );
        std::tm    *tm         = std::localtime( &start_time );
        std::string date_str   = Form( "%04d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );

        std::string timestamp_str = Form( "%04d/%02d/%02d/00:00:00", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );

        std::string   file_path_po214 = Form( "%s/%s%s", monitor_directory.c_str( ), date_str.c_str( ), rate_file_footer_po214.c_str( ) );
        std::string   file_path_po218 = Form( "%s/%s%s", monitor_directory.c_str( ), date_str.c_str( ), rate_file_footer_po218.c_str( ) );
        std::string   file_path_po212 = Form( "%s/%s%s", monitor_directory.c_str( ), date_str.c_str( ), rate_file_footer_po212.c_str( ) );
        std::ofstream ofs_po214( file_path_po214, std::ios::out );
        std::ofstream ofs_po218( file_path_po218, std::ios::out );
        std::ofstream ofs_po212( file_path_po212, std::ios::out );
        if ( !ofs_po214.is_open( ) || !ofs_po218.is_open( ) || !ofs_po212.is_open( ) ) {
            std::cerr << "Error: Could not open output rate file for writing." << std::endl;
            return 1;
        }
        ofs_po214 << timestamp_str << " " << std::fixed << std::setprecision( 2 ) << rate_po214 << " " << rate_po214_err << std::endl;
        ofs_po218 << timestamp_str << " " << std::fixed << std::setprecision( 2 ) << rate_po218 << " " << rate_po218_err << std::endl;
        ofs_po212 << timestamp_str << " " << std::fixed << std::setprecision( 2 ) << rate_po212 << " " << rate_po212_err << std::endl;
    }

    // *******************
    // *** RnRate plot ***
    // *******************

    TCanvas *c_rate = new TCanvas( "c_rate", "Radon Rate", 600, 1200 );
    c_rate->Divide( 1, 3 );

    c_rate->cd( 1 );
    const int  n_panel = 12;
    TPaveText *pt_rnrate[n_panel];
    for ( int i = 0; i < n_panel; i++ ) {
        pt_rnrate[i] = new TPaveText( 0.1, 0.9 - i * 0.075, 0.9, 0.975 - i * 0.075, "NDC" );
        pt_rnrate[i]->SetTextColor( i == 0 ? kWhite : kBlack );
        pt_rnrate[i]->SetFillColor( i == 0 ? kBlack : kWhite );
        pt_rnrate[i]->SetBorderSize( 1 );
        pt_rnrate[i]->Draw( "SAME" );
    }

    // get execution time
    std::time_t now = std::time( nullptr );
    std::tm    *ltm = std::localtime( &now );
    char        time_str[64];
    std::strftime( time_str, sizeof( time_str ), "%Y-%m-%d %H:%M:%S", ltm );

    pt_rnrate[0]->AddText( Form( "Config" ) );
    pt_rnrate[1]->AddText( Form( "Data: %s", input_file.c_str( ) ) );
    pt_rnrate[2]->AddText( Form( "Analysis time: %s", time_str ) );
    pt_rnrate[3]->AddText( Form( "Detector: %s", detector_name.c_str( ) ) );
    pt_rnrate[4]->AddText( Form( "Calibration: slope = %.1f, intercept = %.1f", cal_a, cal_b ) );
    pt_rnrate[5]->AddText( Form( "Livetime: %.2f days", total_live_time_days ) );
    for ( int i = 6; i < n_panel; i++ ) {
        pt_rnrate[i]->AddText( Form( "N/A" ) );
    }

    c_rate->cd( 2 );
    h_spectrum->Scale( 1.0 / h_spectrum->GetBinWidth( 1.0 ) / total_live_time_days );
    h_po218->Scale( 1.0 / h_po218->GetBinWidth( 1.0 ) / total_live_time_days );
    h_po214->Scale( 1.0 / h_po214->GetBinWidth( 1.0 ) / total_live_time_days );
    h_po212->Scale( 1.0 / h_po212->GetBinWidth( 1.0 ) / total_live_time_days );
    h_po218_roi->Scale( 1.0 / h_po218_roi->GetBinWidth( 1.0 ) / ( integ_win_end_in_days - integ_win_start_in_days ) );
    h_po214_roi->Scale( 1.0 / h_po214_roi->GetBinWidth( 1.0 ) / ( integ_win_end_in_days - integ_win_start_in_days ) );
    h_po212_roi->Scale( 1.0 / h_po212_roi->GetBinWidth( 1.0 ) / ( integ_win_end_in_days - integ_win_start_in_days ) );
    h_po218->SetLineColor( kCyan + 2 );
    h_po214->SetLineColor( kMagenta + 2 );
    h_po212->SetLineColor( kGreen + 2 );
    h_po218_roi->SetLineColor( kCyan + 2 );
    h_po214_roi->SetLineColor( kMagenta + 2 );
    h_po212_roi->SetLineColor( kGreen + 2 );
    h_po218->SetFillColor( kCyan + 2 );
    h_po214->SetFillColor( kMagenta + 2 );
    h_po212->SetFillColor( kGreen + 2 );
    h_po218->SetFillStyle( 3001 );
    h_po214->SetFillStyle( 3001 );
    h_po212->SetFillStyle( 3001 );
    h_spectrum->GetXaxis( )->SetTitle( "Energy (MeV)" );
    h_spectrum->GetYaxis( )->SetTitle( "Counts / day / MeV" );
    h_spectrum->SetMarkerStyle( 0 );
    h_spectrum->Draw( "E" );
    h_po218->Draw( "HIST SAME" );
    h_po214->Draw( "HIST SAME" );
    h_po212->Draw( "HIST SAME" );
    h_po218_roi->Draw( "HIST SAME" );
    h_po214_roi->Draw( "HIST SAME" );
    h_po212_roi->Draw( "HIST SAME" );

    const double y_max           = 10000.0;
    double       po214_area_x[4] = { po214_roi_min, po214_roi_max, po214_roi_max, po214_roi_min };
    double       po214_area_y[4] = { 0, 0, y_max, y_max };
    TGraph      *tg_po214_area   = new TGraph( 4, po214_area_x, po214_area_y );
    tg_po214_area->SetFillColorAlpha( kMagenta + 2, 0.2 );
    tg_po214_area->SetFillStyle( 3001 );
    tg_po214_area->Draw( "F SAME" );
    double  po218_area_x[4] = { po218_roi_min, po218_roi_max, po218_roi_max, po218_roi_min };
    double  po218_area_y[4] = { 0, 0, y_max, y_max };
    TGraph *tg_po218_area   = new TGraph( 4, po218_area_x, po218_area_y );
    tg_po218_area->SetFillColorAlpha( kCyan + 2, 0.2 );
    tg_po218_area->SetFillStyle( 3001 );
    tg_po218_area->Draw( "F SAME" );
    double  po212_area_x[4] = { po212_roi_min, po212_roi_max, po212_roi_max, po212_roi_min };
    double  po212_area_y[4] = { 0, 0, y_max, y_max };
    TGraph *tg_po212_area   = new TGraph( 4, po212_area_x, po212_area_y );
    tg_po212_area->SetFillColorAlpha( kGreen + 2, 0.2 );
    tg_po212_area->SetFillStyle( 3001 );
    tg_po212_area->Draw( "F SAME" );

    TLegend *leg_sp = new TLegend( 0.2, 0.7, 0.4, 0.9 );
    leg_sp->AddEntry( h_po218, "Po-218", "f" );
    leg_sp->AddEntry( h_po214, "Po-214", "f" );
    leg_sp->AddEntry( h_po212, "Po-212", "f" );
    leg_sp->SetBorderSize( 0 );
    leg_sp->SetFillStyle( 0 );
    leg_sp->Draw( "SAME" );

    c_rate->cd( 3 );

    tg_po218->GetXaxis( )->SetTitle( "Elapsed days" );
    tg_po218->GetYaxis( )->SetTitle( "Event rate (events/day)" );
    tg_po218->SetMinimum( 0 );
    tg_po218->SetMaximum( show_rate_max );
    tg_po218->SetMarkerColor( kCyan + 2 );
    tg_po214->SetMarkerColor( kMagenta + 2 );
    tg_po212->SetMarkerColor( kGreen + 2 );
    tg_po218->SetMarkerColor( kCyan + 2 );
    tg_po214->SetMarkerColor( kMagenta + 2 );
    tg_po212->SetMarkerColor( kGreen + 2 );
    tg_po218->SetLineColor( kCyan + 2 );
    tg_po214->SetLineColor( kMagenta + 2 );
    tg_po212->SetLineColor( kGreen + 2 );
    tg_po218->SetLineWidth( 2 );
    tg_po214->SetLineWidth( 2 );
    tg_po212->SetLineWidth( 2 );
    tg_po218->Draw( "AP" );
    tg_po214->Draw( "P SAME" );
    tg_po212->Draw( "P SAME" );

    std::vector<double> mask_x;
    std::vector<double> mask_y;
    for ( const auto &range : exclude_ranges ) {
        if ( range.first == 0 && range.second == 0 ) {
            continue;
        }
        if ( range.second < range.first ) {
            continue;
        }

        const double x1 = std::max( 0.0, ( static_cast<double>( range.first - runstarttime ) ) / ( 24.0 * 60.0 * 60.0 ) );
        const double x2 = std::max( x1, std::min( last_time_in_days, ( static_cast<double>( range.second - runstarttime ) ) / ( 24.0 * 60.0 * 60.0 ) ) );
        if ( x2 <= x1 ) {
            continue;
        }

        mask_x.push_back( x1 );
        mask_y.push_back( 0.0 );
        mask_x.push_back( x2 );
        mask_y.push_back( 0.0 );
        mask_x.push_back( x2 );
        mask_y.push_back( show_rate_max );
        mask_x.push_back( x1 );
        mask_y.push_back( show_rate_max );
        mask_x.push_back( x1 );
        mask_y.push_back( 0.0 );
    }

    if ( !mask_x.empty( ) ) {
        TGraph *tg_mask = new TGraph( static_cast<int>( mask_x.size( ) ), mask_x.data( ), mask_y.data( ) );
        tg_mask->SetFillColor( kGray );
        tg_mask->SetFillStyle( 3001 );
        tg_mask->SetLineColor( kGray );
        tg_mask->Draw( "F SAME" );
    }

    TF1 *f_po218 = new TF1( "f_po218", "[0]*(1-exp(-(x+[2])/[1]))", fit_win_start_in_days, fit_win_end_in_days );
    f_po218->SetParameter( 0, show_rate_max );
    f_po218->FixParameter( 1, T_RADON220 );
    f_po218->FixParameter( 2, measurement_offset_in_days );
    TF1 *f_po214 = new TF1( "f_po214", "[0]*(1-exp(-(x+[2])/[1]))", fit_win_start_in_days, fit_win_end_in_days );
    f_po214->SetParameter( 0, show_rate_max );
    f_po214->FixParameter( 1, T_RADON220 );
    f_po214->FixParameter( 2, measurement_offset_in_days );
    TF1 *f_po212 = new TF1( "f_po212", "[0]", fit_win_start_in_days, fit_win_end_in_days );

    tg_po218->Fit( f_po218, "Q", "", fit_win_start_in_days, fit_win_end_in_days );
    tg_po214->Fit( f_po214, "Q", "", fit_win_start_in_days, fit_win_end_in_days );
    tg_po212->Fit( f_po212, "Q", "", fit_win_start_in_days, fit_win_end_in_days );
    f_po218->Draw( "SAME" );
    f_po214->Draw( "SAME" );
    f_po212->Draw( "SAME" );

    const double po218_const     = f_po218->GetParameter( 0 );
    const double po218_const_err = f_po218->GetParError( 0 );
    const double po214_const     = f_po214->GetParameter( 0 );
    const double po214_const_err = f_po214->GetParError( 0 );
    const double po212_const     = f_po212->GetParameter( 0 );
    const double po212_const_err = f_po212->GetParError( 0 );
    std::cout << "Po-218 const: " << po218_const << " +/- " << po218_const_err << std::endl;
    std::cout << "Po-214 const: " << po214_const << " +/- " << po214_const_err << std::endl;
    std::cout << "Po-212 const: " << po212_const << " +/- " << po212_const_err << std::endl;

    TLatex *latex = new TLatex( );
    latex->SetNDC( );
    latex->SetTextSize( 0.04 );
    latex->SetTextColor( kCyan + 2 );
    latex->DrawLatex( 0.2, 0.8, Form( "Po-218: %.2f #pm %.2f", po218_const, po218_const_err ) );
    latex->SetTextColor( kMagenta + 2 );
    latex->DrawLatex( 0.2, 0.75, Form( "Po-214: %.2f #pm %.2f", po214_const, po214_const_err ) );
    latex->SetTextColor( kGreen + 2 );
    latex->DrawLatex( 0.2, 0.70, Form( "Po-212: %.2f #pm %.2f", po212_const, po212_const_err ) );

    f_po218->SetLineColor( kCyan + 2 );
    f_po214->SetLineColor( kMagenta + 2 );
    f_po212->SetLineColor( kGreen + 2 );
    f_po218->SetLineWidth( 2 );
    f_po214->SetLineWidth( 2 );
    f_po212->SetLineWidth( 2 );
    f_po218->Draw( "SAME" );
    f_po214->Draw( "SAME" );
    f_po212->Draw( "SAME" );

    // **********************
    // *** Parameter plot ***
    // **********************

    TCanvas *c_vis = new TCanvas( "c_vis", "Visualization", 1200, 900 );
    c_vis->Divide( 3, 3 );
    c_vis->cd( 1 );
    TPaveText *pt_vis[n_panel];
    for ( int i = 0; i < n_panel; i++ ) {
        pt_vis[i] = new TPaveText( 0.1, 0.9 - i * 0.075, 0.9, 0.975 - i * 0.075, "NDC" );
        pt_vis[i]->SetTextColor( i == 0 ? kWhite : kBlack );
        pt_vis[i]->SetFillColor( i == 0 ? kBlack : kWhite );
        pt_vis[i]->SetBorderSize( 1 );
        pt_vis[i]->Draw( "SAME" );
    }
    pt_vis[0]->AddText( Form( "Config" ) );
    pt_vis[1]->AddText( Form( "Data: %s", input_file.c_str( ) ) );
    pt_vis[2]->AddText( Form( "Analysis time: %s", time_str ) );
    pt_vis[3]->AddText( Form( "Detector: %s", detector_name.c_str( ) ) );
    pt_vis[4]->AddText( Form( "Livetime: %.2f days", total_live_time_days ) );
    for ( int i = 5; i < n_panel; i++ ) {
        pt_vis[i]->AddText( Form( "N/A" ) );
    }

    c_vis->cd( 2 );
    h_spectrum_ph_all->GetXaxis( )->SetTitle( "Pulse height (V)" );
    h_spectrum_ph_all->GetYaxis( )->SetTitle( "Counts" );
    h_spectrum_ph_all->SetLineColor( kBlue + 2 );
    h_spectrum_ph_all->SetFillColor( kBlue + 2 );
    h_spectrum_ph->SetLineColor( kRed + 2 );
    h_spectrum_ph->SetFillColor( kRed + 2 );
    h_spectrum_ph_all->SetFillStyle( 3001 );
    h_spectrum_ph->SetFillStyle( 3001 );
    h_spectrum_ph_all->Draw( "HIST" );
    h_spectrum_ph->Draw( "HIST SAME" );

    TLegend *leg_ph = new TLegend( 0.6, 0.8, 0.9, 0.9 );
    leg_ph->AddEntry( h_spectrum_ph_all, "All events", "f" );
    leg_ph->AddEntry( h_spectrum_ph, "Selected events", "f" );
    leg_ph->SetBorderSize( 0 );
    leg_ph->SetFillStyle( 0 );
    leg_ph->Draw( "SAME" );

    c_vis->cd( 3 );
    c_vis->cd( 3 )->SetRightMargin( 0.12 );
    h_spectrum_area_all->GetXaxis( )->SetTitle( "Area (ADC counts)" );
    h_spectrum_area_all->GetYaxis( )->SetTitle( "Counts" );
    h_spectrum_area_all->SetLineColor( kBlue + 2 );
    h_spectrum_area_all->SetFillColor( kBlue + 2 );
    h_spectrum_area->SetLineColor( kRed + 2 );
    h_spectrum_area->SetFillColor( kRed + 2 );
    h_spectrum_area_all->SetFillStyle( 3001 );
    h_spectrum_area->SetFillStyle( 3001 );
    h_spectrum_area_all->Draw( "HIST" );
    h_spectrum_area->Draw( "HIST SAME" );
    leg_ph->Draw( "SAME" );

    c_vis->cd( 4 );
    h_pmin_pmax_all->GetXaxis( )->SetTitle( "Minimum pulse height (V)" );
    h_pmin_pmax_all->GetYaxis( )->SetTitle( "Maximum pulse height (V)" );
    h_pmin_pmax_all->SetMarkerColor( kBlue + 2 );
    h_pmin_pmax_all->SetLineColor( kBlue + 2 );
    h_pmin_pmax->SetMarkerColor( kRed + 2 );
    h_pmin_pmax->SetLineColor( kRed + 2 );
    h_pmin_pmax_all->SetMarkerStyle( 1 );
    h_pmin_pmax->SetMarkerStyle( 1 );
    h_pmin_pmax_all->Draw( "P" );
    h_pmin_pmax->Draw( "P SAME" );

    TLegend *leg_pminpmax = new TLegend( 0.6, 0.8, 0.9, 0.9 );
    leg_pminpmax->AddEntry( h_pmin_pmax_all, "All events", "p" );
    leg_pminpmax->AddEntry( h_pmin_pmax, "Selected events", "p" );
    leg_pminpmax->SetBorderSize( 0 );
    leg_pminpmax->SetFillStyle( 0 );
    leg_pminpmax->Draw( "SAME" );

    c_vis->cd( 5 );
    h_ph_area_all->GetXaxis( )->SetTitle( "Pulse height (V)" );
    h_ph_area_all->GetYaxis( )->SetTitle( "Area (ADC counts)" );
    h_ph_area_all->SetMarkerColor( kBlue + 2 );
    h_ph_area_all->SetLineColor( kBlue + 2 );
    h_ph_area->SetMarkerColor( kRed + 2 );
    h_ph_area->SetLineColor( kRed + 2 );
    h_ph_area_all->SetMarkerStyle( 1 );
    h_ph_area->SetMarkerStyle( 1 );
    h_ph_area_all->Draw( "P" );
    h_ph_area->Draw( "P SAME" );
    leg_pminpmax->Draw( "SAME" );

    c_vis->cd( 6 );
    h_ped->GetXaxis( )->SetTitle( "Pedestal (V)" );
    h_ped->GetYaxis( )->SetTitle( "Counts" );
    h_ped->SetLineColor( kBlue + 2 );
    h_ped->SetFillColor( kBlue + 2 );
    h_ped->SetFillStyle( 3001 );
    h_ped->Draw( "HIST" );

    c_vis->cd( 7 );
    c_vis->cd( 7 )->SetRightMargin( 0.11 );
    c_vis->cd( 7 )->SetGridy( );
    h_waveform_all->GetXaxis( )->SetTitle( Form( "Clock (%d MHz)", static_cast<int>( sampling_hertz * 1e-6 ) ) );
    h_waveform_all->GetYaxis( )->SetTitle( "ADC counts" );
    h_waveform_all->GetXaxis( )->SetRangeUser( WF_WIN_START, WF_WIN_END );
    h_waveform_all->SetLineColor( kBlue + 2 );
    h_waveform_all->Draw( "COLZ" );

    c_vis->cd( 8 );
    c_vis->cd( 8 )->SetRightMargin( 0.11 );
    c_vis->cd( 8 )->SetGridy( );
    h_waveform->GetXaxis( )->SetTitle( Form( "Clock (%d MHz)", static_cast<int>( sampling_hertz * 1e-6 ) ) );
    h_waveform->GetYaxis( )->SetTitle( "ADC counts" );
    h_waveform->GetXaxis( )->SetRangeUser( WF_WIN_START, WF_WIN_END );
    h_waveform->SetLineColor( kRed + 2 );
    h_waveform->Draw( "COLZ" );

    c_vis->cd( 9 );
    c_vis->cd( 9 )->SetRightMargin( 0.11 );
    c_vis->cd( 9 )->SetGridy( );
    h_waveform_us->GetXaxis( )->SetTitle( "Time (#mus)" );
    h_waveform_us->GetYaxis( )->SetTitle( "Pulse height (V)" );
    h_waveform_us->GetXaxis( )->SetRangeUser( WF_WIN_START / sampling_hertz * 1e6, WF_WIN_END / sampling_hertz * 1e6 );
    h_waveform_us->SetLineColor( kRed + 2 );
    h_waveform_us->Draw( "COLZ" );

    c_rate->SaveAs( Form( "%s/rnrate.png", output_directory.c_str( ) ) );
    // c_rate->SaveAs( Form( "%s/rnrate_%s.pdf", output_directory.c_str( ), input_file.c_str( ) ) );

    c_vis->SaveAs( Form( "%s/vis.png", output_directory.c_str( ) ) );
    // c_vis->SaveAs( Form( "%s/vis_%s.pdf", output_directory.c_str( ), input_file.c_str( ) ) );

    TFile *output = TFile::Open( Form( "%s/plots.root", output_directory.c_str( ) ), "RECREATE" );
    if ( !output || output->IsZombie( ) ) {
        std::cerr << "Error: Could not open output file " << Form( "%s/plots.root", output_directory.c_str( ) ) << std::endl;
        return 1;
    }
    output->cd( );
    h_spectrum->Write( );
    h_po214->Write( );
    h_po218->Write( );
    h_po212->Write( );
    h_po214_roi->Write( );
    h_po218_roi->Write( );
    h_po212_roi->Write( );
    h_spectrum_ph_all->Write( );
    h_spectrum_ph->Write( );
    h_spectrum_area_all->Write( );
    h_spectrum_area->Write( );
    h_pmin_pmax_all->Write( );
    h_pmin_pmax->Write( );
    h_ph_area_all->Write( );
    h_ph_area->Write( );
    h_ped->Write( );
    tg_po214->Write( "tg_po214" );
    tg_po218->Write( "tg_po218" );
    tg_po212->Write( "tg_po212" );
    f_po214->Write( "f_po214" );
    f_po218->Write( "f_po218" );
    f_po212->Write( "f_po212" );
    h_waveform_all->Write( );
    h_waveform->Write( );
    output->Close( );

    std::cout << "### CalcRnRate Done ###" << std::endl;

    return 0;
}