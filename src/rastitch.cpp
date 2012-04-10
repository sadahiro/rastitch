//============================================================================
// Name        : rastitch.cpp
// Author      : Makoto Sadahiro
// Version     :
// Copyright   : Texas Advanced Computing Center
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

long int rorig_x, rorig_y;
//long long int rdim_x, rdim_y;
long int rdim_x, rdim_y;
double rmax_uvw, rmin_uvw;

long int orig_x, orig_y;//, min_rorig_x, min_rorig_y;
long int jaxorig_x, jaxorig_y;
double max_uvw, min_uvw;
long int dim_x, dim_y;
double** dRas;
long int dim_xy, rdim_xy;

void getRange(const string aFileName){

	FILE* pFile;
	pFile = fopen(aFileName.c_str(), "r");
	cout << "scanning " << aFileName << endl;

	int tmpI;
	fread(&tmpI, sizeof(int), 1, pFile);
	rorig_x = tmpI;
	fread(&tmpI, sizeof(int), 1, pFile);
	rorig_y = tmpI;
	cout << "origin:(" << rorig_x << ", " << rorig_y << ")" << endl;

	fread(&tmpI, sizeof(int), 1, pFile);
	rdim_x = tmpI;
	fread(&tmpI, sizeof(int), 1, pFile);
	rdim_y = tmpI;
	cout << "dim:(" << rdim_x << " x " << rdim_y << ")" << endl;

	double rasD;
	fread(&rasD, sizeof(double), 1, pFile);
	rmax_uvw = rasD;
	fread(&rasD, sizeof(double), 1, pFile);
	rmin_uvw = rasD;
	cout << "value range:" << rmin_uvw << " .. " << rmax_uvw << endl
			<< endl;

	if(rorig_x < orig_x){
		orig_x = rorig_x;
		cout <<"orig_x updated: " << orig_x << ", " << orig_y << endl;
	}
	if(jaxorig_x < (rorig_x + rdim_x)){
		jaxorig_x = rorig_x + rdim_x;
	}
	if(rorig_y < orig_y){
		orig_y = rorig_y;
		cout <<"orig_y updated: " << orig_x << ", " << orig_y << endl
				<< endl;
	}
	if(jaxorig_y < (rorig_y + rdim_y)){
		jaxorig_y = rorig_y + rdim_y;
	}
	if(max_uvw < rmax_uvw){
		max_uvw = rmax_uvw;
	}
	if(rmin_uvw < min_uvw){
		min_uvw = rmin_uvw;
	}
	cout << "current matrix" << endl
			<< "(" << orig_x << ", " << orig_y << ") .. ("
			<< jaxorig_x << ", " << orig_y << ")" << endl
			<< " .     ." << endl
			<< "(" << orig_x << ", " << jaxorig_y << ") .. ("
			<< jaxorig_x << ", " << jaxorig_y << ")" << endl
			<< endl
			<< "uvw range: " << min_uvw << " .. " << max_uvw << endl
			<< endl;

	// clean up
	fclose(pFile);
}

int main(int argc, char *argv[]) {

	orig_x = orig_y = numeric_limits<long int>::max();
	jaxorig_x = jaxorig_y = -numeric_limits<long int>::max();
	max_uvw = -numeric_limits<double>::max();
	min_uvw = numeric_limits<double>::max();
	dim_x = dim_y = dim_xy = rdim_xy = 0;

	// survey the range first (first reading cycle)
	string srcFileName = argv[1];
	ifstream ifstreamFileIn;
	ifstreamFileIn.open(srcFileName.c_str());
	string s_line;
	while(ifstreamFileIn){
		getline(ifstreamFileIn, s_line);
//		cout << s_line << endl;
		if(s_line==""){
			cout << "end of file list." << endl;
		}
		else{
		cout << "sending job to getRange:" << s_line << endl;
		getRange(s_line);
		}
	}
	ifstreamFileIn.close();

	// create output array
	dim_x = jaxorig_x - orig_x + 1;
	dim_y = jaxorig_y - orig_y + 1;
	dRas = new double*[dim_y];
	for(long int y = 0; y < dim_y; ++y){
		dRas[y] = new double[dim_x];
		for(long int x = 0; x < dim_x; ++x){
			dRas[y][x] = 0.0;
		}
	}
	cout << "gRaster(dRas) generated" << endl;
	cout << "orig:" << orig_x << "," << orig_y << endl
			<< "  dim:" << dim_x << "," << dim_y << endl;

	// second read cycle
	ifstreamFileIn.open(srcFileName.c_str());
	while(ifstreamFileIn){
		getline(ifstreamFileIn, s_line);
		if(s_line == ""){
			cout << "end of file list." << endl;
		}else{

			// copy partials to global raster, dRas
			FILE* pFile;
			pFile = fopen(s_line.c_str(), "r");

			int tmpI;
			fread(&tmpI, sizeof(int), 1, pFile);
			rorig_x = tmpI;
			fread(&tmpI, sizeof(int), 1, pFile);
			rorig_y = tmpI;
			cout << "rorigin:" << rorig_x << "," << rorig_y << endl;

			fread(&tmpI, sizeof(int), 1, pFile);
			rdim_x = tmpI;
			fread(&tmpI, sizeof(int), 1, pFile);
			rdim_y = tmpI;
			cout << "rdim:" << rdim_x << "," << rdim_y << endl;

			rdim_xy = rdim_x * rdim_y;

			double rasD;
			fread(&rasD, sizeof(double), 1, pFile);
			rmax_uvw = rasD;
			fread(&rasD, sizeof(double), 1, pFile);
			rmin_uvw = rasD;
			cout << "uvw:" << min_uvw << " .. " << max_uvw << endl;

			// here make temp array
//			double** tdRas = new double*[rdim_y];
//			for(long int y = 0; y < rdim_y; ++y){
//				tdRas[y] = new double[rdim_x];
//				for(long int x = 0; x < rdim_x; ++x){
//					tdRas[y][x] = 0.0;
//				}
//			}
			double* tdRas = new double [rdim_xy];
			cout << "rdRas generated" << endl;

			// here read that array
			fread(tdRas, sizeof(double), rdim_xy, pFile);

			// closing one Zone file
			fclose(pFile);

//			for(long int y = 0; y < rdim_y; ++y){
//				for(long int x = 0; x < rdim_x; ++x){
//					if(tdRas[y*rdim_x+x]==0.0){
//						cout << ".";
//					}
//					else{
//					cout << "*";
//					}
//				}
//				cout << endl;
//			}
//			cout << "reading temp values done" << endl;

			cout << "gdim:" << dim_x << "," << dim_y << endl
					<< "rorig:" << rorig_x << "," << rorig_y << endl
					<< "rdim:" << rdim_x << "," << rdim_y << endl;

			// here copy that array
			cout << "copying l to g where gdim:: " << dim_x << "," << dim_y << endl;
			for(long int y = 0; y < rdim_y; ++y){
				for(long int x = 0; x < rdim_x; ++x){
					if(tdRas[(y * rdim_x) + x]!=0.0){
						cout << x + (rorig_x - orig_x) << ", " << y + (rorig_y - orig_y) << endl;
						dRas[y + (rorig_y - orig_y)][x + (rorig_x - orig_x)] = tdRas[(y * rdim_x) + x];
					}
//					if(0.0==(tdRas[(y * rdim_x) + x])){
//						cout << ".";
//					}
//					else{
//						cout << "*";
//					}
				}
//				cout << endl;
			}

			delete[] tdRas;

			cout << "end of a zone file: " << s_line << endl;
		}
	} // end of second read cycle
	ifstreamFileIn.close();

	// write out DRB file
//	cout << "test" << endl;
//	for(long int y = 0; y < dim_y; ++y){
//		for(long int x = 0; x < dim_x; ++x){
//			if(dRas[y][x]!=0.0) cout << dRas[y][x];
//		}
//	}
//	cout << endl;
	FILE* pFile;
	pFile = fopen((srcFileName+".drb").c_str(), "w");

	int tmpI;
	tmpI = orig_x;
	fwrite(&tmpI, sizeof(int), 1, pFile);
	tmpI = orig_y;
	fwrite(&tmpI, sizeof(int), 1, pFile);

	tmpI = dim_x;
	fwrite(&tmpI, sizeof(int), 1, pFile);
	tmpI = dim_y;
	fwrite(&tmpI, sizeof(int), 1, pFile);

	double rasD;
	rasD = max_uvw;
	fwrite(&rasD, sizeof(double), 1, pFile);
	rasD = min_uvw;
	fwrite(&rasD, sizeof(double), 1, pFile);

//	dim_xy = dim_x * dim_y;
//	fwrite(dRas, sizeof(double), dim_xy, pFile);
	for(long int y = 0; y < dim_y; y++){
		for(long int x = 0; x < dim_x; x++){
			fwrite(&dRas[y][x], sizeof(double), 1, pFile);
		}
	}

	fclose(pFile);

	// clean up memory
	cout << "deleting dRas from heap" << endl;
	for(long int y = 0; y < dim_y; ++y){
		delete[] dRas[y];
	}
	delete[] dRas;

	cout << "exiting rastitch." << endl;
	return 0;
}
