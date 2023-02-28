
void chain(int s_sub,int e_sub){

  TChain* chain = new TChain("tree","tree");

  for(int i=s_sub;i<e_sub+1;i++){
    ostringstream ss;
    //    ss << "sub" << std::setw(4) << std::setfill('0') << i <<  ".0000.root";
    ss<<"RD_"<<i<<".root";
    string s(ss.str());
    const char* file = s.c_str();
    chain->Add(file);
  }
  chain->Merge(Form("RD_%d-%d.root",s_sub,e_sub),"recreate");


}
