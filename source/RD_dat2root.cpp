//------------------------------------------------------------------
// Read tree adalm_daq output data
// Update: 9. Feb 2023
// Author: K. Miuchi
//------------------------------------------------------------------

//// STL
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
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TClassTable.h"
#include "TApplication.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TString.h"
//USER


int main(int argc, char** argv){
  int opt;
  int verbose=0;  
  std::cerr<<"## RD_dat2root.cpp ##"<<std::endl;
  //command line parameters
  if(argc < 3){
    std::cout << "Usage :";
    std::cout << "RD_dat2root [-v] datafile configfile" << std::endl;
    return -1;
  }

  while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
    case 'v':
      printf("- Verbose mode\n");
      verbose=1;
      break;
    }
  }
  std::string file = argv[1+optind-1];
  std::string conffilename=argv[2+optind-1];

  //filenames
  std::string filename;
  std::string outfilename = "out.root";
  std::cerr<<"Input file: "<<file<<std::endl;
  std::cerr<<"Output file: "<<outfilename<<std::endl;   
  TFile* out_file = TFile::Open(outfilename.c_str(), "RECREATE");


  //json config file
  std::cerr<<"JSON config file: "<<conffilename<<std::endl;
  std::string::size_type index_conf = conffilename.find(".json");
  if( index_conf == std::string::npos ) { 
    std::cout << "Error: Config file cannot open !!!" << std::endl;
    return 1;
  }
  boost::property_tree::ptree pt;
  read_json(conffilename,pt);
  double dynamic_range = pt.get<double>("DAQ.DYNAMIC RANGE"); // 2.5V
  double sampling_helz = pt.get<double>("DAQ.SAMPLING RATE"); // 10MHz
  int sampling_length = pt.get<int>("DAQ.SAMPLING NUMBER"); // 1024

  //data parameters
  std::string buf;
  std::string buf_ts;
  ULong64_t eventid;
  ULong64_t eventid_start;
  ULong64_t timestamp_start;
  ULong64_t timestamp;
  UInt_t timestamp_usec;
  ULong64_t timestamp_end;
  UInt_t timestamp_usec_end;
  Short_t buf1;
  int ev_thisfile,pos,live,ibin;

  // root trees
  Float_t wf[sampling_length];
  TTree* tree = new TTree("tree", "tree");
  tree->Branch("eventid", &eventid, "eventid/l");
  tree->Branch("timestamp", &timestamp, "timestamp/l");
  tree->Branch("timestamp_usec", &timestamp_usec, "timestamp_usec/i");
  tree->Branch("timestamp_end", &timestamp_end, "timestamp_end/l");
  tree->Branch("timestamp_usec_end", &timestamp_usec_end, "timestamp_usec_end/i");
  tree->Branch("wf", wf, "wf[1024]/F");

  if(verbose){
    std::cerr<<"    dynamic ragne: "<<dynamic_range <<" V"<<std::endl;
    std::cerr<<"    sampling rate: "<< sampling_helz<<" Hz"<<std::endl;
    std::cerr<<"    sampling length: "<< sampling_length<<""<<std::endl;
  }
 
  std::ifstream ifs(file);
  if(!ifs.is_open()) return -1;
  ev_thisfile=0;
  //loop start
  while(ifs >> buf >> eventid >> buf_ts){ 
    pos = buf_ts.find(".");
    timestamp = stoi(buf_ts.substr(0,pos));   
    if(ev_thisfile==0){
      timestamp_start = timestamp;
      eventid_start=eventid;
    }
    ifs >> buf >> buf;  
    for(ibin=0;ibin<sampling_length;ibin++){
      ifs >> buf1 >>buf;
      wf[ibin] = (double)buf1*0.001; //mV -> V
    }
    ifs >> buf>> buf_ts; 
    pos = buf_ts.find(".");
    timestamp_end = stoi(buf_ts.substr(0,pos));   
    std::cerr<<"Event: "<< eventid <<"\r"<<std::flush;
    ev_thisfile++;
    tree->Fill();
  }
  live=timestamp_end-timestamp_start;
  std::cerr<<eventid-eventid_start<<" events in "<<live<<" seconds."<<std::endl;
  tree->Write();
  out_file->Close();
  return 0;
}
