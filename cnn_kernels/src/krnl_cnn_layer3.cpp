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

#include "krnl_cnn_layer3.h"

// Prevent aliasing
#undef BATCH_SIZE

#include "util643.h"

void cnn3_blocked_kernel(cnndata_t BufI[TN_3][TR_3*S_WTS+K_WTS-S_WTS][TC_3*S_WTS+K_WTS-S_WTS],
                        cnndata_t BufO[TM_3][TR_3][TC_3],
                        cnndata_t BufW[TM_3][TN_3][K_WTS][K_WTS]);

#ifdef __VITIS_CL__
extern "C" {
#endif
void krnl_cnn_layer3(const cnndata_t* input, const cnndata_t* weights,
        cnndata_t* output, uint64_t batch_size) {

  index_t iter;
  index_t row, col, to, ti;

  cnndata_t BufI[TN_3][TR_3*S_WTS+K_WTS-S_WTS][TC_3*S_WTS+K_WTS-S_WTS];
  cnndata_t BufO[TM_3][TR_3][TC_3];
  cnndata_t BufW[TM_3][TN_3][K_WTS][K_WTS];

#pragma HLS ARRAY_PARTITION variable=BufO type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=1 //factor=16
#pragma HLS ARRAY_PARTITION variable=BufW type=complete dim=2 //factor=4
#pragma HLS ARRAY_PARTITION variable=BufI type=complete dim=1 //factor=4

  Batch: for(iter = 0; iter < batch_size; iter++) {        // Batch Loop
    R: for(row = 0; row < R_OFM(3); row += TR_3) {     // Tiled Row Loop
      C: for(col = 0; col < C_OFM(3); col += TC_3) {   // Tiled Column Loop
        M: for(to = 0; to < M_OFM(3); to += TM_3) {    // Tiled Output Channel Loop
          // Temporary versions of incremented indices;
          // Same usage as in ZhangIsfpga_3()
          index_t trr, tcc, too, tii;


          // Only need to zero BufO in this loop ordering
          {
            // Indices internal to the block: count from 0
            index_t ioo, icc, irr;

            BufO_zero: for(irr = 0; irr < TR_3; irr++) {
                for(icc = 0; icc < TC_3; icc++) {
                	for(ioo = 0; ioo < TM_3; ioo++) {
#pragma HLS UNROLL
                  BufO[ioo][irr][icc] = 0;
                }
              }
            }
          }

          // Tiled Input Channel Loop
          N: for(ti = 0; ti < N_IFM(3); ti += TN_3) {

            // Load active input feature map into local buffer
            {
              // Indices internal to the block: count from 0
              index_t irr, icc, iii;

              // Incremented temporary indices for input row and col
              index_t xrr, xcc;

              // Loop bounds
              index_t tii_max, xrr_max, xcc_max;
              tii_max = MIN(ti + TN_3, N_IFM(3));
              xrr_max = MIN(row + TR_3, R_OFM(3)) * S_WTS + K_WTS - S_WTS;
              xcc_max = MIN(col + TC_3, C_OFM(3)) * S_WTS + K_WTS - S_WTS;

              BufI_load: for(xrr = row * S_WTS, irr = 0; xrr < xrr_max; xrr++, irr++) {
                  for(xcc = col * S_WTS, icc = 0; xcc < xcc_max; xcc++, icc++) {
                	  for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
                    BufI[iii][irr][icc] = ARRAYi_3(input, iter, tii, xrr, xcc,
                      batch_size, N_IFM(3), R_IFM(3), C_IFM(3));
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
              too_max = MIN(to + TM_3, M_OFM(3));
              tii_max = MIN(ti + TN_3, N_IFM(3));

              BufW_load:for(irr = 0; irr < K_WTS; irr++) {
                    for(icc = 0; icc < K_WTS; icc++) {
                    	for(too = to, ioo = 0; too < too_max; too++, ioo++) {
                    		for(tii = ti, iii = 0; tii < tii_max; tii++, iii++) {
                      BufW[ioo][iii][irr][icc] = ARRAYw_3(weights, too, tii, irr,
                        icc, M_OFM(3), N_IFM(3), K_WTS, K_WTS);
                    }
                  }
                }

                /* Write 0s into over-run regions at the end;
                 * This way convolve_kernel() accumulates correctly
                 * without needing a special case
                 */
                if (iii < TN_3) {
                  for(; iii < TN_3; iii++) {
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
            cnn3_blocked_kernel(BufI, BufO, BufW);
          }

          // Unload finished active intermedaite output feature map from local
          // to full buffer
          {
            // Indices internal to the block: count from 0
            index_t ioo, icc, irr;

            // Loop bounds
            index_t too_max, tcc_max, trr_max;
            too_max = MIN(to + TM_3, M_OFM(3));
            tcc_max = MIN(col + TC_3, C_OFM(3));
            trr_max = MIN(row + TR_3, R_OFM(3));

            BufO_write: for(trr = row, irr = 0; trr < trr_max; trr++, irr++) {
                for(tcc = col, icc = 0; tcc < tcc_max; tcc++, icc++) {
                	for(too = to, ioo = 0; too < too_max; too++, ioo++) {
                  ARRAYo_3(output, iter, too, trr, tcc, batch_size, M_OFM(3),
                    R_OFM(3), C_OFM(3)) = BufO[ioo][irr][icc];
                }
              }
            }
          }
        }
      }
    }
  }
}

#ifdef __VITIS_CL__ // for lab 3
} // extern
#endif

void cnn3_blocked_kernel(cnndata_t BufI[TN_3][TR_3*S_WTS+K_WTS-S_WTS][TC_3*S_WTS+K_WTS-S_WTS],
                        cnndata_t BufO[TM_3][TR_3][TC_3],
                        cnndata_t BufW[TM_3][TN_3][K_WTS][K_WTS]) {
#pragma HLS INLINE
  index_t to_b, ti_b, row_b, col_b;

  index_t i, j;
  Krow: for(i = 0; i < K_WTS; i++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
    Kcol: for(j = 0; j < K_WTS; j++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
      Row: for(row_b = 0; row_b < TR_3; row_b++) {
#pragma HLS pipeline off
//#pragma HLS loop_flatten off
    	Col: for(col_b = 0; col_b < TC_3; col_b++) {
#pragma HLS pipeline
    	  To: for(to_b = 0; to_b < TM_3; to_b++) {
#pragma HLS UNROLL
    		Ti: for(ti_b = 0; ti_b < TN_3; ti_b++) {
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
