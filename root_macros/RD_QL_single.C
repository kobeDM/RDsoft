void RD_QL(){

  TCanvas *cvs=new TCanvas("cvs","cvs",100,50,800,750);
  cvs->Divide(2,3);

  //input files
  const int file_num=2;
  string filename[file_num];
  filename[0]="/home/msgc/RD1/data/20230213/RD_0.dat";
  filename[1]="/home/msgc/RD3/data/20230213/RD_0.dat";

  //output
  //  string outputDir="test";
  //    SetShStyle();
  //  ShUtil::ExistCreateDir( outputDir );

  // display parameters
  TLatex *lt[file_num][8];
  TLatex *lts[file_num][8];
  double Vmin=-2500/1000.;
  double Vmax=2500/1000.;
  int tmin=400;
  int tmax=600;
  int tbin=200;
  int Ybin=100;
  int spbin=100;
  double spmin=0/1000.0;
  double spmax=3000/1000.;
  int spMbin=100;
  double spMmin=0;
  double spMmax=10;

  // data parameters
  double cal_a[2],cal_b[2]; //MeV=a*mV+b
  //  cal_a[0]=1/200.;
  //cal_a[1]=1/105.;
  cal_a[0]=4.80;
  cal_a[1]=9.5;
  cal_b[0]=0;
  cal_b[1]=0;
  double bin_MeV=(spMmax-spMmin)/spMbin;
  double Eth_MeV=2.0;

  //parameters
  double ene_reso = 2.96; //% +- 3sigma
  double Po_energy[2] = {6.00235,7.68682};//MeV
  int Po214_count[2]={0,0};
  int Po218_count[2]={0,0};
  double E;

  //histgrams
  string stWF[2],stSP[2],stSPM[2],stSPPo214[2],stSPPo218[2];
  string stWFs[2],stSPs[2],stSPMs[2],stSPPo214s[2],stSPPo218s[2];
  TH1F *histSps[2],*histSps_nc[2],*histSps_MeV[2];
  TH1F *histSps_Po214[2],*histSps_Po218[2];
  TH2F *histWFs[2];
  for(int i=0;i<file_num;i++){
    stWF[i]="WF ("+filename[i]+")";
    stWFs[i]="WF"+to_string(i);
    stSP[i]="voltage spectrum "+filename[i];
    stSPs[i]="voltage spectrum"+to_string(i);
    stSPM[i]="energy spectrum"+filename[i];
    stSPMs[i]="energy spectrum"+to_string(i);
    stSPPo214[i]="Po spectrum"+filename[i];
    stSPPo214s[i]="Po spectrum"+to_string(i);
    stSPPo218[i]="Po spectrum"+filename[i];
    stSPPo218s[i]="Po spectrum"+to_string(i);
    histWFs[i]=new TH2F(stWFs[i].c_str(),stWF[i].c_str(),tbin,tmin,tmax,Ybin,Vmin,Vmax);
    histSps[i]=  new TH1F(stSPs[i].c_str(),stSP[i].c_str(),spbin,spmin,spmax);
    histSps_nc[i]=  new TH1F(stSPs[i].c_str(),stSP[i].c_str(),spbin,spmin,spmax);
    histSps_MeV[i]=  new TH1F(stSPMs[i].c_str(),stSPM[i].c_str(),spMbin,spMmin,spMmax);    
    histSps_Po214[i]=  new TH1F(stSPMs[i].c_str(),stSPM[i].c_str(),spMbin,spMmin,spMmax); 
    histSps_Po218[i]=  new TH1F(stSPMs[i].c_str(),stSPM[i].c_str(),spMbin,spMmin,spMmax);
    histSps[i]->GetXaxis()->SetTitle("V");
    histSps[i]->GetYaxis()->SetTitle("entries/bin");
    histSps_MeV[i]->GetXaxis()->SetTitle("MeV");
    histSps_MeV[i]->GetYaxis()->SetTitle("counts/MeV/days");
    histSps_nc[i]->GetXaxis()->SetTitle("V");
    histWFs[i]->GetXaxis()->SetTitle("clock(10MHz)");
    histWFs[i]->GetYaxis()->SetTitle("V");
    histSps[i]->SetLineColor(2);
    histSps_nc[i]->SetLineColor(4);
    histSps_Po214[i]->SetFillColor(7);
    histSps_Po214[i]->SetFillStyle(3000);
    histSps_Po218[i]->SetFillColor(6);
    histSps_Po218[i]->SetFillStyle(3000);
  }
  
  //for data
   int ch[file_num];
   double v_cal=0.001;
   double V_avg=0;
   double ch1 = 0;
   int idummy;
   int veto[2];
   int clock = 0;
   double ch1Min = 1023*v_cal;//, ch2Min = 1023;
   double ch1Max = -1024*v_cal;//, ch2Max = -1024;
   int evtID = 0;
   int twin=50;
   int twin_avg[2];
   twin_avg[0]=400;
   twin_avg[1]=twin_avg[0]+twin;
  int veto_neg=-100;
  int utime[2];
  int start_time[2],end_time[2];
  double live_time[2];
  string line,dummy,livestr[2],Po214str[2],Po218str[2];
  string calstr[2];

  //initialize
  for(int i=0; i< file_num; i++){
    veto[i]=0;
  }
  //read
  for(int i=0; i< file_num; i++){
    evtID = 0;
    std::ifstream ifs( filename[i] );
    cerr<<"file name "<<i<<": "<<filename[i]<<endl;
    if( ifs.is_open() == false ) return 1;
    while( !ifs.eof( ) ) {
      line = "";
      std::getline( ifs, line );
      if( line.length( ) <= 0 || strncmp( line.c_str( ), "#Ev.", 4 ) == 0 ) {
	//event head
	//	 cerr<<line<<endl;
	std::stringstream ss( line );
	ss >> dummy >> dummy>>utime[i];
	if(evtID==0)start_time[i]=utime[i];
	else end_time[i]=utime[i];
      }
      else if( line.length( ) <= 0 || strncmp( line.c_str( ), "#", 1 ) == 0 ) {
	//event end
	if( evtID != 0 ) {
	  if(veto[i]==0){
	    //	    histSps[i]->Fill( ch1Max - ch1Min );
	    histSps[i]->Fill( ch1Max - V_avg );
	    //	    E=cal_a[i]*(ch1Max - ch1Min)+cal_b[i];
	    E=cal_a[i]*(ch1Max - V_avg)+cal_b[i];
	    if(E>Eth_MeV){
	      histSps_MeV[i]->Fill(E);
	    }
	    //	    if(Po_energy[0]-Po_energy[0]*3*ene_reso/100<E&&E<Po_energy[0]+Po_energy[0]*3*ene_reso/100){
	    if(Po_energy[0]-Po_energy[0]*3*ene_reso/100<E&&E<Po_energy[0]+Po_energy[0]*0*ene_reso/100){
	      //Po218
	      Po218_count[i]++;
	      histSps_Po218[i]->Fill(E);
	    }
	    //if(Po_energy[1]-Po_energy[1]*3*ene_reso/100<E&&E<Po_energy[1]+Po_energy[1]*3*ene_reso/100){
	    if(Po_energy[1]-Po_energy[1]*3*ene_reso/100<E&&E<Po_energy[1]+Po_energy[1]*0*ene_reso/100){
	      //Po214
	      histSps_Po214[i]->Fill(E);
	      Po214_count[i]++;
	    }

	  }
	  //	  histSps_nc[i]->Fill( ch1Max - ch1Min );
	  histSps_nc[i]->Fill( ch1Max - V_avg );
	}
	++evtID;
	clock = 0;
	ch1Min = 1023;
	ch1Max = -1024;
	V_avg=0;
	veto[i]=0;
	if(evtID%100==0)std::cerr<<evtID<<"\r";
	continue;
      }
      std::stringstream ss( line );
      ss >> ch1 >> idummy;       
      ch1*=v_cal;
      if(clock>twin_avg[0]&&clock<twin_avg[1])V_avg+=ch1/twin;
      if     ( ch1 < ch1Min ) ch1Min = ch1;
      else if( ch1 > ch1Max ) ch1Max = ch1;
      if(ch1<veto_neg){
	veto[i]++;
      }
      histWFs[i]->Fill(clock, ch1);
      ++clock;
    }
    ifs.close();
    //file summary
    //      cerr<<"file name "<<i<<": "<<filename[i]<<endl;
    cerr<<"\n"<<evtID<<" evetns"<<endl;
    cerr<<" started at "<<start_time[i]<<endl;
    cerr<<" ended at "<<end_time[i]<<endl;
    if(end_time[i]>start_time[i])live_time[i]=(end_time[i]-start_time[i])/3600./24;
    cerr<<"realtime: "<<live_time[i]<<" days"<<endl;
    //normalize
    histSps_MeV[i]->Sumw2(); 
    histSps_Po214[i]->Sumw2(); 
    histSps_Po218[i]->Sumw2(); 
    histSps_MeV[i]->Scale(1/live_time[i]/bin_MeV);
    histSps_Po214[i]->Scale(1/live_time[i]/bin_MeV);
    histSps_Po218[i]->Scale(1/live_time[i]/bin_MeV);

  }



   //legends
   for(int i=0; i< file_num; i++){
     //     string livestr="livetime: "+std::to_string(live_time[1])+" days";
     //     livestr[i]="livetime: "+std::to_string(live_time[i])+" days";
     std::stringstream livess,calass,calbss;
     livess<< std::setprecision(2) << live_time[i]; // 
     calass<< std::setprecision(3) << cal_a[i]; // 
     calbss<< std::setprecision(2) << cal_b[i]; // 
     livestr[i]="livetime: "+livess.str()+" days";
     calstr[i]=calass.str()+"*mV+"+calbss.str();
     Po214str[i]="Po214 "+std::to_string(Po214_count[i])+" events";
     Po218str[i]="Po218 "+std::to_string(Po218_count[i])+" events";
     lt[i][0]=new TLatex(tmin+0.1*(tmax-tmin),-0.6*Vmax,filename[i].c_str());
     lts[i][0]=new TLatex(spmin+0.1*(spmax-spmin),(histSps[i]->GetMaximum())*0.8,filename[i].c_str());
     lts[i][1]=new TLatex(spmin+0.1*(spmax-spmin),(histSps[i]->GetMaximum())*0.7,livestr[i].c_str());
     lts[i][4]=new TLatex(spMmin+0.1*(spMmax-spMmin),(histSps_MeV[i]->GetMaximum())*0.9,livestr[i].c_str());
     lts[i][2]=new TLatex(spMmin+0.1*(spMmax-spMmin),(histSps_MeV[i]->GetMaximum())*0.8,Po214str[i].c_str());
     lts[i][3]=new TLatex(spMmin+0.1*(spMmax-spMmin),(histSps_MeV[i]->GetMaximum())*0.7,Po218str[i].c_str());
     lts[i][5]=new TLatex(spMmin+0.1*(spMmax-spMmin),(histSps_MeV[i]->GetMaximum())*0.6,calstr[i].c_str());
   }

   
    
    //histgram draw
   for(int i=0; i< file_num; i++){
     // for(int i=0; i< 1; i++){
      cvs->cd(1+i);
      histWFs[i]->Draw("colz");
      lt[i][0]->Draw();
      cvs->cd(3+i);
      //     histSpCh1_all->Draw();
      //histSpCh1->Draw("same");
      histSps[i]->Draw();
      histSps_nc[i]->Draw("same");
      histSps[i]->Draw("same");
      lts[i][0]->Draw();
      lts[i][1]->Draw();
      cvs->cd(5+i);
      histSps_MeV[i]->Draw();
      histSps_Po214[i]->Draw("same,hist");
      histSps_Po218[i]->Draw("same,hist");
      histSps_MeV[i]->Draw("same,hist");
      lts[i][4]->Draw();
      lts[i][2]->Draw();
      lts[i][3]->Draw();
      lts[i][5]->Draw();
    }

    return 0;
}
