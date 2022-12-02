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
 * The parameters in this file sets the problem sizes
 *
 * You cannot change this file in your final submission
 */
#pragma once

typedef int cnndata_t;

static inline bool nearlyEqual(cnndata_t a, cnndata_t b) { return a == b; }

#define K_WTS (4) // weight width and height (square)
                   // same depth as output
#define S_WTS (2) // sliding stride

#if 1 // fullsize

/*********************************************************************************************************
                                                 FULLSIZE
*********************************************************************************************************/

#define BATCH_SIZE 1

#define layer0_BufI_optimization
#define layer1_BufI_optimization
#define layer2_BufI_optimization
#define layer3_BufI_optimization
#define layer4_BufI_optimization

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

#else // SW emulation debug small

/*********************************************************************************************************
                                                 SMALL
*********************************************************************************************************/

#define BATCH_SIZE 3

// FIXME
//
// Layer 4
//
#define r_4 (4)                                 // height
#define c_4 (4)                                 // width
#define m_4 (16)                           // depth
#define n_4 (32)                              // depth
#define r_i4 (r_4*S_WTS+K_WTS-S_WTS)       // derived height
#define c_i4 (c_4*S_WTS+K_WTS-S_WTS)       // derived width

//
// Layer 3
//
#define r_3 (10)                             // height
#define c_3 (10)                             // width
#define m_3 (32)                             // depth
#define n_3 (32)                             // depth
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

#endif

#define R_OFM(layer) ((layer == 0) ? r_0 : ((layer == 1) ? r_1 : ((layer == 2) ? r_2 : ((layer == 3) ? r_3 : r_4))))
#define C_OFM(layer) ((layer == 0) ? c_0 : ((layer == 1) ? c_1 : ((layer == 2) ? c_2 : ((layer == 3) ? c_3 : c_4))))
#define M_OFM(layer) ((layer == 0) ? m_0 : ((layer == 1) ? m_1 : ((layer == 2) ? m_2 : ((layer == 3) ? m_3 : m_4))))
#define N_IFM(layer) ((layer == 0) ? n_0 : ((layer == 1) ? n_1 : ((layer == 2) ? n_2 : ((layer == 3) ? n_3 : n_4))))
#define R_IFM(layer) ((layer == 0) ? r_i0 : ((layer == 1) ? r_i1 : ((layer == 2) ? r_i2 : ((layer == 3) ? r_i3 : r_i4))))
#define C_IFM(layer) ((layer == 0) ? c_i0 : ((layer == 1) ? c_i1 : ((layer == 2) ? c_i2 : ((layer == 3) ? c_i3 : c_i4))))
