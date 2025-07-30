#include "inc/shinclude.h"

#include <cmath>

const std::string dir_bg = "/nadb/nadb55/namai/newage/03b/radonRate/ana/20240801";
const std::string dir_sample = "/nadb/nadb55/namai/newage/03b/radonRate/ana/20240723";
const double cal = 0.400;          // cpd/(mBq/m3)
const double scale_volume = 0.027; // m3
// const double scale_sample = 1.00;
const double scale_sample = 1.;

double gaussian(double x, double mu, double sigma)
{
    return exp(-0.5 * pow((x - mu) / sigma, 2)) / (sigma * sqrt(2 * M_PI));
}

double searchLimit(const double mu = 0.0, const double sigma = 1.0)
{

    const double dx = 0.001;
    double upper_limit = 0.90;

    const double h1_bin = 100;
    const double x_min = mu - 10 * sigma;
    const double x_max = mu + 10 * sigma;

    TCanvas *c1 = new TCanvas("c1", "c1", 600, 600);
    TH1D *h1 = new TH1D("h1", "h1", h1_bin, x_min, x_max);
    TH1D *h1_dummy = new TH1D("h1_dummy", "h1_dummy", h1_bin, x_min, x_max);

    double sum = 0.0;
    double gaus = 0.0;
    for (double x = x_min; x < x_max; x += dx)
    {
        gaus = gaussian(x, mu, sigma);
        h1_dummy->Fill(x, gaus);
        if (x > 0)
        {
            h1->Fill(x, gaus);
            sum += gaus * dx;
        }
    }

    std::cout << sum << std::endl;
    // std::cout << sum << std::endl;

    h1->Scale(dx);
    h1_dummy->Scale(dx);
    h1->GetXaxis()->SetLimits(x_min, x_max);
    h1->GetYaxis()->SetRangeUser(0, h1_dummy->GetMaximum() * 1.2);
    h1->SetLineColor(0);
    h1->SetFillColor(kBlue);
    h1->SetFillStyle(3003);
    h1->GetXaxis()->SetTitle("Events");
    h1->GetYaxis()->SetTitle("Normalized probability density [A.U.]");
    h1->GetXaxis()->SetLabelSize(0.03);
    h1->GetYaxis()->SetLabelSize(0.03);
    h1->Draw("hist");
    h1_dummy->SetLineColor(kBlue);
    h1_dummy->SetLineWidth(2);
    h1_dummy->Draw("hist same");
    c1->SetGrid();

    double sum_upper = 0.0;
    for (double x = x_min; x < x_max; x += dx)
    {
        if (x > 0)
            sum_upper += gaussian(x, mu, sigma) * dx;
        if (sum_upper > upper_limit * sum)
        {
            TLine *upper_line = new TLine(x, 0, x, h1->GetBinContent(h1->FindBin(x)));
            upper_line->SetLineColor(kRed);
            upper_line->SetLineWidth(2);
            upper_line->SetLineStyle(2);
            upper_line->Draw();
            TLegend *leg = new TLegend(0.6, 0.8, 0.9, 0.9);
            leg->AddEntry(h1, "ROI", "f");
            leg->AddEntry(upper_line, "90\% C.L. upper limit", "l");
            leg->Draw();
            const double tx = x_min + (x_max - x_min) * 0.1;
            const double ty = h1->GetMaximum() * 0.8;
            TText *t_mu = new TText(tx, ty, Form("mean = %.2f", mu));
            TText *t_sigma = new TText(tx, ty - h1->GetMaximum() * 0.05, Form("Sigma = %.2f", sigma));
            TText *t_upper = new TText(tx, ty - h1->GetMaximum() * 0.10, Form("Upper limit = %.2f", x));
            t_mu->SetTextSize(0.03);
            t_sigma->SetTextSize(0.03);
            t_upper->SetTextSize(0.03);
            t_mu->Draw();
            t_sigma->Draw();
            t_upper->Draw();
            // TLine *upper_90CL = new TLine(mu + sigma * 1.65, 0, mu + sigma * 1.65, h1->GetBinContent(h1->FindBin(mu + sigma * 1.65)));
            // TLine *lower_90CL = new TLine(mu - sigma * 1.65, 0, mu - sigma * 1.65, h1->GetBinContent(h1->FindBin(mu - sigma * 1.65)));
            // upper_90CL->SetLineColor(kBlack);
            // lower_90CL->SetLineColor(kBlack);
            // upper_90CL->SetLineWidth(2);
            // lower_90CL->SetLineWidth(2);
            // upper_90CL->SetLineStyle(2);
            // lower_90CL->SetLineStyle(2);
            // upper_90CL->Draw();
            // lower_90CL->Draw();
            c1->SaveAs(Form("%s/upperLimit.png", dir_sample.c_str()));
            return x;
        }
    }

    return 0.0;
}

int calcLimit()
{
    SetShStyle();

    const std::string fitrespath_bg = dir_bg + "/fitres.dat";
    const std::string fitrespath_sample = dir_sample + "/fitres.dat";

    std::ifstream ifs_bg(fitrespath_bg);
    std::ifstream ifs_sample(fitrespath_sample);
    if (!ifs_bg)
    {
        std::cerr << "Error: cannot open " << fitrespath_bg << std::endl;
        return 1;
    }
    if (!ifs_sample)
    {
        std::cerr << "Error: cannot open " << fitrespath_sample << std::endl;
        return 1;
    }
    std::string line;
    double bg, bg_err, sample, sample_err, tmp;
    while (std::getline(ifs_bg, line))
    {
        if (line[0] == '#')
            continue;
        std::istringstream iss(line);
        iss >> bg >> bg_err >> tmp >> tmp >> tmp >> tmp >> tmp;
        std::cout << "Background 214Po : " << bg << " +- " << bg_err << " cpd" << std::endl;
    }
    while (std::getline(ifs_sample, line))
    {
        if (line[0] == '#')
            continue;
        std::istringstream iss(line);
        iss >> sample >> sample_err >> tmp >> tmp >> tmp >> tmp >> tmp;
        std::cout << "Sample 214Po     : " << sample << " +- " << sample_err << " cpd" << std::endl;
    }
    double rate = sample - bg;
    double err = sqrt(sample_err * sample_err + bg_err * bg_err);

    double x = searchLimit(rate, err);

    if (x == 0)
    {
        std::cerr << "Error: cannot find limit" << std::endl;
        return 1;
    }
    std::cout << "--- 214 Po rate ---" << std::endl;
    std::cout << "difference: " << rate << " +- " << err << " cpd" << std::endl;
    std::cout << "upper limit (90\% C.L.): < " << x << " cpd" << std::endl;
    std::cout << "--- Radon emanation ---" << std::endl;
    std::cout << "difference: " << rate / cal << " +- " << err / cal << " mBq/m3" << std::endl;
    std::cout << "upper limit (90\% C.L.): < " << x / cal << " mBq/m3" << std::endl;
    std::cout << "--- In detector ---" << std::endl;
    std::cout << "difference: " << rate / cal * scale_volume * scale_sample << " +- " << err / cal * scale_volume * scale_sample << " (mBq/m3)/" << scale_volume << "m3" << std::endl;
    std::cout << "Upper limit (90\% C.L.): < " << x / cal * scale_volume * scale_sample << " (mBq/m3)/" << scale_volume << "m3" << std::endl;

    std::ofstream ofs(Form("%s/limit.dat", dir_sample.c_str()));
    ofs << "# calibration factor: " << cal << " cpd/(mBq/m3)" << std::endl;
    ofs << "# scale volume: " << scale_volume << " m3" << std::endl;
    ofs << "# scale sample: " << scale_sample << std::endl;
    ofs << std::endl;
    ofs << "# --- 214 Po rate ---" << std::endl;
    ofs << "difference: " << rate << " +- " << err << " cpd" << std::endl;
    ofs << "upperLimit(90\% C.L.): < " << x << " cpd" << std::endl;
    ofs << "# --- Radon emanation ---" << std::endl;
    ofs << "difference: " << rate / cal << " +- " << err / cal << " mBq/m3" << std::endl;
    ofs << "upperLimit(90\% C.L.): < " << x / cal << " mBq/m3" << std::endl;
    ofs << "# --- In detector ---" << std::endl;
    ofs << "difference: " << rate / cal * scale_volume * scale_sample << " +- " << err / cal * scale_volume * scale_sample << " (mBq/m3)/" << scale_volume << "m3" << std::endl;
    ofs << "UpperLimit(90\% C.L.): < " << x / cal * scale_volume * scale_sample << " (mBq/m3)/" << scale_volume << "m3" << std::endl;
    ofs.close();

    return 0;
}