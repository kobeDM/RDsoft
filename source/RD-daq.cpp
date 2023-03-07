#define _USE_MATH_DEFINES
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/analog/m2kanalogout.hpp>

using namespace std;
using namespace libm2k;
using namespace libm2k::analog;
using namespace libm2k::context;

// uncomment the following definition to test triggering
#define TRIGGERING

int main(int argc, char* argv[])
{
	
	//timestamp
	struct timeval tv;

	if(argc!=11){
		printf("./RD-daq outfileheader sub_entries sampling_rate(Hz) sampling_number dynamic_range_(0(+/-25V) or 1(+/-2.5V)) ch1_Vth(V)] ch2_Vth(V)] trigger_source_(0(ch1) or 1(ch2) or 2(or)) Trig_Edge_(RISE=0 or FALL=1) comment\n");
		printf("ex) RD-daq out 1000 1000000 1024 2 0.1 0.1 2 0 comment\n");
		return -1;
	}
	
	//----------------------------
	//  configure
	const char* fheader = argv[1];
	int sub_entries = atoi(argv[2]);	
	double sampling_rate = double(atof(argv[3]));	
	double sampling_num = atoi(argv[4]);	
	//double dynamic_range = double(atof(argv[5])); // V	
	int dynamic_range = atoi(argv[5]); // V	
	double Vth1 = double(atof(argv[6])); // V (TRIGGER THRESHOLD)	
	double Vth2 = double(atof(argv[7])); // V	(TRIGGER THRESHOLD)
	int trigger_source = atoi(argv[8]);
	int trig_edge = atoi(argv[9]);
	const char* comment = argv[10];
	//----------------------------
	//  Open ADALM2000
	M2k *ctx = m2kOpen();
	if (!ctx) {
		std::cout << "Connection Error: No ADALM2000 device available/connected to your PC." << std::endl;
		return 1;
	}

	ctx->calibrateADC();
	ctx->calibrateDAC();

	M2kAnalogIn *ain = ctx->getAnalogIn();
	ain->setVerticalOffset(ANALOG_IN_CHANNEL_1,0);
	ain->setVerticalOffset(ANALOG_IN_CHANNEL_2,0);
	M2kAnalogOut *aout = ctx->getAnalogOut();

#ifdef TRIGGERING
	M2kHardwareTrigger *trig = ain->getTrigger();
#endif

	// Setup analog in
	ain->enableChannel(0, true);
	ain->enableChannel(1, true);
	ain->setSampleRate(sampling_rate);
	ain->setRange((ANALOG_IN_CHANNEL)0,(M2K_RANGE)dynamic_range);
	ain->setRange((ANALOG_IN_CHANNEL)1,(M2K_RANGE)dynamic_range);
	//ain->setRange((ANALOG_IN_CHANNEL)0,-dynamic_range,dynamic_range);
	//ain->setRange((ANALOG_IN_CHANNEL)1,-dynamic_range,dynamic_range);

#ifdef TRIGGERING
	// setup analog trigger
	trig->setAnalogSource((M2K_TRIGGER_SOURCE_ANALOG)trigger_source);
	//trig->setAnalogSource(CHANNEL_1_OR_CHANNEL_2);
	trig->setAnalogDelay(-int(sampling_num/2));
	// set CH1 trigger
	trig->setAnalogCondition(0,(M2K_TRIGGER_CONDITION_ANALOG)trig_edge);
	trig->setAnalogLevel(0, Vth1);
	trig->setAnalogMode(0,ANALOG);
	// set CH2 trigger
	trig->setAnalogCondition(1,(M2K_TRIGGER_CONDITION_ANALOG)trig_edge);
	trig->setAnalogLevel(1, Vth2);
	trig->setAnalogMode(1,ANALOG);
#endif

/*/ for test
	// setup analog output
	aout->setSampleRate(0,750000);
	aout->setSampleRate(1,750000);
	aout->enableChannel(0, true);
	aout->enableChannel(1, true);
	// create output buffers
	vector<double> sinv;
	vector<double> saw;
	for(int i=0;i<1024;i++)
	{
		double rad = 2*M_PI*(i/1024.0);
		double val = 1.*sin(rad);
		sinv.push_back(val);
		saw.push_back((0*i)/1024.0);
	}
	aout->setCyclic(true);
	aout->push({sinv,saw});
	//aout->push({saw,sinv});
*/


	string strfheader = fheader;
	//string fname = strfheader + ".dat";
	string fname = strfheader;
	string fconfname = strfheader + ".cnf";
	printf("-------- SETTING CONFIGURE --------\n");
	printf("RUN CONFIG FILE NAME   : %s\n",        fconfname.c_str());
	printf("OUTPUT FILE NAME       : %s\n",        fname.c_str());
	printf("SAMPLING RATE          : %.1lf (Hz)\n",sampling_rate);
	printf("SAMPLING NUMBER        : %.0lf\n",     sampling_num);
	printf("DYNAMIC RANGE          : %d (0=+/-25V,1=+/-2.5V)\n", dynamic_range);
	printf("TRIGGER THRESHOLD CH1  : %.3lf (V)\n", Vth1);
	printf("TRIGGER THRESHOLD CH2  : %.3lf (V)\n", Vth2);
	printf("TRIGGER SOURCE         : %d    [ch1=0,ch2=1,or-trig=2,and-trig=3]\n",        trigger_source);
	printf("TRIGGER EDGE (R=0,F=1) : %d\n",        trig_edge);
	printf("-----------------------------------\n");
	FILE* fconf;
	fconf = fopen(fconfname.c_str(),"w");
	fprintf(fconf,"---------- RUN CONFIGURE ----------\n");
	fprintf(fconf,"RUN CONFIG FILE NAME   : %s\n",        fconfname.c_str());
	fprintf(fconf,"OUTPUT FILE NAME       : %s\n",        fname.c_str());
	fprintf(fconf,"SAMPLING RATE          : %.1lf (Hz)\n",sampling_rate);
	fprintf(fconf,"SAMPLING NUMBER        : %.0lf\n",     sampling_num);
	fprintf(fconf,"DYNAMIC RANGE          : %d (0=+/-25V,1=+/-2.5V)\n", dynamic_range);
	fprintf(fconf,"TRIGGER THRESHOLD CH1  : %.3lf (V)\n", Vth1);
	fprintf(fconf,"TRIGGER THRESHOLD CH2  : %.3lf (V)\n", Vth2);
	fprintf(fconf,"TRIGGER SOURCE         : %d    [ch1=0,ch2=1,or-trig=2,and-trig=3]\n",trigger_source);
	fprintf(fconf,"TRIGGER EDGE (R=0,F=1) : %d\n",        trig_edge);
	fprintf(fconf,"COMMENT                : %s\n",        comment);
	fprintf(fconf,"-----------------------------------\n");

	FILE* ofile;

	int trigger_num = 0;
	int subrun_num = 0;
	gettimeofday(&tv,NULL);
	fprintf(fconf,"RUN START : timestamp %ld.%06lu\n",tv.tv_sec,tv.tv_usec);
	fflush(fconf);
	printf(" RUN START : timestamp %ld.%06lu\n",tv.tv_sec,tv.tv_usec);
	while(true){
		//file close/open
		if(trigger_num%sub_entries==0){
			if(trigger_num != 0){
				fclose(ofile);
				gettimeofday(&tv,NULL);
				fprintf(fconf,"SUBRUN%d END   : timestamp %ld.%06lu\n",subrun_num,tv.tv_sec,tv.tv_usec);
				fflush(fconf);
				printf("\nSUBRUN%d END   : timestamp %ld.%06lu\n",subrun_num,tv.tv_sec,tv.tv_usec);
				subrun_num++;
			}
			std::stringstream ss;
			ss << fname << "_" << subrun_num << ".dat";
			ofile = fopen(ss.str().c_str(),"w");
			gettimeofday(&tv,NULL);
			fprintf(fconf,"SUBRUN%d START : timestamp %ld.%06lu\n",subrun_num,tv.tv_sec,tv.tv_usec);
			fflush(fconf);
			printf("\nSUBRUN%d START : timestamp %ld.%06lu\n",subrun_num,tv.tv_sec,tv.tv_usec);
			if(ofile == NULL){
				printf("cannot open file .\n");
				return -1;
			}
		}

		ain->startAcquisition(sampling_num);
		auto data = ain->getSamplesRaw(sampling_num);
		printf("\rTRIGGER NUM : %d",trigger_num);
		fflush(stdout);
		gettimeofday(&tv,NULL);
		fprintf(ofile,"#Ev. %d %ld.%06lu\n",trigger_num,tv.tv_sec,tv.tv_usec);
		fprintf(ofile,"#CH1 CH2\n");
		for(int samp=0;samp<sampling_num;samp++){
			fprintf(ofile ,"%.0lf %.0lf\n",data[0][samp],data[1][samp]);
		}
		/*
		for(double val : data[0])
		{
			fprintf(ofile,"%.0lf\n",val);
		}
		fprintf(ofile,"CH2\n");
		for(double val : data[1])
		{
			fprintf(ofile,"%.0lf\n",val);
		}
		*/
		gettimeofday(&tv,NULL);
		fprintf(ofile,"#Write %ld.%06lu\n",tv.tv_sec,tv.tv_usec);
		fflush(ofile);
		ain->stopAcquisition();
		trigger_num++;
	};
	gettimeofday(&tv,NULL);
	printf("\nRUN END   : timestamp %ld.%06lu\n",tv.tv_sec,tv.tv_usec);
	fprintf(fconf,"\nRUN END   : timestamp %ld.%06lu\n",tv.tv_sec,tv.tv_usec);
	
	fclose(ofile);
	fclose(fconf);
	
	contextClose(ctx);
}
