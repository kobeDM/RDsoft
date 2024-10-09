#include "inc/shinclude.h"
#include "TGraphErrors.h"

void compRate()
{
    SetShStyle();

    const std::string data_dir = "/nadb/nadb55/namai/newage/03b/radonRate/ana/";
    const std::string outputDir = "/nadb/nadb55/namai/newage/03b/radonRate/ana/plot";
    const int sampleN = 13;
    const std::string sample_dirs[sampleN] = {
        "20240202",
        "20240222",
        "20240314",
        "20240329",
        "20240409",
        "20240416",
        "20240424",
        "20240509",
        "20240518",
        "20240603",
        "20240624",
        "20240702",
        "20240723"};

    std::vector<double> rate(sampleN);
    std::vector<double> rate_err(sampleN);
    std::vector<double> limit(sampleN);

    TCanvas *c_Po = new TCanvas("c_Po", "c_Po", 800, 600);
    TCanvas *c_Rn = new TCanvas("c_Rn", "c_Rn", 800, 600);
    TGraphErrors *ge_Rn = new TGraphErrors();
    TGraphErrors *ge_Po = new TGraphErrors();
    TGraph *g_Rn_lim = new TGraph();
    TGraph *g_Po_lim = new TGraph();

    for (int i = 0; i < sampleN; i++)
    {
        std::string file_path = data_dir + sample_dirs[i] + "/limit.dat";
        std::ifstream ifs(file_path);
        if (!ifs)
        {
            std::cerr << "Error: file not found" << std::endl;
            return;
        }
        std::string line;
        int lineID = 0;
        while (std::getline(ifs, line))
        {
            std::istringstream iss(line);
            std::string tmp;
            double tmp_Po, tmp_Po_err, tmp_Po_lim;
            double tmp_Rn, tmp_Rn_err, tmp_Rn_lim;
            if (lineID == 5)
            {
                iss >> tmp >> tmp_Po >> tmp >> tmp_Po_err;
                // std::cout << tmp_Po << " " << tmp_Po_err << std::endl;
            }
            if (lineID == 6)
            {
                iss >> tmp >> tmp >> tmp >> tmp_Po_lim;
                // std::cout << tmp_Po_lim << std::endl;
            }
            if (lineID == 11)
            {
                iss >> tmp >> tmp_Rn >> tmp >> tmp_Rn_err;
            }
            if (lineID == 12)
            {
                iss >> tmp >> tmp >> tmp >> tmp_Rn_lim;
            }
            ge_Po->SetPoint(i, i, tmp_Po);
            ge_Po->SetPointError(i, 0, tmp_Po_err);
            g_Po_lim->SetPoint(i, i, tmp_Po_lim);
            ge_Rn->SetPoint(i, i, tmp_Rn);
            ge_Rn->SetPointError(i, 0, tmp_Rn_err);
            g_Rn_lim->SetPoint(i, i, tmp_Rn_lim);
            lineID++;
        }
    }

    gStyle->SetNdivisions(200 + sampleN, "X");

    c_Po->cd();
    c_Po->SetGrid();
    ge_Po->SetMarkerStyle(20);
    ge_Po->SetMarkerSize(1.0);
    ge_Po->SetMarkerColor(kBlue);
    ge_Po->SetLineColor(kBlue);
    ge_Po->SetLineWidth(2);
    ge_Po->GetXaxis()->SetLimits(-1, sampleN);
    ge_Po->GetYaxis()->SetRangeUser(0, 30);
    ge_Po->GetXaxis()->SetTitle("Sample ID");
    ge_Po->GetYaxis()->SetTitle("^{214}Po rate [cpd]");
    ge_Po->Draw("AP");
    g_Po_lim->SetMarkerStyle(20);
    g_Po_lim->SetMarkerSize(1.0);
    g_Po_lim->SetMarkerColor(kRed);
    g_Po_lim->SetLineColor(kRed);
    g_Po_lim->SetLineWidth(2);
    g_Po_lim->Draw("P");

    TLegend *leg_Po = new TLegend(0.2, 0.8, 0.4, 0.9);
    leg_Po->AddEntry(ge_Po, "^{214}Po rate", "lp");
    leg_Po->AddEntry(g_Po_lim, "Upper limit (90\% C.L.)", "p");
    leg_Po->Draw();

    c_Rn->cd();
    c_Rn->SetGrid();
    ge_Rn->SetMarkerStyle(20);
    ge_Rn->SetMarkerSize(1.0);
    ge_Rn->SetMarkerColor(kBlue);
    ge_Rn->SetLineColor(kBlue);
    ge_Rn->SetLineWidth(2);
    ge_Rn->GetXaxis()->SetLimits(-1, sampleN);
    ge_Rn->GetYaxis()->SetRangeUser(0, 10);
    ge_Rn->GetXaxis()->SetTitle("Sample ID");
    ge_Rn->GetYaxis()->SetTitle("^{222}Rn rate in 0.3b [Bq]");
    ge_Rn->Draw("AP");
    g_Rn_lim->SetMarkerStyle(20);
    g_Rn_lim->SetMarkerSize(1.0);
    g_Rn_lim->SetMarkerColor(kRed);
    g_Rn_lim->SetLineColor(kRed);
    g_Rn_lim->SetLineWidth(2);
    g_Rn_lim->Draw("P");

    TLegend *leg_Rn = new TLegend(0.2, 0.8, 0.4, 0.9);
    leg_Rn->AddEntry(ge_Rn, "^{222}Rn rate", "lp");
    leg_Rn->AddEntry(g_Rn_lim, "Upper limit (90\% C.L.)", "p");
    leg_Rn->Draw();

    c_Po->SaveAs(Form("%s/PoRate.png", outputDir.c_str()));
    c_Rn->SaveAs(Form("%s/RnRate.png", outputDir.c_str()));

    return;
}