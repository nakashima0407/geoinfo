#include "gpspoint.h"

double gpspoint::distanceTo(const gpspoint &q) const {
	//http://dtan4.hatenablog.com/entry/2013/06/10/013724
	//https://en.wikipedia.org/wiki/Geographical_distance#Ellipsoidal_Earth_projected_to_a_plane
	//https://stackoverflow.com/questions/27928/calculate-distance-between-two-latitude-longitude-points-haversine-formula
	//const int mode = 1;

	// convert degree values to radians
	double plat = DEG2RAD(lat), plon = DEG2RAD(lon), 
			qlat = DEG2RAD(q.lat), qlon = DEG2RAD(q.lon);

	// 緯度と経度の差
	//double latdiff = plat - qlat, londiff = plon - qlon;
	// the average of lattitude values
	double latavr = (plat + qlat) / 2.0;

	// 測地系による値の違い
	double a = 6378137.0; //mode ? 6378137.0 : 6377397.155; // 赤道半径
	//double b = 6356752.314140356; //mode ? 6356752.314140356 : 6356078.963; // 極半径
	//$e2 = ($a*$a - $b*$b) / ($a*$a);
	double e2 = 0.00669438002301188; //mode ? 0.00669438002301188 : 0.00667436061028297; // 第一離心率^2
	//$a1e2 = $a * (1 - $e2);
	double a1e2 = 6335439.32708317; // mode ? 6335439.32708317 : 6334832.10663254; // 赤道上の子午線曲率半径

	double sin_latavr = sin(latavr);
	double W2 = 1.0 - e2 * (sin_latavr * sin_latavr);
	double M = a1e2 / (sqrt(W2) * W2); // 子午線曲率半径M
	double N = a / sqrt(W2); // 卯酉線曲率半径

	double t1 = M * (plat - qlat); //latdiff;
	double t2 = N * cos(latavr) * (plon - qlon); //londiff;
	return sqrt((t1 * t1) + (t2 * t2));
}

double gpspoint::distanceTo(const gpspoint &q1, const gpspoint &q2) const {
	if ( inner_prod(q1, q2, *this) < epsilon ) { // < 0.0
		return q1.distanceTo(*this);
	}
	if ( inner_prod(q2, q1, *this) < epsilon ) { // < 0.0
		return q2.distanceTo(*this);
	}
	return ABS(norm_outer_prod(q1, q2, *this)) / q1.distanceTo(q2);
}


std::vector<std::pair<float,float>> gpspoint::lcs(
		std::vector<gpspoint> &pseq, std::vector<gpspoint> &qseq,
		const double &bound) {
	dptable table(qseq.size(), pseq.size());
	unsigned int ip, iq; // column, row

	// computing the top-frame and the left side-frame cells
	for (ip = 0; ip < pseq.size(); ++ip) {
		table(0, ip).clear();
	}
	for (iq = 0; iq < qseq.size(); ++iq) {
		// ip == 0
		table(iq, 0).clear();
	
		if ( iq != 0 && pseq[0].distanceTo(qseq[iq-1], qseq[iq]) <= bound )
			table(iq,0).lp = 1;
	}

	// computing inner cells by the inductive relation
	// iq -- row, ip -- column
	for (iq = 1; iq < qseq.size(); ++iq) {
		for (ip = 1; ip < pseq.size(); ++ip) {
			table(iq,ip).clear();
			//  lp (iq-1, [iq <-> ip])
			if ( pseq[ip].distanceTo(qseq[iq-1],qseq[iq]) <= bound ) {
				table(iq,ip).lp = 1;
			}
			table(iq,ip).lp = MAX_AMONG3(
					table(iq-1,ip).lp,
					table(iq-1,ip-1).lp + table(iq,ip).lp,
					table(iq,ip-1).lp );
		
		}
	}

	// back track the subsequence
	std::vector<std::pair<float,float>> matchedpairs;
	ip = pseq.size() - 1;
	iq = qseq.size() - 1;

	while (ip > 0 && iq > 0) {
		//skip through
			if ( table(iq,ip).lp == table(iq-1,ip).lp ) {
				iq -= 1;	
			} else if ( table(iq, ip).lp == table(iq,ip-1).lp ) {
				ip -= 1;
			} else if ( table(iq,ip).lp == table(iq-1,ip-1).lp ) {
				ip -= 1;
				iq -= 1;
			} else if ( table(iq,ip).lp == table(iq-1,ip-1).lp + 1 ) {
				matchedpairs.push_back(std::pair<float,float>((float)iq - 0.5, ip));
				iq -= 1;
			} else {
				std::cerr << "back trace error lp @ " << iq << ", " << ip << std::endl;
				break;
			}
			continue;
		} 
	std::reverse(matchedpairs.begin(), matchedpairs.end());
	return matchedpairs;
}



