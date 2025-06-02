#include "inc/shinclude.h"

void DrawWaveform()
{
    SetShStyle();

    const std::string rootfile_path = "/home/msgc/RD3/ana/20241030/rootfile/RD_0-688.root";
    TFile *file = new TFile(rootfile_path.c_str());
    if (!file->IsOpen())
    {
        std::cout << "rootfile not found" << std::endl;
        return;
    }
    TTree *tree = (TTree *)file->Get("tree");

    const int evtN = tree->GetEntries();
    std::cout << "Total events: " << evtN << std::endl;
    Float_t wf[1024] = {};
    tree->SetBranchAddress("wf", wf);

    TCanvas *c_wf = new TCanvas("c_wf", "c_wf", 800, 600);
    TH2D *h_wf = new TH2D("h_wf", "h_wf", 1024, 0, 1024, 100, -2.5, 2.5);

    for (int evt = 0; evt < evtN; evt++)
    {
        ShUtil::PrintProgressBar(evt, evtN);
        tree->GetEntry(evt);
        for (int clk = 0; clk < 1024; clk++)
        {
            h_wf->Fill(clk, wf[clk]);
        }
        // break;
    }

    h_wf->Draw("colz");

    c_wf->SaveAs("./waveform.png");
}