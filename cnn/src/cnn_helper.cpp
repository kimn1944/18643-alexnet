/****************************************************************
 * Copyright (c) 2017~2022, 18-643 Course Staff, CMU
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

#include "lab3_kernels.h"
#include "cnn_helper.h"
#include <string.h>
#include <assert.h>

// Set kernel arguments and execute it
#ifdef __VITIS_CL__
void static execute_kernel(cl_object &cl_obj, int layer, bool reuse) {
    cl_int err;

    std::cout << "Running kernel for layer " << layer << "..." << std::endl;

    // Get i/o buffers from kernel object
    cl::Buffer *buffer_in = &cl_obj.buffers[layer * 2];
    cl::Buffer *buffer_wts = &cl_obj.buffers[layer * 2 + 1];
    cl::Buffer *buffer_out = &cl_obj.buffers[layer * 2 + 2];

    // Set the kernel Arguments
    uint64_t narg = 0;
    OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, *buffer_in));            // input
    OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, *buffer_wts));           // weights
    OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, *buffer_out));           // output
    OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, (uint64_t) BATCH_SIZE)); // batch size

    if (reuse) {
        OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, (uint64_t) R_OFM(layer)));
        OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, (uint64_t) C_OFM(layer)));
        OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, (uint64_t) M_OFM(layer)));
        OCL_CHECK(err, err = cl_obj.krnl->setArg(narg++, (uint64_t) N_IFM(layer)));
    }

    // Data will be migrated to kernel space
    OCL_CHECK(err, err = cl_obj.q.enqueueMigrateMemObjects({*buffer_in, *buffer_wts}, 0/* 0 means from host*/));

    // Launch the Kernel; this is nonblocking.
    OCL_CHECK(err, err = cl_obj.q.enqueueTask(*cl_obj.krnl));

    // The result of the previous kernel execution will need to be retrieved in
    // order to view the results. This call will transfer the data from FPGA to
    // source_results vector
    OCL_CHECK(err, cl_obj.q.enqueueMigrateMemObjects({*buffer_out}, CL_MIGRATE_MEM_OBJECT_HOST));

    // Wait for all tasks to finish
    OCL_CHECK(err, cl_obj.q.finish());
}

// TODO change this when we have less kernels, remove program_kernel()
void cnn_run_kernel(cl_object &cl_obj, krnl_object *krnl_obj, times_t *times) {

    // Layer 0
    gettimeofday(&times->layer0_start, NULL);
    execute_kernel(cl_obj, 0, false);
    gettimeofday(&times->layer0_end, NULL);

    // Layer 1
    gettimeofday(&times->layer01_recon_start, NULL);
    program_kernel(cl_obj, krnl_obj[1]);
    gettimeofday(&times->layer01_recon_end, NULL);
    gettimeofday(&times->layer1_start, NULL);
    execute_kernel(cl_obj, 1, true);
    gettimeofday(&times->layer1_end, NULL);

    // Layer 2
    gettimeofday(&times->layer12_recon_start, NULL);
//    program_kernel(cl_obj, krnl_obj[2]);
    gettimeofday(&times->layer12_recon_end, NULL);
    gettimeofday(&times->layer2_start, NULL);
    execute_kernel(cl_obj, 2, true);
    gettimeofday(&times->layer2_end, NULL);

    // Layer 3
    gettimeofday(&times->layer23_recon_start, NULL);
    program_kernel(cl_obj, krnl_obj[3]);
    gettimeofday(&times->layer23_recon_end, NULL);
    gettimeofday(&times->layer3_start, NULL);
    execute_kernel(cl_obj, 3, false);
    gettimeofday(&times->layer3_end, NULL);

    // Layer 4
    gettimeofday(&times->layer34_recon_start, NULL);
    program_kernel(cl_obj, krnl_obj[4]);
    gettimeofday(&times->layer34_recon_end, NULL);
    gettimeofday(&times->layer4_start, NULL);
    execute_kernel(cl_obj, 4, false);
    gettimeofday(&times->layer4_end, NULL);

    std::cout << "Kernel executions completed" << std::endl;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// DO NOT MODIFY BELOW THIS LINE ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static const std::string cnn_error_message =
        "Error: Result mismatch:\n"
        "i = %d CPU result = %d Device result = %d\n";

// Verify a single batch of data
bool verify(cnndata_t *ref, cnndata_t *checkit, uint64_t iter, uint64_t layer) {
    uint64_t row, col, to;

    for(to = 0; to < M_OFM(layer); to++) {
        for(row = 0; row < R_OFM(layer); row++) {
            for(col = 0; col < C_OFM(layer) ; col++) {
                cnndata_t refval = ARRAY4(ref, iter, to, row, col,
                        BATCH_SIZE, M_OFM(layer), R_OFM(layer), C_OFM(layer));

                cnndata_t checkval;
                // TODO modify when changing kernel count
                switch (layer) {
                case (0):
                    checkval = ARRAYo_0(checkit, iter, to, row, col, BATCH_SIZE,
                             M_OFM(layer), R_OFM(layer), C_OFM(layer));
                    break;
                case (1):
                    checkval = ARRAYo_1(checkit, iter, to, row, col, BATCH_SIZE,
                             M_OFM(layer), R_OFM(layer), C_OFM(layer));
                    break;
                case (2):
                    checkval = ARRAYo_2(checkit, iter, to, row, col, BATCH_SIZE,
                             M_OFM(layer), R_OFM(layer), C_OFM(layer));
                    break;
                case (3):
                    checkval = ARRAYo_3(checkit, iter, to, row, col, BATCH_SIZE,
                             M_OFM(layer), R_OFM(layer), C_OFM(layer));
                    break;
                case (4):
                    checkval = ARRAYo_4(checkit, iter, to, row, col, BATCH_SIZE,
                             M_OFM(layer), R_OFM(layer), C_OFM(layer));
                    break;
                default:
                    assert(false);
                }

                if (!nearlyEqual(checkval, refval)) {
                    printf("\n***Result does not match reference: layer = %lu, "
                            "row = %lu, col = %lu***\n", to, row, col);
                    return 0;
                }
            }
        }
    }
    return 1;
}

bool cnn_check(cnndata_t *ptr_input, cnndata_t *ptr_weight, cnndata_t *ptr_output,
        cnndata_t *ref_input, cnndata_t *ref_weight, cnndata_t *ref_output,
        uint64_t layer) {
    std::cout << "Verifying cnn result..." << std::endl;

    //Verify the result
    uint64_t mismatch = 0;
    uint64_t iter;

    for(iter = 0; iter < BATCH_SIZE; iter++) {
        ZhangIsfpga15_1_fp(ref_input, ref_output, ref_weight, iter, layer);
        if (!verify(ref_output, ptr_output, iter, layer)) {
            mismatch = 1;
            break;
        }
    }
    return mismatch;
}

void print_params(uint64_t layer) {

    std::cout << "===== Printing the CNN parameters Layer "
              << layer << " ======" << std::endl;

    std::cout << "Batch size: " << (uint64_t) BATCH_SIZE << std::endl;

    printf("Layer Parameters: \nK_wts: \t%d\tS_wts:\t%d\nR_ofm:\t%d\tC_ofm:"
           "\t%d\tM_ofm:\t%d\tN_ifm:\t%d\tR_ifm:\t%d\tC_ifm:\t%d\n", K_WTS, S_WTS, R_OFM(layer),
           C_OFM(layer), M_OFM(layer), N_IFM(layer), R_IFM(layer), C_IFM(layer));

    // TODO modify when we have less kernels
    switch (layer) {
    case 0:
        printf("Kernel Parameters: \nTr: \t%d\tTc:\t%d\tTm:\t%d\tTn:\t%d\n\n",
                TR_0, TC_0, TM_0, TN_0);
        break;
    case 1:
        printf("Kernel Parameters: \nTr: \t%d\tTc:\t%d\tTm:\t%d\tTn:\t%d\n\n",
                TR_1, TC_1, TM_1, TN_1);
        break;
    case 2:
        printf("Kernel Parameters: \nTr: \t%d\tTc:\t%d\tTm:\t%d\tTn:\t%d\n\n",
                TR_2, TC_2, TM_2, TN_2);
        break;
    case 3:
        printf("Kernel Parameters: \nTr: \t%d\tTc:\t%d\tTm:\t%d\tTn:\t%d\n\n",
                TR_3, TC_3, TM_3, TN_3);
        break;
    case 4:
        printf("Kernel Parameters: \nTr: \t%d\tTc:\t%d\tTm:\t%d\tTn:\t%d\n\n",
                TR_4, TC_4, TM_4, TN_4);
        break;
    default:
        assert(false);
    }
}

void initialize_buffer(cnndata_t *ptr, unsigned size, bool notzero) {
    for (unsigned i = 0; i < size; i++) {
        ptr[i] = notzero ? (rand() % VRANGE) : 0;
    }
}

void ZhangIsfpga15_1_fp(cnndata_t *input, cnndata_t *output, cnndata_t *weights,
        uint64_t iter, uint64_t layer) {
    uint64_t row, col, to, ti;

    for(row = 0; row < R_OFM(layer); row++) {
        for(col = 0; col < C_OFM(layer); col++) {
            for(to = 0; to < M_OFM(layer); to++) {
                ARRAY4(output, iter, to, row, col, BATCH_SIZE, M_OFM(layer),
                        R_OFM(layer), C_OFM(layer)) = 0;
            }
        }
    }

    for(row = 0; row < R_OFM(layer); row++) {
      for(col = 0; col < C_OFM(layer); col++) {
        for(to = 0; to < M_OFM(layer); to++) {
          for(ti = 0; ti < N_IFM(layer); ti++) {
            uint64_t i, j;
            for(i = 0; i < K_WTS; i++) {
              for(j = 0; j < K_WTS; j++) {
                ARRAY4(output, iter, to, row, col, BATCH_SIZE, M_OFM(layer),
                    R_OFM(layer), C_OFM(layer))
                    += ARRAY4(weights, to, ti, i, j, M_OFM(layer), N_IFM(layer),
                    K_WTS, K_WTS)
                    * ARRAY4(input, iter, ti, S_WTS * row + i, S_WTS * col + j,
                       BATCH_SIZE, N_IFM(layer), R_IFM(layer), C_IFM(layer));
              }
            }
          }
        }
      }
    }
}
