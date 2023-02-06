void RD_QL(){

  TCanvas *cvs=new TCanvas("cvs","cvs",100,50,800,600);
  cvs->Divide(2,2);

  //input files
  const int file_num=2;
  string filename[file_num];
  filename[0]="/home/msgc/RD1/data/BG/data/20230207/sub_0.dat";
  filename[1]="/home/msgc/RD3/data/BG/data/20230207/sub_0.dat";

  //out put
  //  string outputDir="test";
  //    SetShStyle();
  //  ShUtil::ExistCreateDir( outputDir );

  // display parameters
  TLatex *lt[file_num][8];
  TLatex *lts[file_num][8];
  double Vmin=-2500;
  double Vmax=2500;
  int tmin=400;
  int tmax=600;
  int tbin=200;
  int Ybin=100;
  int spbin=100;
  int spmin=0;
  int spmax=3000;

  //histgrams
  string stWF[2],stSP[2];
  TH1F *histSps[2],*histSps_nc[2];
  TH2F *histWFs[2];
  for(int i=0;i<file_num;i++){
    stWF[i]="WF ("+filename[i]+")";
    stSP[i]="energy spectrum "+filename[i];
    histWFs[i]=new TH2F("histWF_file0",stWF[i].c_str(),tbin,tmin,tmax,Ybin,Vmin,Vmax);
    histSps[i]=  new TH1F("histSPCh1",stSP[i].c_str(),spbin,spmin,spmax);
    histSps_nc[i]=  new TH1F("histSPCh1",stSP[i].c_str(),spbin,spmin,spmax);
    histSps[i]->SetLineColor(2);
    histSps_nc[i]->SetLineColor(4);
  }
  
    TH2F *histWFCh1=new TH2F("histWF_file0",stWF[0].c_str(),tbin,tmin,tmax,Ybin,Vmin,Vmax);
   TH2F *histWFCh2=new TH2F("histWF_file1",stWF[1].c_str(),tbin,tmin,tmax,Ybin,Vmin,Vmax);

  //for data
   int ch[file_num];
   int ch1 = 0, ch2 = 0;
  int veto[2];
  int clock = 0;
  int ch1Min = 1023, ch2Min = 1023;
  int ch1Max = -1024, ch2Max = -1024;
  int evtID = 0;
  
  int veto_neg=-100;
  int utime[2];
  int start_time[2],end_time[2];
  double live_time[2];
  string line,dummy,livestr[2];

  std::ifstream ifs( filename[0] );
  if( ifs.is_open() == false ) return;
  while( !ifs.eof( ) ) {
    line = "";
    std::getline( ifs, line );
    if( line.length( ) <= 0 || strncmp( line.c_str( ), "#Ev.", 4 ) == 0 ) {
      //cerr<<line<<endl;
      std::stringstream ss( line );
      ss >> dummy >> dummy>>utime[0];
	  if(evtID==0)start_time[0]=utime[0];
	else end_time[0]=utime[0];
	}
        else if( line.length( ) <= 0 || strncmp( line.c_str( ), "#", 1 ) == 0 ) {
            if( evtID != 0 ) {
	      //if(veto[0]==0){
		//		histSpCh1->Fill( ch1Max - ch1Min );
		//		histSps[0]->Fill( ch1Max - ch1Min );
	      //}
	      //       histSpCh1_all->Fill( ch1Max - ch1Min );
	      //	       histSps_nc[0]->Fill( ch1Max - ch1Min );
			
            }            
            ++evtID;
            clock = 0;
            ch1Min = 1023;
            ch1Max = -1024;
	    veto[0]=0;
            continue;
        }
        std::stringstream ss( line );
        ss >> ch1 >> ch2;

	//        if     ( ch1 < ch1Min ) ch1Min = ch1;
        //else if( ch1 > ch1Max ) ch1Max = ch1;
	//	if(ch1<veto_neg){
	//  veto[0]++;
	  //	  cerr<<"ch1 veto "<<ch1<<" "<<veto[0]<<endl;

	//}
	//        histWFCh1->Fill(clock, ch1);
	//        histWFs[0]->Fill(clock, ch1);
        //++clock;
  }
  ifs.close();
    //}
  

    double texty=200;
    //initialize
    for(int i=0; i< file_num; i++){
      veto[i]=0;
   }
    //read
    for(int i=0; i< file_num; i++){
      evtID = 0;
      std::ifstream ifs( filename[i] );
      if( ifs.is_open() == false ) return 1;
      while( !ifs.eof( ) ) {
       line = "";
       std::getline( ifs, line );
       if( line.length( ) <= 0 || strncmp( line.c_str( ), "#Ev.", 4 ) == 0 ) {
	 cerr<<line<<endl;
	 std::stringstream ss( line );
	 ss >> dummy >> dummy>>utime[i];
	 if(evtID==0)start_time[i]=utime[i];
	 else end_time[i]=utime[i];
       }
       else if( line.length( ) <= 0 || strncmp( line.c_str( ), "#", 1 ) == 0 ) {
	  if( evtID != 0 ) {
	    if(veto[i]==0){
	      histSps[i]->Fill( ch1Max - ch1Min );
	    }
	    histSps_nc[i]->Fill( ch1Max - ch1Min );
	    
	  }
	  ++evtID;
	  clock = 0;
	  ch1Min = 1023;
	  ch1Max = -1024;
	  veto[i]=0;
	  continue;
       }
       std::stringstream ss( line );
       ss >> ch1 >> ch2;       
       if     ( ch1 < ch1Min ) ch1Min = ch1;
       else if( ch1 > ch1Max ) ch1Max = ch1;
       if(ch1<veto_neg){
	 veto[i]++;
       }
       histWFs[i]->Fill(clock, ch1);
        ++clock;
     }
      ifs.close();
   }


   //legends
   for(int i=0; i< file_num; i++){
     //     string livestr="livetime: "+std::to_string(live_time[1])+" days";
     livestr[i]="livetime: "+std::to_string(live_time[i])+" days";
     lt[i][0]=new TLatex(tmin+0.1*(tmax-tmin),Vmax*0.8,filename[i].c_str());
     lts[i][0]=new TLatex(spmin+0.5*(spmax-spmin),texty*0.8,filename[i].c_str());
     lts[i][1]=new TLatex(spmin+0.5*(spmax-spmin),texty*0.7,livestr[i].c_str());
   }

   

    
    //histgram draw
   for(int i=0; i< file_num; i++){
     // for(int i=0; i< 1; i++){
      cerr<<"file name "<<i<<": "<<filename[0]<<endl;
      cerr<<"started at "<<start_time[0]<<endl;
      cerr<<"ended at "<<end_time[0]<<endl;
      if(end_time[0]>start_time[0])live_time[0]=(end_time[0]-start_time[0])/3600./24;
      cerr<<"livetime (days)"<<live_time[0]<<endl;
      cvs->cd(1+i);
      histWFs[i]->Draw("colz");
      lt[i][0]->Draw();
      cvs->cd(3+i);
      //     histSpCh1_all->Draw();
      //histSpCh1->Draw("same");
      histSps_nc[i]->Draw();
      histSps[i]->Draw("same");
      lts[i][0]->Draw();
      lts[i][1]->Draw();
    }

    return 0;
    /**   clock = 0;
    evtID = 0;

    std::ifstream ifs2( filename[1] );
    if( ifs.is_open() == false ) return;
    while( !ifs.eof( ) ) {
        string line = "";
        std::getline( ifs, line );
        if( line.length( ) <= 0 || strncmp( line.c_str( ), "#Ev.", 4 ) == 0 ) {
	  //cerr<<line<<endl;
	  std::stringstream ss( line );
	  ss >> dummy >> dummy>>utime[1];
	  if(evtID==0)start_time[1]=utime[1];
	else end_time[1]=utime[1];
	}
        else if( line.length( ) <= 0 || strncmp( line.c_str( ), "#", 1 ) == 0 ) {
            if( evtID != 0 ) {
	      //	      if(veto[0]==0)//    histSpCh2->Fill( ch1Max - ch1Min );
	      //histSpCh2_all->Fill( ch1Max - ch1Min );
            }
            
            ++evtID;
            clock = 0;
            ch1Min = 1023;
            ch1Max = -1024;
	    veto[0]=0;
            continue;
        }
        std::stringstream ss( line );
        ss >> ch1 >> ch2;

        if     ( ch1 < ch1Min ) ch1Min = ch1;
        else if( ch1 > ch1Max ) ch1Max = ch1;
	if(ch1<veto_neg){
	  veto[0]++;
	  //	  cerr<<"ch1 veto "<<ch1<<" "<<veto[0]<<endl;

	}
	//        histWFCh2->Fill(clock, ch1);
        ++clock;
    }
    cerr<<"file name 1"<<filename[1]<<endl;
    cerr<<"started at "<<start_time[1]<<endl;
    cerr<<"ended at "<<end_time[1]<<endl;
    if(end_time[1]>start_time[1])live_time[1]=(end_time[1]-start_time[1])/3600./24;
    cerr<<"livetime (days)"<<live_time[1]<<endl;





		      //std::sto_string(live_time).c_str());


    //    TH1F *h1 = new TH1F( "h1", "historgram", 100, -10., 10. );
    // h1->FillRandom("gaus");
    // h1->Draw();
    
    //  cvs->SaveAs( Form( "%s/%s_wfch1.png", outputDir.c_str( ), outname.c_str() ) );
    **/
  return 0;
}
