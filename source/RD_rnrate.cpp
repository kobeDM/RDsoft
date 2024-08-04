//------------------------------------------------------------------
// Read tree ad2_daq output data
// Update: 07. July 2020
// Author: T.Shimada
//------------------------------------------------------------------

// STL
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <time.h>
// ROOT
#include "TROOT.h"
#include "TStyle.h"
#include "TTree.h"
#include "TF1.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TClassTable.h"
#include "TApplication.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TText.h"
#include "TBox.h"
#include "TLatex.h"
#include "TString.h"
#include "TLine.h"
// USER

// #define ADC_BIT 1024 //10bit

int main(int argc, char **argv)
{
    std::cerr<<"## RD_rnrate.cpp ##"<<std::endl;

    int batch_switch = 0;
    int verbose = 0;
    int auto_fitrange = 0;
    int opt;
    int det_id = 0;
    const double t_radon220 = 3.824 / log(2);
    const double ene_Po218 = 6.00235;
    const double ene_Po214 = 7.68682;
    const double ene_Po212 = 8.785;

    gStyle->SetPalette(kRainBow);

    if (argc < 3)
    {
        std::cout << "Usage:";
        std::cout << "RD_rnrate [-b] [-v] [-f] rootfile configfile [detectorID]" << std::endl;
        return 1;
    }

    while ((opt = getopt(argc, argv, "bvf")) != -1)
    {
        switch (opt)
        {
        case 'b':
            batch_switch = 1;
            printf("- Batch mode\n");
            break;
        case 'v':
            printf("- Verbose mode\n");
            verbose = 1;
            break;
        case 'f':
            printf("- Auto fiting mode\n");
            auto_fitrange = 1;
            break;
        default:
            printf("Usage: %s [-b] [-v] file.root config.json\n", argv[0]);
            break;
        }
    }

    if (verbose)
        std::cout << "optind=" << optind << std::endl;

    std::string infile = argv[1 + optind - 1];
    std::string outfile_tmp;
    TString infilename = argv[1 + optind - 1];

    // json database
    std::string conffilename = argv[2 + optind - 1];

    det_id = atoi(argv[3 + optind - 1]) - 1;
    std::cout << "Input file: " << infilename << std::endl;
    std::cout << "JSON config file: " << conffilename << std::endl;
    std::cout << "Detector: RD" << det_id + 1 << std::endl;

    std::string::size_type index_conf = conffilename.find(".json");
    if (index_conf == std::string::npos)
    {
        std::cout << "Error: Config file cannot open !!!" << std::endl;
        return 1;
    }

    // config read
    boost::property_tree::ptree pt;
    read_json(conffilename, pt);
    double dynamic_range = pt.get<double>("DAQ.DYNAMIC RANGE");
    double sampling_hertz = pt.get<double>("DAQ.SAMPLING RATE");
    int sampling_number = pt.get<int>("DAQ.SAMPLING NUMBER");
    double daq_Vth = pt.get<double>("DAQ.TRIGGER THRESHOLD CH1");

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
    double fit_win_start_in_days = pt.get<double>("ana.fit_win_start_in_days");
    double fit_win_end_in_days = pt.get<double>("ana.fit_win_end_in_days");
    double cal_a[3], cal_b[3];
    cal_a[0] = pt.get<double>("ana.cal_factor_a_RD1");
    cal_a[1] = pt.get<double>("ana.cal_factor_a_RD2");
    cal_a[2] = pt.get<double>("ana.cal_factor_a_RD3");
    cal_b[0] = pt.get<double>("ana.cal_factor_b_RD1");
    cal_b[1] = pt.get<double>("ana.cal_factor_b_RD2");
    cal_b[2] = pt.get<double>("ana.cal_factor_b_RD3");

    int twin = twin_avg_window_us * sampling_hertz / 1e6;
    ;
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

    if (verbose)
    {
        std::cout << "    Dynamic Range: " << dynamic_range << std::endl;
        std::cout << "    Sampling Rate: " << sampling_hertz << std::endl;
        std::cout << "    Sampling Number: " << sampling_number << std::endl;
        std::cout << "    Calibration Factor: " << cal_a[det_id] << std::endl;
        std::cout << "    Run Start: " << runstarttime << std::endl;
    }

    double tbin = time_win_hour / 24.; // days
    TString filename;
    std::string::size_type pos = infile.find(".root");
    outfile_tmp = infile.substr(0, pos);
    filename = outfile_tmp;
    TString outfilename = filename + "_vis.root";
    std::string mondir = "rnmon/";
    TString monfilename_214, monfilename_218, monfilename_212;
    TString monfilename_last_214 = "0";
    TString monfilename_last_218 = "0";
    TString monfilename_last_212 = "0";
    pos = outfile_tmp.find("/root");
    filename = outfile_tmp.substr(0, pos);
    TString ratefilename = filename + "/rate.dat";
    TString fitresfilename = filename + "/fitres.dat";
    TString rnrate_png = filename + "/rnrate.png";
    TString vis_png = filename + "/vis.png";

    if (verbose)
        std::cout << "Rate file: " << ratefilename << std::endl;
    std::cout << "Fitres file: " << fitresfilename << std::endl;
    std::cout << "Image file: " << rnrate_png << ", " << vis_png << std::endl;
    char tmpc[3][32];

    // TFile*
    std::ofstream out_file_rate;
    std::ofstream out_file_218;
    std::ofstream out_file_214;
    std::ofstream out_file_212;

    // Application
    TApplication app("app", &argc, argv);
    if (verbose)
        std::cout << "--- Application start ---" << std::endl;

    // #################################
    //  input file
    // #################################
    ULong64_t eventid;
    ULong64_t timestamp;
    UInt_t timestamp_usec;
    ULong64_t timestamp_end;
    UInt_t timestamp_usec_end;
    Float_t wf[sampling_number];

    TFile *in_file = TFile::Open(infilename, "read");
    TTree *tree = (TTree *)in_file->Get("tree");
    if (verbose)
        std::cout << "--- Read input file ---" << std::endl;
    tree->SetBranchAddress("eventid", &eventid);
    tree->SetBranchAddress("timestamp", &timestamp);
    tree->SetBranchAddress("timestamp_usec", &timestamp_usec);
    tree->SetBranchAddress("timestamp_end", &timestamp_end);
    tree->SetBranchAddress("timestamp_usec_end", &timestamp_usec_end);
    tree->SetBranchAddress("wf", wf);

    // ##############################
    //  Histograms & Graphs
    // ##############################
    TH1D *h_ph = new TH1D("h_ph", "h_ph", spbin, spmin, spmax);
    TH1D *h_ph_nc = new TH1D("h_ph_nc", "h_ph_nc", spbin, spmin, spmax);
    h_ph->GetXaxis()->SetTitle("pulse height(V)");
    h_ph->GetYaxis()->SetTitle("counts");
    h_ph->SetLineColor(2);
    h_ph_nc->GetXaxis()->SetTitle("pulse height(V)");
    h_ph_nc->GetYaxis()->SetTitle("counts");
    h_ph_nc->SetLineColor(4);
    TH1D *h_q = new TH1D("h_q", "h_q", spbin, -dynamic_range * 100, dynamic_range * 100);
    h_q->GetXaxis()->SetTitle("charge");
    h_q->GetYaxis()->SetTitle("counts");
    TH2D *h_ph_q = new TH2D("h_ph_q", "h_ph_q", spbin, -dynamic_range, dynamic_range, spbin, -dynamic_range * 100, dynamic_range * 100);
    h_ph_q->GetXaxis()->SetTitle("pulse height (V)");
    h_ph_q->GetYaxis()->SetTitle("charge");
    TH2D *h_wf = new TH2D("h_wf", "h_wf", sampling_number, 0, (double)sampling_number / sampling_hertz * 1e6, spbin, -dynamic_range, dynamic_range);
    h_wf->GetXaxis()->SetTitle("us");
    h_wf->GetYaxis()->SetTitle("volt");
    TH1D *h_ene = new TH1D("h_ene", "h_ene", spMbin, spMmin, spMmax);
    TH1D *h_poraw = new TH1D("h_poraw", "h_poraw", spMbin, spMmin, spMmax);
    TH1D *h_po212 = new TH1D("h_po212", "h_po212", spMbin, spMmin, spMmax);
    TH1D *h_po214 = new TH1D("h_po214", "h_po214", spMbin, spMmin, spMmax);
    TH1D *h_po218 = new TH1D("h_po218", "h_po218", spMbin, spMmin, spMmax);
    TH1D *h_po212_sel = new TH1D("h_po212_sel", "h_po212_sel", spMbin, spMmin, spMmax);
    TH1D *h_po214_sel = new TH1D("h_po214_sel", "h_po214_sel", spMbin, spMmin, spMmax);
    TH1D *h_po218_sel = new TH1D("h_po218_sel", "h_po218_sel", spMbin, spMmin, spMmax);

    int col_Po212 = kGreen + 1;
    int col_Po214 = kMagenta + 1;
    int col_Po218 = kAzure + 1;
    double markersize_rate = 1.2;
    double linewidth_rate = 2;
    int markerstyle_rate = 8;

    TGraphErrors *g_po212_rate = new TGraphErrors();
    g_po212_rate->SetName("g_po212_rate");
    TGraphErrors *g_po214_rate = new TGraphErrors();
    g_po214_rate->SetName("g_po214_rate");
    TGraphErrors *g_po218_rate = new TGraphErrors();
    g_po218_rate->SetName("g_po218_rate");

    if (verbose)
        std::cout << "--- Set histograms ---" << std::endl;

    TLine *l_inte_s = new TLine(twin_avg[0] / sampling_hertz * 1e6, -0.5, twin_avg[0] / sampling_hertz * 1e6, 0.5);
    TLine *l_inte_e = new TLine(twin_avg[1] / sampling_hertz * 1e6, -0.5, twin_avg[1] / sampling_hertz * 1e6, 0.5);
    l_inte_s->SetLineStyle(1);
    l_inte_e->SetLineStyle(1);
    l_inte_s->SetLineWidth(2);
    l_inte_e->SetLineWidth(2);
    l_inte_s->SetLineColor(kGreen + 2);
    l_inte_e->SetLineColor(kGreen + 2);

    // ##############################
    //  main loop
    // ##############################
    int binmax = 4000;
    int thisbin;
    double live = 0;
    double dead_time = 0;
    double real = 0;
    double ev_end_time;
    double ev_start_time;
    double cut_after_time;
    double po214_ev_time;
    int count_po214_ev = 0;
    int fill_po214_ev_count = 0;
    int fill_po214_count = 0;
    int fill_po214_flag = 0;
    double po218_ev_time;
    int count_po218_ev = 0;
    int fill_po218_ev_count = 0;
    int fill_po218_count = 0;
    int fill_po218_flag = 0;
    double po212_ev_time;
    int count_po212_ev = 0;
    int fill_po212_ev_count = 0;
    int fill_po212_count = 0;
    int fill_po212_flag = 0;
    int po214_tdep[binmax];
    int po218_tdep[binmax];
    int po212_tdep[binmax];
    int po214_time[binmax];
    int po218_time[binmax];
    int po212_time[binmax];
    ULong64_t first_ev_time;
    time_t t_temp;
    int vetocut = 1;
    int veto;
    double volt, offset, E;
    double V_avg = 0;
    double volt_max;
    double charge = 0;
    double veto_neg = neg_veto_factor * daq_Vth;
    double Eth_MeV = 2.0;

    struct tm *ptm;
    for (int i = 0; i < binmax; i++)
    {
        po214_tdep[i] = po218_tdep[i] = po212_tdep[i] = 0;
    }
    if (verbose)
        std::cout << "--- Loop start ---" << std::endl;
    int ev_max = tree->GetEntries();
    for (int ev = 0; ev < ev_max; ev++)
    {
        tree->GetEntry(ev);
        veto = 0;
        volt_max = -1 * dynamic_range;

        // live time calc
        if (ev == 0)
        {
            first_ev_time = timestamp;
            if (verbose)
            {
                std::cout << "First event unixtime: " << first_ev_time << std::endl;
                std::cout << "Run start unixtime: " << runstarttime << std::endl;
            }
            ev_start_time = timestamp;
            ev_end_time = timestamp_end;
            dead_time += timestamp * 1e6 + timestamp_usec - (timestamp_end * 1e6 + timestamp_usec_end);
        }
        else
        {
            ev_start_time = timestamp;
            ev_end_time = timestamp_end;
            dead_time += timestamp * 1e6 + timestamp_usec - (timestamp_end * 1e6 + timestamp_usec_end);
        }
        offset = 0;
        for (int i = twin_avg[0]; i < twin_avg[1]; i++)
        {
            offset += wf[i] / twin;
        }
        for (int samp = 0; samp < sampling_number; samp++)
        {
            volt = wf[samp];
            if (volt < veto_neg)
            {
                veto = 1;
            }
            if (volt > volt_max)
                volt_max = volt;
        }
        E = cal_a[det_id] * (volt_max - offset) + cal_b[det_id];

        // Fill
        for (int samp = 0; samp < sampling_number; samp++)
        {
            h_wf->Fill(samp / sampling_hertz * 1e6, wf[samp]);
        }
        h_ph_nc->Fill(volt_max - offset);
        if (veto == 0)
        {
            thisbin = int((timestamp - runstarttime) / 60. / 60 / 24 / tbin);
            h_ph->Fill(volt_max - offset);
            h_q->Fill(charge);
            h_ph_q->Fill(volt_max - offset, charge);
            if (E > Eth_MeV)
                h_ene->Fill(E);

            // po218
            if (ene_Po218 * (1 - ene_win_lower * ene_reso / 100) < E && E < ene_Po218 * (1 + ene_win_upper * ene_reso / 100))
            {
                h_po218->Fill(E);
                if (thisbin > -1 && thisbin < binmax)
                {
                    po218_tdep[thisbin]++;
                    po218_time[thisbin] = timestamp;
                    // if (verbose)
                    // {
                    //     std::cout << "Po218\t" << thisbin << "\t" << E << "\t" << po218_tdep[thisbin] << "\t" << timestamp << "\t" << runstarttime << "\t" << timestamp - runstarttime << std::endl;
                    // }
                }
                if ((timestamp - runstarttime) / 60. / 60 / 24 > integ_win_start_in_days && (timestamp - runstarttime) / 60. / 60 / 24 < integ_win_end_in_days)
                {
                    h_po218_sel->Fill(E);
                }
            }

            // po212
            if (ene_Po212 * (1 - ene_win_lower * ene_reso / 100) < E && E < ene_Po212 * (1 + ene_win_upper * ene_reso / 100))
            {
                h_po212->Fill(E);
                if (thisbin > -1 && thisbin < binmax)
                {
                    po212_tdep[thisbin]++;
                    po212_time[thisbin] = timestamp;
                    // if (verbose)
                    // {
                    //     std::cout << "Po212\t" << thisbin << "\t" << E << "\t" << po212_tdep[thisbin] << "\t" << timestamp << "\t" << runstarttime << "\t" << timestamp - runstarttime << std::endl;
                    // }
                }
                if ((timestamp - runstarttime) / 60. / 60 / 24 > integ_win_start_in_days && (timestamp - runstarttime) / 60. / 60 / 24 < integ_win_end_in_days)
                {
                    h_po212_sel->Fill(E);
                }
            }

            // Po214
            if (ene_Po214 * (1 - ene_win_lower * ene_reso / 100) < E && E < ene_Po214 * (1 + ene_win_upper * ene_reso / 100))
            {
                h_po214->Fill(E);
                if (thisbin > -1 && thisbin < binmax)
                {
                    po214_tdep[thisbin]++;
                    po214_time[thisbin] = timestamp;
                    // if (verbose)
                    // {
                    //     std::cout << "Po214\t" << thisbin << "\t" << E << "\t" << timestamp << "\t" << runstarttime << std::endl;
                    // }
                }
                if ((timestamp - runstarttime) / 60. / 60 / 24 > integ_win_start_in_days && (timestamp - runstarttime) / 60. / 60 / 24 < integ_win_end_in_days)
                {
                    h_po214_sel->Fill(E);
                }
            }

            // rate calc (previous event)
            // double rate;
            // if (timestamp == cut_after_time)
            // {
            //     rate = 1e10;
            // }
            // else
            // {
            //     rate = 1. / (timestamp - cut_after_time);
            // }
            // cut_after_time = ev_end_time;
            // if (ev % 10000 == 0)
            // {
            //     std::cout << "Ev." << ev << "/" << ev_max << " | Rate : " << std::setprecision(10) << rate << "\t\t\r" << std::flush;
            // }
        }
    }
    live = (ev_start_time - runstarttime) / 60. / 60 / 24; // days

    if (verbose)
        std::cout << "Live time: " << live << " days" << std::endl;
    if (verbose)
        std::cout << "Time bin: " << tbin << " days" << std::endl;

    // graph fill and output
    for (int i = 0; i < live / tbin; i++)
    {
        g_po212_rate->SetPoint(i, (i + 0.5) * tbin, po212_tdep[i] / tbin);
        g_po212_rate->SetPointError(i, tbin / 2, pow(po212_tdep[i], 0.5) / tbin);
        g_po214_rate->SetPoint(i, (i + 0.5) * tbin, po214_tdep[i] / tbin);
        g_po214_rate->SetPointError(i, tbin / 2, pow(po214_tdep[i], 0.5) / tbin);
        g_po218_rate->SetPoint(i, (i + 0.5) * tbin, po218_tdep[i] / tbin);
        g_po218_rate->SetPointError(i, tbin / 2, pow(po218_tdep[i], 0.5) / tbin);

        if (verbose)
        {
            if (i == 0) std::cout << "# bin\tPo212\tPo214\tPo218" << std::endl;
            std::cout << i << "\t" << po212_tdep[i] / tbin << "\t" << po214_tdep[i] / tbin << "\t" << po218_tdep[i] / tbin << std::endl;
        }

        t_temp = runstarttime + (i + 0.5) * tbin * 60 * 60 * 24;
        ptm = localtime(&t_temp);
        strftime(tmpc[1], sizeof(tmpc[1]), "%Y/%m/%d/%H:%M:%S", ptm);
        strftime(tmpc[2], sizeof(tmpc[2]), "rnmon/%Y/", ptm);
        mondir = tmpc[2];

        // output Po212
        strftime(tmpc[0], sizeof(tmpc[0]), "%Y%m%d_po212.dat", ptm);
        monfilename_212 = tmpc[0];
        monfilename_212 = mondir + monfilename_212;
        if (monfilename_212 != monfilename_last_212)
        {
            out_file_212.close();
            monfilename_last_212 = monfilename_212;
            out_file_212.open(monfilename_212, std::ios::out);
        }
        out_file_212 << tmpc[1] << " " << std::fixed << std::setprecision(2) << po212_tdep[i] / tbin << " " << pow(po212_tdep[i], 0.5) / tbin << std::endl;
     
        // output Po214
        strftime(tmpc[0], sizeof(tmpc[0]), "%Y%m%d_po214.dat", ptm);
        monfilename_214 = tmpc[0];
        monfilename_214 = mondir + monfilename_214;
        if (monfilename_214 != monfilename_last_214)
        {
            out_file_214.close();
            monfilename_last_214 = monfilename_214;
            out_file_214.open(monfilename_214, std::ios::out);
        }
        out_file_214 << tmpc[1] << " " << std::fixed << std::setprecision(2) << po214_tdep[i] / tbin << " " << pow(po214_tdep[i], 0.5) / tbin << std::endl;

        // output Po218
        strftime(tmpc[0], sizeof(tmpc[0]), "%Y%m%d_po218.dat", ptm);
     
        monfilename_218 = tmpc[0];
        monfilename_218 = mondir + monfilename_218;
        if (monfilename_218 != monfilename_last_218)
        {
            out_file_218.close();
            monfilename_last_218 = monfilename_218;
            out_file_218.open(monfilename_218, std::ios::out);
        }
        out_file_218 << tmpc[1] << " " << std::fixed << std::setprecision(2) << po218_tdep[i] / tbin << " " << pow(po218_tdep[i], 0.5) / tbin << std::endl;
    }

    // ##############################
    //  config panel
    // ##############################
    TPaveText *ptext[8];
    for (int i = 0; i < 8; i++)
    {
        ptext[7 - i] = new TPaveText(.05, i * 0.1 + 0.1, .95, i * 0.1 + 0.2);
    }
    ptext[0]->AddText("config");
    ptext[1]->AddText(Form("DATA: %s", infile.c_str()));
    ptext[2]->AddText(Form("DETECTOR: RD%d", det_id + 1));
    ptext[3]->AddText(Form("cal_a: %.1lf(MeV/V)", cal_a[det_id]));
    ptext[4]->AddText(Form("cal_b: %.1lf(MeV)", cal_b[det_id]));
    ptext[5]->AddText(Form("Live-time: %lf(days)", live));
    ptext[6]->AddText(Form("negative veto %.2lf V", veto_neg));
    if (verbose)
        std::cout << "--- config panel set ---" << std::endl;

    for (int i = 0; i < h_ene->GetNbinsX(); i++)
    {
        E = h_ene->GetBinCenter(i);
        int counts = h_ene->GetBinContent(i);
        for (int j = 0; j < counts; j++)
        {
            h_poraw->Fill(E);
        }
    }
    std::cerr << h_po212->Integral() << " " << live << std::endl;
    std::cerr << h_po214->Integral() << " " << live << std::endl;
    std::cerr << h_po218->Integral() << " " << live << std::endl;
    double po212_rate = double(h_po212->Integral()) / live;
    double po214_rate = double(h_po214->Integral()) / live;
    double po218_rate = double(h_po218->Integral()) / live;
    double po212_rate_sel = double(h_po212_sel->Integral()) / (integ_win_end_in_days - integ_win_start_in_days);
    double po214_rate_sel = double(h_po214_sel->Integral()) / (integ_win_end_in_days - integ_win_start_in_days);
    double po218_rate_sel = double(h_po218_sel->Integral()) / (integ_win_end_in_days - integ_win_start_in_days);
    double po212_rate_error = sqrt(double(h_po212->Integral())) / live;
    double po214_rate_error = sqrt(double(h_po214->Integral())) / live;
    double po218_rate_error = sqrt(double(h_po218->Integral())) / live;
    double po212_rate_sel_error = sqrt(double(h_po212_sel->Integral())) / (integ_win_end_in_days - integ_win_start_in_days);
    double po214_rate_sel_error = sqrt(double(h_po214_sel->Integral())) / (integ_win_end_in_days - integ_win_start_in_days);
    double po218_rate_sel_error = sqrt(double(h_po218_sel->Integral())) / (integ_win_end_in_days - integ_win_start_in_days);

    // scaling
    h_ene->GetXaxis()->SetTitle("MeV");
    h_ene->GetYaxis()->SetTitle("counts/MeV/day");
    h_ene->Sumw2();
    h_ene->Scale(1. / h_ene->GetBinWidth(1) / live);

    // #############################
    //  c_vis
    // #############################
    TCanvas *c_vis = new TCanvas("c_vis", "c_vis", 1000, 600);
    c_vis->Divide(3, 2);
    c_vis->cd(1);
    for (int i = 0; i < 8; i++)
    {
        ptext[i]->Draw();
    }
    c_vis->cd(2);
    h_wf->GetXaxis()->SetRangeUser(40, 60);
    h_wf->GetYaxis()->SetRangeUser(-2, 2);
    h_wf->Draw("colz");
    l_inte_s->Draw();
    l_inte_e->Draw();
    c_vis->cd(3);
    h_ph_nc->Draw();
    h_ph->Draw("same");
    c_vis->cd(4);
    h_ph_q->GetXaxis()->SetRangeUser(0, dynamic_range);
    h_ph_q->GetYaxis()->SetRangeUser(0, dynamic_range * 100);
    h_ph_q->Draw("colz");
    c_vis->cd(5);
    h_ene->Draw("hist e");
    c_vis->cd(6);
    h_q->GetXaxis()->SetRangeUser(0, dynamic_range * 100);
    h_q->Draw();

    TCanvas *c_rn = new TCanvas("c_radonrate", "c_radonrate", 480, 800);
    c_rn->Divide(1, 3);
    c_rn->cd(1);
    h_po212->SetLineColor(col_Po212);
    h_po212_sel->SetLineColor(col_Po212);
    h_po212_sel->SetFillColor(col_Po212);
    h_po214->SetLineColor(col_Po214);
    h_po214_sel->SetLineColor(col_Po214);
    h_po214_sel->SetFillColor(col_Po214);
    h_po218->SetLineColor(col_Po218);
    h_po218_sel->SetLineColor(col_Po218);
    h_po218_sel->SetFillColor(col_Po218);
    h_poraw->SetTitle("radon spectra");
    h_poraw->GetXaxis()->SetTitle("MeV");
    h_poraw->GetYaxis()->SetTitle("counts/MeV/day");
    h_poraw->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po212->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po214->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po218->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po212_sel->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po214_sel->Scale(1. / h_ene->GetBinWidth(1) / live);
    h_po218_sel->Scale(1. / h_ene->GetBinWidth(1) / live);

    h_poraw->Draw();
    if (show_Po212 == 1)
        h_po212->Draw("hist same");
    if (show_Po214 == 1)
        h_po214->Draw("hist same");
    if (show_Po218 == 1)
        h_po218->Draw("hist same");
    if (show_Po212 == 1)
        h_po212_sel->Draw("hist same");
    if (show_Po214 == 1)
        h_po214_sel->Draw("hist same");
    if (show_Po218 == 1)
        h_po218_sel->Draw("hist same");
    TLatex lat;
    lat.SetTextSize(0.06);
    float labelposx = spMmin + .05 * (spMmax - spMmin);
    lat.DrawLatex(labelposx, 0.9 * h_poraw->GetMaximum(), Form("DATA: %s", infile.c_str()));
    lat.DrawLatex(labelposx, 0.8 * h_poraw->GetMaximum(), Form("DETECTOR: RD%d", det_id + 1));
    lat.DrawLatex(labelposx, 0.7 * h_poraw->GetMaximum(), Form("cal_a: %.2lf", cal_a[det_id]));
    lat.DrawLatex(labelposx, 0.6 * h_poraw->GetMaximum(), Form("live time: %.2f days", live));
    c_rn->cd(2);
    c_rn->DrawFrame(0, 0, live * 1.1, show_rate_max, "Rn Rate;time(day);counts/day");
    g_po212_rate->SetLineColor(col_Po212);
    g_po212_rate->SetLineWidth(linewidth_rate);
    g_po212_rate->SetMarkerStyle(markerstyle_rate);
    g_po212_rate->SetMarkerSize(markersize_rate);
    g_po214_rate->SetLineColor(col_Po214);
    g_po214_rate->SetLineWidth(linewidth_rate);
    g_po214_rate->SetMarkerStyle(markerstyle_rate);
    g_po214_rate->SetMarkerSize(markersize_rate);
    g_po212_rate->SetMarkerColor(col_Po212);
    g_po214_rate->SetMarkerColor(col_Po214);
    g_po218_rate->SetLineColor(col_Po218);
    g_po218_rate->SetMarkerColor(col_Po218);
    g_po218_rate->SetMarkerSize(markersize_rate);
    g_po218_rate->SetLineWidth(linewidth_rate);
    g_po218_rate->SetMarkerStyle(markerstyle_rate);
    // if (show_Po212 == 1)
    if (show_Po212)
        g_po212_rate->Draw("p same");
    // if (show_Po214 == 1)
    if (show_Po214)
        g_po214_rate->Draw("p same");
    // if (show_Po218 == 1)
    if (show_Po218)
        g_po218_rate->Draw("p same");
    if (auto_fitrange) 
    {
        fit_win_end_in_days = live;
    }
    TF1 *func_po212 = new TF1("func_po212", "[0]", fit_win_start_in_days, fit_win_end_in_days);
    func_po212->SetLineColor(col_Po212);
    func_po212->SetLineWidth(linewidth_rate);
    if (show_Po212)
        g_po212_rate->Fit(func_po212, "", "", fit_win_start_in_days, fit_win_end_in_days);

    TF1 *func_po214 = new TF1("func_po214", "[0]*(1-exp(-(x+[2])/[1]))", fit_win_start_in_days, fit_win_end_in_days);
    func_po214->SetLineColor(col_Po214);
    func_po214->SetLineWidth(linewidth_rate);
    func_po214->SetParameter(0, show_rate_max);
    func_po214->FixParameter(1, t_radon220);
    func_po214->FixParameter(2, measurement_offset_in_days);
    if (show_Po214)
        g_po214_rate->Fit(func_po214, "", "", fit_win_start_in_days, fit_win_end_in_days);

    TF1 *func_po218 = new TF1("func_po218", "[0]*(1-exp(-(x+[2])/[1]))", fit_win_start_in_days, fit_win_end_in_days);
    func_po218->SetLineColor(kAzure + 1);
    func_po218->SetLineWidth(2);
    func_po218->SetParameter(0, show_rate_max);
    func_po218->FixParameter(1, t_radon220);
    func_po218->FixParameter(2, measurement_offset_in_days);
    if (show_Po218)
        g_po218_rate->Fit(func_po218, "", "", fit_win_start_in_days, fit_win_end_in_days);

    double po212_rate_fit = func_po212->GetParameter(0);
    double po212_rate_fit_err = func_po212->GetParError(0);
    double po214_rate_fit = func_po214->GetParameter(0);
    double po214_rate_fit_err = func_po214->GetParError(0);
    double po218_rate_fit = func_po218->GetParameter(0);
    double po218_rate_fit_err = func_po218->GetParError(0);

    TLegend *leg_rate = new TLegend(0.7, 0.7, 0.85, 0.85);
    leg_rate->AddEntry(g_po212_rate, "212Po+ rate", "pl");
    leg_rate->AddEntry(g_po214_rate, "214Po+ rate", "pl");
    leg_rate->AddEntry(g_po218_rate, "218Po+ rate", "pl");

    lat.SetTextSize(0.06);
    lat.DrawLatex(live * .05, 0.9 * show_rate_max, Form("DATA:%s", infile.c_str()));
    lat.DrawLatex(live * .05, 0.82 * show_rate_max, Form("DETECTOR:RD%d", det_id + 1));
    lat.SetTextColor(col_Po214);
    // if (show_Po214 == 1)
    if (show_Po214)
        lat.DrawLatex(live * .05, 0.74 * show_rate_max, Form("Po214: %.1f +- %.1f cpd.", po214_rate_fit, po214_rate_fit_err));
    lat.SetTextColor(col_Po218);
    // if (show_Po218 == 1)
    if (show_Po218)
        lat.DrawLatex(live * .05, 0.66 * show_rate_max, Form("Po218: %.1f +- %.1f cpd.", po218_rate_fit, po218_rate_fit_err));
    lat.SetTextColor(col_Po212);
    // if (show_Po212 == 1)
    if (show_Po212)
        lat.DrawLatex(live * .05, 0.58 * show_rate_max, Form("Po212: %.1f +- %.1f cpd.", po212_rate_fit, po212_rate_fit_err));

    c_rn->cd(3);
    c_rn->DrawFrame(0, 0, 1, 1, "Rn Rate");

    lat.SetTextSize(0.06);
    lat.SetTextColor(kBlack);
    lat.DrawLatex(0.1, 0.82, Form("212Po (all period) %.2lf +- %.2lf counts/day", po212_rate, po212_rate_error));
    lat.DrawLatex(.1, 0.74, Form("214Po (all period) %.2lf +- %.2lf counts/day", po214_rate, po214_rate_error));
    lat.DrawLatex(.1, 0.66, Form("218Po (all period)%.2lf +- %.2lf counts/day", po218_rate, po218_rate_error));
    lat.DrawLatex(.1, 0.58, Form("212Po (day%.1lf-%.1lf) %.2lf +- %.2lf counts/day", integ_win_start_in_days, integ_win_end_in_days, po212_rate_sel, po212_rate_sel_error));
    lat.DrawLatex(.1, 0.5, Form("214Po (day%.1lf-%.1lf) %.2lf +- %.2lf counts/day", integ_win_start_in_days, integ_win_end_in_days, po214_rate_sel, po214_rate_sel_error));
    lat.DrawLatex(.1, 0.42, Form("218Po (day%.1lf-%.1lf) %.2lf +- %.2lf counts/day", integ_win_start_in_days, integ_win_end_in_days, po218_rate_sel, po218_rate_sel_error));

    if (verbose)
        std::cout << "--- file output ---" << std::endl;
    TFile *out_file = TFile::Open(outfilename, "RECREATE");
    h_ph->Write();
    h_q->Write();
    h_ene->Write();
    h_ph_q->Write();
    h_poraw->Write();
    h_po212->Write();
    h_po214->Write();
    h_po218->Write();
    g_po212_rate->Write();
    g_po214_rate->Write();
    g_po218_rate->Write();
    TTree *info_tree = new TTree("info_tree", "info_tree");
    double info_live, info_cal, info_po212_rate, info_po214_rate, info_po218_rate;
    TString info_infilename;

    info_tree->Branch("live", &info_live);
    info_tree->Branch("cal", &info_cal);
    info_tree->Branch("po212_rate", &info_po212_rate);
    info_tree->Branch("po214_rate", &info_po214_rate);
    info_tree->Branch("po218_rate", &info_po218_rate);
    info_tree->Branch("infile", &info_infilename);
    info_live = live;
    info_cal = cal_a[det_id];
    info_po212_rate = po212_rate;
    info_po214_rate = po214_rate;
    info_po218_rate = po218_rate;
    info_infilename = infilename;
    info_tree->Fill();
    info_tree->Write();
    c_vis->Write();
    c_vis->Print(vis_png);
    c_rn->Write();
    c_rn->Print(rnrate_png);
    out_file->Close();

    out_file_rate.open(ratefilename, std::ios::out);
    out_file_rate << po214_rate_sel << "\t" << po214_rate_sel_error << "\t" << po218_rate_sel << "\t" << po218_rate_sel_error << "\t" << po212_rate_sel << "\t" << po212_rate_sel_error << "\t" << live << std::endl;
    // out_file_rate << po214_rate_fit << "\t" << po214_rate_fit_err << "\t" << po218_rate_fit << "\t" << po218_rate_fit_err << "\t" << po212_rate_fit << "\t" << po212_rate_fit_err << "\t" << live << std::endl;
    out_file_rate.close();
    out_file_rate.open(fitresfilename, std::ios::out);
    out_file_rate << "# po214_rate"
                  << "\t"
                  << "po214_rate_err"
                  << "\t"
                  << "po218_rate"
                  << "\t"
                  << "po218_rate_err"
                  << "\t"
                  << "po212_rate"
                  << "\t"
                  << "po212_rate_err"
                  << "\t"
                  << "livetime(days)" << std::endl;
    out_file_rate << po214_rate_fit << "\t" << po214_rate_fit_err << "\t" << po218_rate_fit << "\t" << po218_rate_fit_err << "\t" << po212_rate_fit << "\t" << po212_rate_fit_err << "\t" << live << std::endl;
    out_file_rate.close();

    // for monitoring
    if (!batch_switch)
    {
        app.Run();
    }

    return 0;
}
