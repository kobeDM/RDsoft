#include <iostream>
#include <sstream>
#include <string>

#include "TChain.h"

void chain(int s_sub, int e_sub)
{
  std::cout << "file range: " << s_sub << " - " << e_sub << std::endl;

  TChain *chain = new TChain("tree", "tree");

  for (int i = s_sub; i < e_sub + 1; i++)
  {
    ostringstream ss;
    //    ss << "sub" << std::setw(4) << std::setfill('0') << i <<  ".0000.root";
    ss << "RD_" << i << ".root";
    string s(ss.str());
    const char *file = s.c_str();
    chain->Add(file);
  }
  chain->Merge(Form("RD_%d-%d.root", s_sub, e_sub), "recreate");
}

void chain(std::string chainList, std::string outFileName)
{
  std::cout << "chainList: " << chainList << std::endl;

  TChain *chain = new TChain("tree", "tree");

  std::string line;
  std::ifstream infile(chainList.c_str());
  while (std::getline(infile, line))
  {
    chain->Add(line.c_str());
  }
  // chain->Merge("RD_chain.root", "recreate");
  chain->Merge(outFileName.c_str(), "recreate");
}