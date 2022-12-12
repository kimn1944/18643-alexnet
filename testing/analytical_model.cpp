#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>

typedef int index_t;
typedef struct Tiles {
    int TR;
    int TC;
    int TM;
    int TN;
} tiles_t;

//
// Layer 4
//
#define r_4 (8)                                 // height
#define c_4 (8)                                 // width
#define m_4 (128)                           // depth
#define n_4 (256)                              // depth
#define r_i4 (r_4*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i4 (c_4*S_WTS+K_WTS-S_WTS)       // derived width

//
// Layer 3
//
#define r_3 (18)                             // height
#define c_3 (18)                             // width
#define m_3 (256)                             // depth
#define n_3 (256)                             // depth
#define r_i3 (r_3*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i3 (c_3*S_WTS+K_WTS-S_WTS)       // derived width

//
// Layer 2
//
#define r_2 (38)                         // height
#define c_2 (38)                         // width
#define m_2 (256)                         // depth
#define n_2 (128)                              // depth
#define r_i2 (r_2*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i2 (c_2*S_WTS+K_WTS-S_WTS)       // derived width

//
// Layer 1
//
#define r_1 (78)                             // height
#define c_1 (78)                             // width
#define m_1 (128)                             // depth
#define n_1 (64)                             // depth
#define r_i1 (r_1*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i1 (c_1*S_WTS+K_WTS-S_WTS)       // derived width

//
// Layer 0
//
#define r_0 (158)                         // height
#define c_0 (158)                         // width
#define m_0 (64)                         // depth
#define n_0 (4)                              // depth
#define r_i0 (r_0*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i0 (c_0*S_WTS+K_WTS-S_WTS)       // derived width

#define R_OFM(layer) ((layer == 0) ? r_0 : ((layer == 1) ? r_1 : ((layer == 2) ? r_2 : ((layer == 3) ? r_3 : r_4))))
#define C_OFM(layer) ((layer == 0) ? c_0 : ((layer == 1) ? c_1 : ((layer == 2) ? c_2 : ((layer == 3) ? c_3 : c_4))))
#define M_OFM(layer) ((layer == 0) ? m_0 : ((layer == 1) ? m_1 : ((layer == 2) ? m_2 : ((layer == 3) ? m_3 : m_4))))
#define N_IFM(layer) ((layer == 0) ? n_0 : ((layer == 1) ? n_1 : ((layer == 2) ? n_2 : ((layer == 3) ? n_3 : n_4))))
#define R_IFM(layer) ((layer == 0) ? r_i0 : ((layer == 1) ? r_i1 : ((layer == 2) ? r_i2 : ((layer == 3) ? r_i3 : r_i4))))
#define C_IFM(layer) ((layer == 0) ? c_i0 : ((layer == 1) ? c_i1 : ((layer == 2) ? c_i2 : ((layer == 3) ? c_i3 : c_i4))))

#define K_WTS 4
#define S_WTS 2 


//int TR_0 = 158;
//int Tc = 158;
//int Tm = 64;
//int Tm = 4;

int MIN(int i, int j){
	return (i > j) ? j : i;
}

int total_O_accesses(int layer, int Tr, int Tc, int Tm, int Tn){
	int O_total = 0;
	for(int row = 0; row < R_OFM(layer); row += Tr) {
		index_t trr_max = MIN(row + Tr, R_OFM(layer)) - row;
		index_t xrr_max = trr_max * S_WTS + K_WTS - S_WTS;

		for(int col = 0; col < C_OFM(layer); col += Tc) {
			index_t tcc_max = MIN(col + Tc, C_OFM(layer)) - col;
			index_t xcc_max = tcc_max * S_WTS + K_WTS - S_WTS;
			
			for(int to = 0; to < M_OFM(layer); to += Tm) {
            			index_t too_max;
				too_max = MIN(to + Tm, M_OFM(layer)) - to;

				for(int ti = 0; ti < N_IFM(layer); ti += Tn) {

					index_t tii_max = MIN(ti + Tn, N_IFM(layer)) - ti;
					O_total += tii_max*xrr_max*xcc_max;
					O_total += too_max*tii_max*K_WTS*K_WTS;

				}

				O_total += too_max*tcc_max*trr_max;
			}
		}
	}

	return O_total;
}

int total_I_accesses(int layer, int Tr, int Tc, int Tm, int Tn){
	int I_total = 0;
	for(int row = 0; row < R_OFM(layer); row += Tr) {
		index_t trr_max = MIN(row + Tr, R_OFM(layer)) - row;
		index_t xrr_max = trr_max * S_WTS + K_WTS - S_WTS;
		for(int col = 0; col < C_OFM(layer); col += Tc) {
			index_t tcc_max = MIN(col + Tc, C_OFM(layer)) - col;
			index_t xcc_max = tcc_max * S_WTS + K_WTS - S_WTS;

			for(int ti = 0; ti < N_IFM(layer); ti += Tn) {
				index_t tii_max = MIN(ti + Tn, N_IFM(layer)) - ti;
				I_total += tii_max*xrr_max*xcc_max;

				for(int to = 0; to < M_OFM(layer); to += Tm) {
            				index_t too_max = MIN(to + Tm, M_OFM(layer)) - to;
					I_total += too_max*tii_max*K_WTS*K_WTS;
					I_total += 2*too_max*tcc_max*trr_max;
				}
			}
		}
	}
	return I_total;
}

int total_W_accesses(int layer, int Tr, int Tc, int Tm, int Tn){
	int W_total = 0;
	for(int to = 0; to < M_OFM(layer); to += Tm) {
            	index_t too_max = MIN(to + Tm, M_OFM(layer)) - to;

		for(int ti = 0; ti < N_IFM(layer); ti += Tn) {
			index_t tii_max = MIN(ti + Tn, N_IFM(layer)) - ti;
			W_total += too_max*tii_max*K_WTS*K_WTS;

			for(int row = 0; row < R_OFM(layer); row += Tr) {
				index_t trr_max = MIN(row + Tr, R_OFM(layer)) - row;
				index_t xrr_max = trr_max * S_WTS + K_WTS - S_WTS;

				for(int col = 0; col < C_OFM(layer); col += Tc) {
					index_t tcc_max = MIN(col + Tc, C_OFM(layer)) - col;
					index_t xcc_max = tcc_max * S_WTS + K_WTS - S_WTS;

					W_total += tii_max*xrr_max*xcc_max;
					W_total += 2*too_max*tcc_max*trr_max;
				}
			}
		}
	}
	return W_total;
}

double find_ideal_tiles(int layer){
	int DSP_factor = 86; 
	char ta_type;
	double ideal_ta = 100000000;
	double wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
	int cap = 200;
        int TR_max = (R_OFM(layer) <= cap) ? R_OFM(layer) : cap;
        int TC_max = (C_OFM(layer) <= cap) ? C_OFM(layer) : cap;

	for (int TR = 1; TR <= TR_max; TR++){
		for (int TC = 1; TC <= TC_max; TC++){
			for (int TM = 1; TM <= M_OFM(layer); TM++){
				for (int TN = 1; TN <= N_IFM(layer); TN++){
					if (TN * TM <= DSP_factor) {
						wta = total_W_accesses(layer, TR, TC, TM, TN);
						ita = total_I_accesses(layer, TR, TC, TM, TN);
						ota = total_O_accesses(layer, TR, TC, TM, TN);
						if (wta < ideal_ta) {
							ta_type = 'w';
							ideal_ta = wta;
							ideal_TN = TN;
							ideal_TM = TM;
							ideal_TR = TR;
							ideal_TC = TC;
						}
						if (ita < ideal_ta){
							ta_type = 'i';
							ideal_ta = ita;
							ideal_TN = TN;
							ideal_TM = TM;
							ideal_TR = TR;
							ideal_TC = TC;
						}
						if (ota < ideal_ta){
							ta_type = 'o';
							ideal_ta = ota;
							ideal_TN = TN;
							ideal_TM = TM;
							ideal_TR = TR;
							ideal_TC = TC;
						}
					}
				}
			}
		}
	}
	//printf("%d,%d,%d,%d,%d\n", DSP_factor, ideal_TR, ideal_TC, ideal_TM, ideal_TN);
	printf("LAYER%d: TR=%d TC=%d TM=%d TN=%d --> (%c) ideal_ta: %f\n", layer, ideal_TR, ideal_TC, ideal_TM, ideal_TN, ta_type, ideal_ta);
	return ideal_ta;
}

void find_ideal_tiles_DSP(int layer){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
	int cap = 64;
        int TR_max = (R_OFM(layer) <= cap) ? R_OFM(layer) : cap;
        int TC_max = (C_OFM(layer) <= cap) ? C_OFM(layer) : cap;

	printf("DSP_factor,ideal_TR,ideal_TC,ideal_TM,ideal_TN\n");
	for(int DSP_factor = 80; DSP_factor <= 120; DSP_factor++){
		for (int TR = 1; TR <= TR_max; TR++){
			for (int TC = 1; TC <= TC_max; TC++){
				for (int TM = 1; TM <= M_OFM(layer); TM++){
					for (int TN =
							1; TN <= N_IFM(layer); TN++){
						if (TN * TM <= DSP_factor) {
							wta = total_W_accesses(layer, TR, TC, TM, TN);
							ita = total_I_accesses(layer, TR, TC, TM, TN);
							ota = total_O_accesses(layer, TR, TC, TM, TN);
							if (wta < ideal_ta) {
								ta_type = 'w';
								ideal_ta = wta;
								ideal_TN = TN;
								ideal_TM = TM;
								ideal_TR = TR;
								ideal_TC = TC;
							}
							if (ita < ideal_ta){
								ta_type = 'i';
								ideal_ta = ita;
								ideal_TN = TN;
								ideal_TM = TM;
								ideal_TR = TR;
								ideal_TC = TC;
							}
							if (ota < ideal_ta){
								ta_type = 'o';
								ideal_ta = ota;
								ideal_TN = TN;
								ideal_TM = TM;
								ideal_TR = TR;
								ideal_TC = TC;
							}
						}
					}
				}
			}
		}
		printf("%d,%d,%d,%d,%d\n",DSP_factor,ideal_TR,ideal_TC,ideal_TM,ideal_TN);
		//printf("%d: TR=%d TC=%d TM=%d TN=%d --> (%c) ideal_ta: %d\n", layer, ideal_TR, ideal_TC, ideal_TM, ideal_TN, ta_type, ideal_ta);
	}
}

void find_ideal_tiles_cap(int layer){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
    int cap = 65;
    //std::map<int,tiles_t> ideal_tas; 

    int TR_min, TR_max, TC_min, TC_max;
    TR_max = (R_OFM(layer) <= cap) ? R_OFM(layer) : cap;
    TC_max = (C_OFM(layer) <= cap) ? C_OFM(layer) : cap;
    if (layer == 4){
        TR_min = 8;
        TC_min = 8;
    } else if (layer == 3){
        TR_min = 18;
        TC_min = 18;
    } else {
        TR_min = 24;
        TC_min = 24;
    }

	printf("TR,TC,TM,TN,wta,ita,ota,ideal_ta\n");
	for (int TR = TR_min; TR <= TR_max; TR++){
       int TC = TR;
	   for (int TM = 1; TM <= M_OFM(layer); TM++){
	    	for (int TN = 1; TN <= N_IFM(layer); TN++){
	    		if ((TN * TM <= 86) && (TN * TM >= 16)) {
	    			wta = total_W_accesses(layer, TR, TC, TM, TN);
	    			ita = total_I_accesses(layer, TR, TC, TM, TN);
	    			ota = total_O_accesses(layer, TR, TC, TM, TN);
                    int ideal_ta = (wta < ita) ? wta : ita;
                    ideal_ta = (ideal_ta < ota) ? ideal_ta : ota;
                    //ideal_tas[ideal_ta] = tt; 

                    //if (!ideal_tas.count(ideal_ta)) {
                    //    tiles_t tt = {TR, TC, TM, TN};
                    //    ideal_tas[ideal_ta] = tt; 
                    //    printf("%d,%d,%d,%d,%d,%d,%d,%d\n",TR,TC,TM,TN,wta,ita,ota,ideal_ta);
                    //}
	    		}
	    	}
	   }
    }
	//printf("%d: TR=%d TC=%d TM=%d TN=%d --> (%c) ideal_ta: %d\n", layer, ideal_TR, ideal_TC, ideal_TM, ideal_TN, ta_type, ideal_ta);
	
}

void find_ideal_tiles_2combined(int layer0, int layer1){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
    int cap = 64;
    std::map<int,tiles_t> ideal_tas; 

	int TR_max0 = (R_OFM(layer0) <= cap) ? R_OFM(layer0) : cap;
	int TR_max1 = (R_OFM(layer1) <= cap) ? R_OFM(layer1) : cap;
	
    int TR_min;
    if ((layer0 == 4) || (layer1 == 4)){
        TR_min = 8;
    } else if ((layer0 == 3) || (layer1 == 3)){
        TR_min = 18;
    } else {
        TR_min = 24;
    }

    int TR_max = MIN(TR_max0,TR_max1);
    int TM_max = MIN(M_OFM(layer0),M_OFM(layer1));
    int TN_max = MIN(N_IFM(layer0),N_IFM(layer1));

	printf("TR,TC,TM,TN,wta,ita,ota,ideal_ta\n");
	for (int TR = TR_min; TR <= TR_max; TR++){
       int TC = TR;
	   for (int TM = 1; TM <= TM_max; TM++){
	    	for (int TN = 1; TN <= TN_max; TN++){
	    		if ((TN * TM <= 86) && (TN * TM >= 16)) {
	    			int wta0 = total_W_accesses(layer0, TR, TC, TM, TN);
	    			int wta1 = total_W_accesses(layer1, TR, TC, TM, TN);
                    int wta = wta0 + wta1;

	    			int ita0 = total_I_accesses(layer0, TR, TC, TM, TN);
	    			int ita1 = total_I_accesses(layer1, TR, TC, TM, TN);
                    int ita = ita0 + ita1;

	    			int ota0 = total_O_accesses(layer0, TR, TC, TM, TN);
	    			int ota1 = total_O_accesses(layer1, TR, TC, TM, TN);
                    int ota = ota0 + ota1;

                    int ideal_ta = (wta < ita) ? wta : ita;
                    ideal_ta = (ideal_ta < ota) ? ideal_ta : ota;

                    if (!ideal_tas.count(ideal_ta)) {
                        tiles_t tt = {TR, TC, TM, TN};
                        ideal_tas[ideal_ta] = tt; 
                        printf("%d,%d,%d,%d,%d,%d,%d,%d\n",TR,TC,TM,TN,wta,ita,ota,ideal_ta);
                    }
	    		}
	    	}
	   }
    }
}

void find_ideal_tiles_3combined(int layer0, int layer1, int layer2){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
    int cap = 64;
    std::map<int,tiles_t> ideal_tas; 

	int TR_max0 = (R_OFM(layer0) <= cap) ? R_OFM(layer0) : cap;
	int TR_max1 = (R_OFM(layer1) <= cap) ? R_OFM(layer1) : cap;
	int TR_max2 = (R_OFM(layer2) <= cap) ? R_OFM(layer2) : cap;
	
    int TR_min;
    if ((layer0 == 4) || (layer1 == 4) || (layer2 == 4)){
        TR_min = 8;
    } else if ((layer0 == 3) || (layer1 == 3) || (layer2 == 3)){
        TR_min = 18;
    } else {
        TR_min = 24;
    }

    int TR_max = MIN(TR_max0,TR_max1);
    TR_max = MIN(TR_max, TR_max2);

    int TM_max = MIN(M_OFM(layer0),M_OFM(layer1));
    TM_max = MIN(TM_max,M_OFM(layer2));

    int TN_max = MIN(N_IFM(layer0),N_IFM(layer1));
    TN_max = MIN(TN_max,N_IFM(layer2));

	printf("TR,TC,TM,TN,wta,ita,ota,ideal_ta\n");
	for (int TR = TR_min; TR <= TR_max; TR++){
       int TC = TR;
	   for (int TM = 1; TM <= MIN(M_OFM(layer0),M_OFM(layer1)); TM++){
	    	for (int TN = 1; TN <= MIN(N_IFM(layer0), N_IFM(layer1)); TN++){
	    		if ((TN * TM <= 86) && (TN * TM >= 16)) {
	    			int wta0 = total_W_accesses(layer0, TR, TC, TM, TN);
	    			int wta1 = total_W_accesses(layer1, TR, TC, TM, TN);
	    			int wta2 = total_W_accesses(layer2, TR, TC, TM, TN);
                    int wta = wta0 + wta1 + wta2;

	    			int ita0 = total_I_accesses(layer0, TR, TC, TM, TN);
	    			int ita1 = total_I_accesses(layer1, TR, TC, TM, TN);
	    			int ita2 = total_I_accesses(layer2, TR, TC, TM, TN);
                    int ita = ita0 + ita1 + ita2;

	    			int ota0 = total_O_accesses(layer0, TR, TC, TM, TN);
	    			int ota1 = total_O_accesses(layer1, TR, TC, TM, TN);
	    			int ota2 = total_O_accesses(layer2, TR, TC, TM, TN);
                    int ota = ota0 + ota1 + ota2;

                    int ideal_ta = (wta < ita) ? wta : ita;
                    ideal_ta = (ideal_ta < ota) ? ideal_ta : ota;

                    if (!ideal_tas.count(ideal_ta)) {
                        tiles_t tt = {TR, TC, TM, TN};
                        ideal_tas[ideal_ta] = tt; 
                        printf("%d,%d,%d,%d,%d,%d,%d,%d\n",TR,TC,TM,TN,wta,ita,ota,ideal_ta);
                    }
	    		}
            }
       }
    }
}

void find_ideal_tiles_4combined(int layer0, int layer1, int layer2, int layer3){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
    int cap = 64;
    std::map<int,tiles_t> ideal_tas; 

	int TR_max0 = (R_OFM(layer0) <= cap) ? R_OFM(layer0) : cap;
	int TR_max1 = (R_OFM(layer1) <= cap) ? R_OFM(layer1) : cap;
	int TR_max2 = (R_OFM(layer2) <= cap) ? R_OFM(layer2) : cap;
	int TR_max3 = (R_OFM(layer3) <= cap) ? R_OFM(layer3) : cap;
	
    int TR_min;
    if ((layer0 == 4) || (layer1 == 4) || (layer2 == 4) || (layer3 == 4)){
        TR_min = 8;
    } else if ((layer0 == 3) || (layer1 == 3) || (layer2 == 3) || (layer3 == 3)){
        TR_min = 18;
    } else {
        TR_min = 24;
    }

    int TR_max = MIN(TR_max0,TR_max1);
    TR_max = MIN(TR_max, TR_max2);
    TR_max = MIN(TR_max, TR_max3);

    int TM_max = MIN(M_OFM(layer0),M_OFM(layer1));
    TM_max = MIN(TM_max,M_OFM(layer2));
    TM_max = MIN(TM_max,M_OFM(layer3));

    int TN_max = MIN(N_IFM(layer0),N_IFM(layer1));
    TN_max = MIN(TN_max,N_IFM(layer2));
    TN_max = MIN(TN_max,N_IFM(layer3));

	printf("TR,TC,TM,TN,wta,ita,ota,ideal_ta\n");
	for (int TR = TR_min; TR <= TR_max; TR++){
       int TC = TR;
	   for (int TM = 1; TM <= TM_max; TM++){
	    	for (int TN = 1; TN <= TN_max; TN++){
	    		if ((TN * TM <= 86) && (TN * TM >= 16)) {
	    			int wta0 = total_W_accesses(layer0, TR, TC, TM, TN);
	    			int wta1 = total_W_accesses(layer1, TR, TC, TM, TN);
	    			int wta2 = total_W_accesses(layer2, TR, TC, TM, TN);
	    			int wta3 = total_W_accesses(layer3, TR, TC, TM, TN);
                    int wta = wta0 + wta1 + wta2 + wta3;

	    			int ita0 = total_I_accesses(layer0, TR, TC, TM, TN);
	    			int ita1 = total_I_accesses(layer1, TR, TC, TM, TN);
	    			int ita2 = total_I_accesses(layer2, TR, TC, TM, TN);
	    			int ita3 = total_I_accesses(layer3, TR, TC, TM, TN);
                    int ita = ita0 + ita1 + ita2 + ita3;

	    			int ota0 = total_O_accesses(layer0, TR, TC, TM, TN);
	    			int ota1 = total_O_accesses(layer1, TR, TC, TM, TN);
	    			int ota2 = total_O_accesses(layer2, TR, TC, TM, TN);
	    			int ota3 = total_O_accesses(layer3, TR, TC, TM, TN);
                    int ota = ota0 + ota1 + ota2 + ota3;

                    int ideal_ta = (wta < ita) ? wta : ita;
                    ideal_ta = (ideal_ta < ota) ? ideal_ta : ota;

                    if (!ideal_tas.count(ideal_ta)) {
                        tiles_t tt = {TR, TC, TM, TN};
                        ideal_tas[ideal_ta] = tt; 
                        printf("%d,%d,%d,%d,%d,%d,%d,%d\n",TR,TC,TM,TN,wta,ita,ota,ideal_ta);
                    }
	    		}
            }
       }
    }
}

void find_ideal_tiles_5combined(int layer0, int layer1, int layer2, int layer3, int layer4){
	char ta_type;
	int ideal_ta = 100000000;
	int wta, ita, ota;
	int ideal_TN, ideal_TM, ideal_TR, ideal_TC = 0;
    int cap = 64;
    std::map<int,tiles_t> ideal_tas; 

    int TN_max = 4;
    int TM_max = 64;

	printf("TR,TC,TM,TN,wta,ita,ota,ideal_ta\n");
    int TR = 8;
    int TC = TR;
	for (int TM = 1; TM <= TM_max; TM++){
	 	for (int TN = 1; TN <= TN_max; TN++){
	 		if ((TN * TM <= 86) && (TN * TM >= 16)) {
	 			int wta0 = total_W_accesses(layer0, TR, TC, TM, TN);
	 			int wta1 = total_W_accesses(layer1, TR, TC, TM, TN);
	 			int wta2 = total_W_accesses(layer2, TR, TC, TM, TN);
	 			int wta3 = total_W_accesses(layer3, TR, TC, TM, TN);
	 			int wta4 = total_W_accesses(layer4, TR, TC, TM, TN);
                 int wta = wta0 + wta1 + wta2 + wta3 + wta4;

	 			int ita0 = total_I_accesses(layer0, TR, TC, TM, TN);
	 			int ita1 = total_I_accesses(layer1, TR, TC, TM, TN);
	 			int ita2 = total_I_accesses(layer2, TR, TC, TM, TN);
	 			int ita3 = total_I_accesses(layer3, TR, TC, TM, TN);
	 			int ita4 = total_I_accesses(layer4, TR, TC, TM, TN);
                 int ita = ita0 + ita1 + ita2 + ita3 + ita4;

	 			int ota0 = total_O_accesses(layer0, TR, TC, TM, TN);
	 			int ota1 = total_O_accesses(layer1, TR, TC, TM, TN);
	 			int ota2 = total_O_accesses(layer2, TR, TC, TM, TN);
	 			int ota3 = total_O_accesses(layer3, TR, TC, TM, TN);
	 			int ota4 = total_O_accesses(layer4, TR, TC, TM, TN);
                 int ota = ota0 + ota1 + ota2 + ota3 + ota4;

                 int ideal_ta = (wta < ita) ? wta : ita;
                 ideal_ta = (ideal_ta < ota) ? ideal_ta : ota;

                 if (!ideal_tas.count(ideal_ta)) {
                     tiles_t tt = {TR, TC, TM, TN};
                     ideal_tas[ideal_ta] = tt; 
                     printf("%d,%d,%d,%d,%d,%d,%d,%d\n",TR,TC,TM,TN,wta,ita,ota,ideal_ta);
                 }
	 		}
         }
    }
    
}

void run_set_tiles(int layer, int TR, int TC, int TM, int TN){
	int wta = total_W_accesses(layer, TR, TC, TM, TN);
	int ita = total_I_accesses(layer, TR, TC, TM, TN);
	int ota = total_O_accesses(layer, TR, TC, TM, TN);
	printf("LAYER%d: TR=%d TC=%d TM=%d TN=%d --> wta=%d, ita=%d, ota=%d\n", layer, TR, TC, TM, TN, wta, ita, ota);
}

int main(int argc, char **argv){
	//for (int i = 16; i <= 64; i+=2) run_set_tiles(0, 158, 158, i, 4);
	//run_set_tiles(0, 158, 158, 64, 1); 
	//run_set_tiles(0, 158, 158, 64, 2); 
	//run_set_tiles(0, 158, 158, 64, 3); 
	//run_set_tiles(0, 158, 158, 64, 4); 
	//for (int i = 0; i < 5; i++) find_ideal_tiles(i);
    int layer0 = strtol(argv[1], NULL, 10);
    int layer1 = strtol(argv[2], NULL, 10);
    int layer2 = strtol(argv[3], NULL, 10);
    int layer3 = strtol(argv[4], NULL, 10);
    int layer4 = strtol(argv[5], NULL, 10);
    find_ideal_tiles_5combined(layer0, layer1, layer2, layer3, layer4);

	return 0;
}
