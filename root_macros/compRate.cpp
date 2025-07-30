#include "inc/shinclude.h"
#include "TGraphErrors.h"

void compRate()
{
    SetShStyle(0);

    const std::string data_dir = "/nadb/nadb55/namai/newage/RD3/ana/";
    const std::string outputDir = "/nadb/nadb55/namai/newage/RD3/ana/plot";
    const int sampleN = 4;
    const std::string sample_dirs[sampleN] = {
        "20230810",
        "20230927",
        "20240921",
        "20250709",
    };

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

            // Extract values from specific lines
            if (lineID == 5)
            {
                iss >> tmp >> tmp_Po >> tmp >> tmp_Po_err;
            }
            if (lineID == 6)
            {
                iss >> tmp >> tmp >> tmp >> tmp_Po_lim;
            }
            if (lineID == 11)
            {
                iss >> tmp >> tmp_Rn >> tmp >> tmp_Rn_err;
            }
            if (lineID == 12)
            {
                iss >> tmp >> tmp >> tmp >> tmp_Rn_lim;
            }

            // Fill data points
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

    // ----- ^{222}Rn plot -----
    c_Rn->cd();
    c_Rn->SetGrid();
    ge_Rn->SetMarkerStyle(20);
    ge_Rn->SetMarkerSize(1.0);
    ge_Rn->SetMarkerColor(kBlue);
    ge_Rn->SetLineColor(kBlue);
    ge_Rn->SetLineWidth(2);
    ge_Rn->GetXaxis()->SetLimits(-1, sampleN);
    ge_Rn->GetYaxis()->SetRangeUser(-0.5, 0.5);
    ge_Rn->GetXaxis()->SetTitle("Sample ID");
    ge_Rn->GetYaxis()->SetTitle("^{222}Rn rate in 0.3b [Bq]");
    ge_Rn->Draw("AP");

    g_Rn_lim->SetMarkerStyle(20);
    g_Rn_lim->SetMarkerSize(1.0);
    g_Rn_lim->SetMarkerColor(kRed);
    g_Rn_lim->SetLineColor(kRed);
    g_Rn_lim->SetLineWidth(2);
    g_Rn_lim->Draw("P");

    const double hlen = 0.1; // Half length of horizontal bar
    const double arrow_len = 0.1;
    int npoints = ge_Rn->GetN();

    for (int i = 0; i < npoints; ++i)
    {
        double x = 0, y = 0;
        g_Rn_lim->GetPoint(i, x, y); // Get coordinates of limit point

        // Horizontal line centered at (x, y)
        TLine *hline = new TLine(x - hlen, y, x + hlen, y);
        hline->SetLineColor(kRed);
        hline->SetLineWidth(2);
        hline->Draw();

        // Downward arrow from (x, y)
        TArrow *arrow = new TArrow(x, y, x, y - arrow_len, 0.02, "|>");
        arrow->SetLineColor(kRed);
        arrow->SetLineWidth(2);
        arrow->Draw();
    }

    // Draw dashed line at y = 0
    double xmin = ge_Rn->GetXaxis()->GetXmin();
    double xmax = ge_Rn->GetXaxis()->GetXmax();

    TLine *zeroLine = new TLine(xmin, 0, xmax, 0);
    zeroLine->SetLineStyle(2); // Dashed
    zeroLine->SetLineWidth(2);
    zeroLine->SetLineColor(kGray + 2);
    zeroLine->Draw();

    // Draw hatched area for y < 0
    double ymin = ge_Rn->GetHistogram()->GetMinimum();
    double ymax = ge_Rn->GetHistogram()->GetMaximum();

    if (ymin > 0)
        ymin = 0; // Ensure hatch covers below y=0

    // Draw rectangle with hatch style
    TBox *hatchBox = new TBox(xmin, ymin, xmax, 0);
    hatchBox->SetFillStyle(3004); // Hatched pattern
    hatchBox->SetFillColor(kGray + 1);
    hatchBox->SetLineColor(0); // No border
    hatchBox->Draw("same");

    // Add legend
    TLegend *leg_Rn = new TLegend(0.2, 0.75, 0.5, 0.9);
    leg_Rn->AddEntry(ge_Rn, "^{222}Rn rate", "lp");
    leg_Rn->AddEntry(g_Rn_lim, "Upper limit (90% C.L.)", "p");
    leg_Rn->Draw();

    // Save canvas
    c_Rn->SaveAs(Form("%s/RnRateLBGuPIC.png", outputDir.c_str()));

    return;
}
