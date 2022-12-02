/****************************************************************
 * Copyright (c) 2020~2022, 18-643 Course Staff, CMU
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.

 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.

 * The views and conclusions contained in the software and
 * documentation are those of the authors and should not be
 * interpreted as representing official policies, either expressed or
 * implied, of the FreeBSD Project.
 ****************************************************************/

/*
 * CMU 18643 Fall 2022 Lab Exercise
 *
 * You can edit this file
 */

/****************************************************************
 * Blocked convolution layer implementation
 * based on Figure 5:
 *    C. Zhang, et al., "Optimizing FPGA-based Accelerator
 *    Design for Deep Convolutional Neural Networks," FPGA, 2015.
 ****************************************************************/

#include "krnl_cnn_layer4.h"

// Prevent aliasing
#undef BATCH_SIZE

#include "util643.h"

void cnn4_blocked_kernel(cnndata_t BufI[TN_4][TR_4*S_WTS+K_WTS-S_WTS][TC_4*S_WTS+K_WTS-S_WTS],
                        cnndata_t BufO[TM_4][TR_4][TC_4],
                        cnndata_t BufW[TM_4][TN_4][K_WTS][K_WTS]);

#ifdef __VITIS_CL__
extern "C" {
#endif

#ifdef layer4_BufI_optimization

void krnl_cnn_layer4(const cnndata_t* input, const cnndata_t* weights,
        cnndata_t* output, uint64_t batch_size) {

  index_t iter;
  index_t row, col, to, ti;


  cnndata_t BufI[TN_4][TR_4*S_WTS+K_WTS-S_WTS][TC_4*S_WTS+K_WTS-S_WTS];
  cnndata_t BufO[TM_4][TR_4][TC_4];
  cnndata_t BufW[TM_4][TN_4][K_WTS][K_WTS];

#pragma HLS ARRAY_PARTITION variable=BufO type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=2 //factor=4
#pragma HLS ARRAY_PARTITION variable=BufI type=complete dim=1 //factor=4

  Batch: for(iter = 0; iter < batch_size; iter++) {        // Batch Loop
	R: for(row = 0; row < R_OFM(4); row += TR_4) {     // Tiled Row Loop
	  C: for(col = 0; col < C_OFM(4); col += TC_4) {   // Tiled Column Loop
		N: for(ti = 0; ti < N_IFM(4); ti += TN_4) {
		  index_t trr, tcc, too, tii;

		  // Load active input feature map into local buffer
			{
			  // Indices internal to the block: count from 0
			  index_t irr, icc, iii;

			  // Incremented temporary indices for input row and col
			  index_t xrr, xcc;

			  // Loop bounds
			  index_t tii_max, xrr_max, xcc_max;
			  tii_max = MIN(ti + TN_4, N_IFM(4));
			  xrr_max = MIN(row + TR_4, R_OFM(4)) * S_WTS + K_WTS - S_WTS;
			  xcc_max = MIN(col + TC_4, C_OFM(4)) * S_WTS + K_WTS - S_WTS;

			  BufI_load: for(xrr = row * S_WTS, irr = 0; xrr < xrr_max; xrr++, irr++) {
				  for(xcc = col * S_WTS, icc = 0; xcc < xcc_max; xcc++, icc++) {
					  for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
					BufI[iii][irr][icc] = ARRAYi_4(input, iter, tii, xrr, xcc,
					  batch_size, N_IFM(4), R_IFM(4), C_IFM(4));
				  }
				}
			  }
			}

		  M: for(to = 0; to < M_OFM(4); to += TM_4) {

			// Load active weights into local buffer
			{
			  // Indices internal to the block: count from 0
			  index_t ioo, iii, irr, icc;

			  // Loop bounds
			  index_t too_max, tii_max;
			  too_max = MIN(to + TM_4, M_OFM(4));
			  tii_max = MIN(ti + TN_4, N_IFM(4));

			  BufW_load:for(irr = 0; irr < K_WTS; irr++) {
					for(icc = 0; icc < K_WTS; icc++) {
						for(too = to, ioo = 0; too < too_max; too++, ioo++) {
							for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
					  BufW[ioo][iii][irr][icc] = ARRAYw_4(weights, too, tii, irr,
						icc, M_OFM(4), N_IFM(4), K_WTS, K_WTS);
					}
				  }
				}
			  }

				/* Write 0s into over-run regions at the end;
				 * This way convolve_kernel() accumulates correctly
				 * without needing a special case
				 */
				if (iii < TN_4) {
				  for(; iii < TN_4; iii++) {
					for(irr = 0; irr < K_WTS; irr++) {
					  for(icc = 0; icc < K_WTS; icc++) {
						BufW[ioo][iii][irr][icc] = 0;
					  }
					}
				  }
				}

			}

          //READ IN BUF_O FIRST since you're only partially calculating a cell in the output feature map, each iteration of the M loop
		  {
			// Indices internal to the block: count from 0
			index_t ioo, icc, irr;

			// Loop bounds
			index_t too_max, tcc_max, trr_max;
			too_max = MIN(to + TM_4, M_OFM(4));
			tcc_max = MIN(col + TC_4, C_OFM(4));
			trr_max = MIN(row + TR_4, R_OFM(4));

			BufO_read: for(trr = row, irr = 0; trr < trr_max; trr++, irr++) {
				for(tcc = col, icc = 0; tcc < tcc_max; tcc++, icc++) {
					for(too = to, ioo = 0; too < too_max; too++, ioo++) {
						if (ti == 0) {
							BufO[ioo][irr][icc] = 0;
						}
						else {
							BufO[ioo][irr][icc] =  ARRAYo_4(output, iter, too, trr, tcc, batch_size, M_OFM(4), R_OFM(4), C_OFM(4));
						}
					}
				}
			  }
		  }

		  // Call the blocked cnn kernel
		  cnn4_blocked_kernel(BufI, BufO, BufW);

          //WRITE TO BUF_O now that the computation kernel has run and the local copy of BufO has the partially calculated cell
		  {
			// Indices internal to the block: count from 0
			index_t ioo, icc, irr;

			// Loop bounds
			index_t too_max, tcc_max, trr_max;
			too_max = MIN(to + TM_4, M_OFM(4));
			tcc_max = MIN(col + TC_4, C_OFM(4));
			trr_max = MIN(row + TR_4, R_OFM(4));

			BufO_write: for(trr = row, irr = 0; trr < trr_max; trr++, irr++) {
				for(tcc = col, icc = 0; tcc < tcc_max; tcc++, icc++) {
					for(too = to, ioo = 0; too < too_max; too++, ioo++) {
				  ARRAYo_4(output, iter, too, trr, tcc, batch_size, M_OFM(4),
					R_OFM(4), C_OFM(4)) = BufO[ioo][irr][icc];
				  }
			    }
			  }
		    }


		  }
		}
	  }
	}
  }
}

#else //optimize for BufO

void krnl_cnn_layer4(const cnndata_t* input, const cnndata_t* weights,
        cnndata_t* output, uint64_t batch_size) {

  index_t iter;
  index_t row, col, to, ti;

  cnndata_t BufI[TN_4][TR_4*S_WTS+K_WTS-S_WTS][TC_4*S_WTS+K_WTS-S_WTS];
  cnndata_t BufO[TM_4][TR_4][TC_4];
  cnndata_t BufW[TM_4][TN_4][K_WTS][K_WTS];

#pragma HLS ARRAY_PARTITION variable=BufO type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=2 //factor=4
#pragma HLS ARRAY_PARTITION variable=BufI type=complete dim=1 //factor=4

  Batch: for(iter = 0; iter < batch_size; iter++) {        // Batch Loop
	R: for(row = 0; row < R_OFM(4); row += TR_4) {     // Tiled Row Loop
	  C: for(col = 0; col < C_OFM(4); col += TC_4) {   // Tiled Column Loop
		M: for(to = 0; to < M_OFM(4); to += TM_4) {    // Tiled Output Channel Loop
		  // Temporary versions of incremented indices;
		  // Same usage as in ZhangIsfpga_2()
		  index_t trr, tcc, too, tii;


		  // Only need to zero BufO in this loop ordering
		  {
			// Indices internal to the block: count from 0
			index_t ioo, icc, irr;

			BufO_zero: for(irr = 0; irr < TR_4; irr++) {
				for(icc = 0; icc < TC_4; icc++) {
					for(ioo = 0; ioo < TM_4; ioo++) {
#pragma HLS UNROLL
				  BufO[ioo][irr][icc] = 0;
				}
			  }
			}
		  }

		  // Tiled Input Channel Loop
		  N: for(ti = 0; ti < N_IFM(4); ti += TN_4) {

			// Load active input feature map into local buffer
			{
			  // Indices internal to the block: count from 0
			  index_t irr, icc, iii;

			  // Incremented temporary indices for input row and col
			  index_t xrr, xcc;

			  // Loop bounds
			  index_t tii_max, xrr_max, xcc_max;
			  tii_max = MIN(ti + TN_4, N_IFM(4));
			  xrr_max = MIN(row + TR_4, R_OFM(4)) * S_WTS + K_WTS - S_WTS;
			  xcc_max = MIN(col + TC_4, C_OFM(4)) * S_WTS + K_WTS - S_WTS;

			  BufI_load: for(xrr = row * S_WTS, irr = 0; xrr < xrr_max; xrr++, irr++) {
				  for(xcc = col * S_WTS, icc = 0; xcc < xcc_max; xcc++, icc++) {
					  for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
					BufI[iii][irr][icc] = ARRAYi_4(input, iter, tii, xrr, xcc,
					  batch_size, N_IFM(4), R_IFM(4), C_IFM(4));
				  }
				}
			  }
			}

			// Load active weights into local buffer
			{
			  // Indices internal to the block: count from 0
			  index_t ioo, iii, irr, icc;

			  // Loop bounds
			  index_t too_max, tii_max;
			  too_max = MIN(to + TM_4, M_OFM(4));
			  tii_max = MIN(ti + TN_4, N_IFM(4));

			  BufW_load:for(irr = 0; irr < K_WTS; irr++) {
					for(icc = 0; icc < K_WTS; icc++) {
						for(too = to, ioo = 0; too < too_max; too++, ioo++) {
							for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
					  BufW[ioo][iii][irr][icc] = ARRAYw_4(weights, too, tii, irr,
						icc, M_OFM(4), N_IFM(4), K_WTS, K_WTS);
					}
				  }
				}

				/* Write 0s into over-run regions at the end;
				 * This way convolve_kernel() accumulates correctly
				 * without needing a special case
				 */
				if (iii < TN_4) {
				  for(; iii < TN_4; iii++) {
					for(irr = 0; irr < K_WTS; irr++) {
					  for(icc = 0; icc < K_WTS; icc++) {
						BufW[ioo][iii][irr][icc] = 0;
					  }
					}
				  }
				}
			  }
			}

			// Call the blocked cnn kernel
			cnn4_blocked_kernel(BufI, BufO, BufW);
		  }

		  // Unload finished active intermedaite output feature map from local
		  // to full buffer
		  {
			// Indices internal to the block: count from 0
			index_t ioo, icc, irr;

			// Loop bounds
			index_t too_max, tcc_max, trr_max;
			too_max = MIN(to + TM_4, M_OFM(4));
			tcc_max = MIN(col + TC_4, C_OFM(4));
			trr_max = MIN(row + TR_4, R_OFM(4));

			BufO_write: for(trr = row, irr = 0; trr < trr_max; trr++, irr++) {
				for(tcc = col, icc = 0; tcc < tcc_max; tcc++, icc++) {
					for(too = to, ioo = 0; too < too_max; too++, ioo++) {
				  ARRAYo_4(output, iter, too, trr, tcc, batch_size, M_OFM(4),
					R_OFM(4), C_OFM(4)) = BufO[ioo][irr][icc];
				}
			  }
			}
		  }
		}
	  }
	}
  }
}

#endif



#ifdef __VITIS_CL__ // for lab 3
} // extern
#endif

void cnn4_blocked_kernel(cnndata_t BufI[TN_4][TR_4*S_WTS+K_WTS-S_WTS][TC_4*S_WTS+K_WTS-S_WTS],
                        cnndata_t BufO[TM_4][TR_4][TC_4],
                        cnndata_t BufW[TM_4][TN_4][K_WTS][K_WTS]) {
#pragma HLS INLINE
  index_t to_b, ti_b, row_b, col_b;

  index_t i, j;
  Krow: for(i = 0; i < K_WTS; i++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
    Kcol: for(j = 0; j < K_WTS; j++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
      Row: for(row_b = 0; row_b < TR_4; row_b++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
    	Col: for(col_b = 0; col_b < TC_4; col_b++) {
#pragma HLS pipeline
    	  To: for(to_b = 0; to_b < TM_4; to_b++) {
#pragma HLS UNROLL
    		Ti: for(ti_b = 0; ti_b < TN_4; ti_b++) {
#pragma HLS UNROLL
                BufO[to_b][row_b][col_b] += BufW[to_b][ti_b][i][j] *
                  BufI[ti_b][S_WTS*row_b+i][S_WTS*col_b+j];
            }
          }
        }
      }
    }
  }
}
