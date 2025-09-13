#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "../include/NAPStyle.h"
#include "../include/NAPUtil.h"

void drawEneSpect( const std::string& inputFile, const std::string& configFile )
{
    SetNAPStyle( );

    // config read
    boost::property_tree::ptree pt;
    read_json( configFile, pt );
    double dynamic_range = pt.get<double>("DAQ.DYNAMIC RANGE");
    double sampling_hertz = pt.get<double>("DAQ.SAMPLING RATE");
    int sampling_number = pt.get<int>("DAQ.SAMPLING NUMBER");
    double daq_Vth = pt.get<double>("DAQ.TRIGGER THRESHOLD CH1");
    int det_id = pt.get<int>("DAQ.DETECTOR_ID");

    double ene_reso = pt.get<double>("ana.ene_reso");
    double ene_win_upper = pt.get<double>("ana.ene_win_upper");
    double ene_win_lower = pt.get<double>("ana.ene_win_lower");
    double neg_veto_factor = pt.get<double>("ana.neg_veto_factor");
    double twin_avg_start_us = pt.get<double>("ana.twin_avg_start_us");
    double twin_avg_window_us = pt.get<double>("ana.twin_avg_window_us");
    double time_win_minute = pt.get<double>("ana.time_win_minute");
    double time_win_hour = pt.get<double>("ana.time_win_hour");
    double integ_win_start_in_days = pt.get<double>("ana.integ_win_start_in_days");
    double integ_win_end_in_days = pt.get<double>("ana.integ_win_end_in_days");
    double measurement_offset_in_days = pt.get<double>("ana.measurement_offset_in_days");
    double ana_start_in_days = pt.get<double>("ana.ana_start_in_days");
    double ana_end_in_days = pt.get<double>("ana.ana_end_in_days");
    double fit_win_start_in_days = pt.get<double>("ana.fit_win_start_in_days");
    double fit_win_end_in_days = pt.get<double>("ana.fit_win_end_in_days");
    double area_threshold = pt.get<double>("ana.area_threshold");
    double pulse_height_threshold = pt.get<double>("ana.pulse_height_threshold");
    double cal_a[3], cal_b[3];
    cal_a[0] = pt.get<double>("ana.cal_factor_a_RD1");
    cal_a[1] = pt.get<double>("ana.cal_factor_a_RD2");
    cal_a[2] = pt.get<double>("ana.cal_factor_a_RD3");
    cal_b[0] = pt.get<double>("ana.cal_factor_b_RD1");
    cal_b[1] = pt.get<double>("ana.cal_factor_b_RD2");
    cal_b[2] = pt.get<double>("ana.cal_factor_b_RD3");

    double veto_neg = neg_veto_factor * daq_Vth;
    int twin = twin_avg_window_us * sampling_hertz / 1e6;

    int twin_avg[2];
    twin_avg[0] = twin_avg_start_us * sampling_hertz / 1e6;
    twin_avg[1] = twin_avg[0] + twin;

    double show_rate_max = pt.get<double>("view.show_rate_max");

    double Toffset = 0;
    int show_Po214 = pt.get<int>("view.show_Po214");
    int show_Po218 = pt.get<int>("view.show_Po218");
    int show_Po212 = pt.get<int>("view.show_Po212");
    int spbin = pt.get<int>("view.show_sp_bin");
    double spmax = pt.get<double>("view.show_sp_max");
    double spmin = pt.get<double>("view.show_sp_min");
    int spMbin = pt.get<int>("view.show_sp_bin_MeV");
    double spMmax = pt.get<double>("view.show_sp_max_MeV");
    double spMmin = pt.get<double>("view.show_sp_min_MeV");
    ULong64_t runstarttime = pt.get<double>("DAQ.RUN START");

    ULong64_t eventid = 0;
    ULong64_t timestamp = 0;
    UInt_t timestamp_usec = 0;
    ULong64_t timestamp_end = 0;
    UInt_t timestamp_usec_end = 0;
    Float_t wf[sampling_number];

    TFile file( inputFile.c_str( ) );
    TTree* pTree = dynamic_cast< TTree* >( file.Get( "tree" ) );
    if( pTree == nullptr ) return;

    pTree->SetBranchAddress("eventid", &eventid);
    pTree->SetBranchAddress("timestamp", &timestamp);
    pTree->SetBranchAddress("timestamp_usec", &timestamp_usec);
    pTree->SetBranchAddress("timestamp_end", &timestamp_end);
    pTree->SetBranchAddress("timestamp_usec_end", &timestamp_usec_end);
    pTree->SetBranchAddress("wf", wf);


    TH2D *h_wf = new TH2D("h_wf", "h_wf", sampling_number, 0, (double)sampling_number / sampling_hertz * 1e6, spbin, -dynamic_range, dynamic_range);
    h_wf->GetXaxis()->SetTitle("#musec.");
    h_wf->GetYaxis()->SetTitle("Voltage (V)");
    
    TH1D *pHistEne = new TH1D("histEne", "histEne", spMbin, spMmin, spMmax);
    
    int ev_max = pTree->GetEntries();
    for( int ev = 0; ev < ev_max; ++ev ) {
        pTree->GetEntry(ev);

        bool veto = false;
        double volt_max = -1.0 * dynamic_range;

        // calculate waveform offset
        double offset = 0.0;
        for( int i = twin_avg[0]; i < twin_avg[1]; i++ ) offset += wf[i] / twin;

        // calcutate adc-sum
        double volt_sum = 0.0;
        for( int i = 0; i < sampling_number; i++ ) volt_sum += wf[i] - offset;

        double volt = 0.0;
        for (int samp = 0; samp < sampling_number; samp++) {
            volt = wf[samp];
            if( volt < veto_neg ) veto = true; // veto under threshold
            if( volt > volt_max ) volt_max = volt;
        }

        // apply area threshold
        if( volt_sum < area_threshold ) veto = true;

        // apply pulse height threshold
        if( volt_max < pulse_height_threshold ) veto = true;

        double energy = cal_a[det_id - 1] * (volt_max - offset) + cal_b[det_id - 1];

        // Fill
        for( int samp = 0; samp < sampling_number; samp++ )
            if( veto == false ) h_wf->Fill(samp / sampling_hertz * 1e6, wf[samp]);


        if( veto == true ) continue;

        
        double event_time_day = (timestamp - runstarttime) / 60.0 / 60.0 / 24.0;
        if( ana_start_in_days > -0.01 && ana_end_in_days > -0.01 ) {
            std::cout << event_time_day << "\t" << ana_start_in_days << "\t" << ana_end_in_days << "\t" << energy << std::endl;
            if( event_time_day > ana_start_in_days && event_time_day < ana_end_in_days ) {
                if( energy > 2.0 ) pHistEne->Fill( energy );
            }
        }
        
    }
    
    TCanvas cvs( "cvs", "cvs", 800, 600 );
    pHistEne->GetXaxis( )->SetTitle( "Energy [MeV]" );
    pHistEne->GetYaxis( )->SetTitle( "Entries / 0.1 MeV" );
    pHistEne->GetYaxis( )->SetRangeUser( 0.0, pHistEne->GetMaximum( ) * 2.0 );
    pHistEne->SetMarkerStyle( 20 );
    pHistEne->SetLineWidth( 3 );
    pHistEne->Draw("pe");

    NAPUtil::CreateDrawText( 0.22, 0.85, "NEWAGE2025" );
    NAPUtil::CreateDrawText( 0.22, 0.78, Form( "RD-%d", det_id ) );
    NAPUtil::CreateDrawText( 0.55, 0.85, Form( "Elapsed time: %0.1lf days", ( ana_end_in_days - ana_start_in_days ) ) );

    cvs.SaveAs( "eneSpect.png" );
    cvs.SaveAs( "eneSpect.pdf" );
    
    return;
}
