#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <chrono>

#include <vector>
#include "gpspoint.h"

#include <iostream>
#include <fstream>

#include <iomanip>

#define FRAC(x)  ((x) - (int)(x))

int read_gpspoint_csv(char *filename, std::vector<gpspoint> &array) {
	char buff[1024];
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "file open failed.\n");
		return 0; // failed
	}
	char tstr[1024];
	double t, la, lo;
	while (fgets(buff, 1024, fp) != NULL) {
		sscanf(buff, "%[^,],%lf,%lf", tstr, &la, &lo);
		
		t = atof(tstr);
		array.push_back(gpspoint(t, la, lo));
	}
	fclose(fp);
	return 1; // succeeded
}

int main(int argc, char **argv) {
	if (!(argc > 2)) {
		fprintf(stderr, "two file names requested.\n");
		return EXIT_FAILURE;
	}
	std::vector<gpspoint> parray, qarray;
	read_gpspoint_csv(argv[1], qarray);
	read_gpspoint_csv(argv[2], parray);

// argv[1]: 移動経路(点列)
// argv[2]: 地図データ(線列)
	printf("\n%s:\n", argv[1]);
	printf("%lu points.\n", (unsigned long) qarray.size());
	printf("%s:\n", argv[2]);
	printf("%lu points.\n", (unsigned long) parray.size());

//計算結果を出力するファイルtest.csv
std::ofstream ofs("test.csv");

	auto sw = std::chrono::system_clock::now();
	std::vector<std::pair<float,float>> result = gpspoint::lcs(parray, qarray, 30);
	auto dur = std::chrono::system_clock::now() - sw;

	double similarity = 0;
	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << " millis." << std::endl;

printf("\n\n");
	std::cout << std::fixed;
	for (auto i = result.begin(); i != result.end(); ++i) {
		float iq = i->first, ip = i->second;
		if ( FRAC(iq) != 0 ) {
			printf("%.1lf ([%lf, %lf]-[%lf, %lf]) ", iq, qarray[(int)iq].lat, qarray[(int)iq].lon, qarray[1+(int)iq].lat, qarray[1+(int)iq].lon);
			ofs << std::setprecision(8) << (qarray[(int)iq].lat + qarray[1+(int)iq].lat) / 2 << "," << std::setprecision(8) << (qarray[(int)iq].lon + qarray[1+(int)iq].lon) / 2 << std::endl;
		} else {
			printf("%d ([%lf, %lf]) ",  (int)iq, qarray[iq].lat, qarray[iq].lon);
			ofs << std::setprecision(8) << qarray[iq].lat << "," << std::setprecision(8) << qarray[iq].lon << std::endl;
		}
		if ( FRAC(ip) != 0 ) {
			printf(", %.1lf ([%lf, %lf]-[%lf, %lf]) ", ip, parray[ip].lat, parray[ip].lon, parray[1+(int)ip].lat, parray[1+(int)ip].lon);
			ofs << std::setprecision(8) << (parray[(int)ip].lat + parray[1+(int)ip].lat)/2 << "," << std::setprecision(8) << (parray[(int)ip].lon + parray[1+(int)ip].lon)/2 << std::endl;
		} else {
			printf(", %d ([%lf, %lf]) ",  (int)ip, parray[ip].lat, parray[ip].lon);
			ofs << std::setprecision(8) << parray[ip].lat << "," << std::setprecision(8) << parray[ip].lon << std::endl; 

		}
		if ( FRAC(iq) == 0 && FRAC(ip) == 0 ) {
			printf("%lf", qarray[iq].distanceTo(parray[ip]) );
			similarity += 1;
		} else if ( FRAC(iq) == 0 && FRAC(ip) != 0 ) {
			printf("%lf", qarray[(int)iq].distanceTo(parray[(int)ip],parray[1+(int)ip]) );
			similarity += 0.5;
		} else if ( FRAC(iq) != 0 && FRAC(ip) == 0 ) {
			printf("%lf", parray[ip].distanceTo(qarray[(int)iq],qarray[1+(int)iq]) );
			similarity += 0.5;
		} else {
			printf("error!");
		}
		printf("\n");
	}

	return EXIT_SUCCESS; 
}
