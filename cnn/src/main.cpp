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
#include <sys/time.h>

using namespace std;

int main(int argc, char* argv[]) {
    struct timeval start_time, end_time;
    bool mismatch[5] = {false, false, false, false, false};

    cnndata_t *ptr_input, *ptr_weight[5], *ptr_output[5];
    cnndata_t *ref_input, *ref_weight[5], *ref_output[5];

    // Compute the size of array in bytes
    uint64_t num_elem_inputs = BATCH_SIZE * N_IFM(0) * R_IFM(0) * C_IFM(0);
    uint64_t num_elem_weights[5] = {
            M_OFM(0) * N_IFM(0) * K_WTS * K_WTS,
            M_OFM(1) * N_IFM(1) * K_WTS * K_WTS,
            M_OFM(2) * N_IFM(2) * K_WTS * K_WTS,
            M_OFM(3) * N_IFM(3) * K_WTS * K_WTS,
            M_OFM(4) * N_IFM(4) * K_WTS * K_WTS
    };
    uint64_t num_elem_outputs[5] = {
            BATCH_SIZE * M_OFM(0) * R_OFM(0) * C_OFM(0),
            BATCH_SIZE * M_OFM(1) * R_OFM(1) * C_OFM(1),
            BATCH_SIZE * M_OFM(2) * R_OFM(2) * C_OFM(2),
            BATCH_SIZE * M_OFM(3) * R_OFM(3) * C_OFM(3),
            BATCH_SIZE * M_OFM(4) * R_OFM(4) * C_OFM(4)
    };

#ifdef __VITIS_CL__
    // Hard coding xclbin filenames, ignoring command line arguments
    string xclbinFilename[5] = { // TODO mod when less kernels
        "binary_container_0.xclbin",
        "binary_container_1.xclbin",
        "binary_container_2.xclbin",
        "binary_container_3.xclbin",
        "binary_container_4.xclbin"
    };
#endif
    print_params(0);
    print_params(1);
    print_params(2);
    print_params(3);
    print_params(4);

    cl_object cl_obj;

    // TODO modify when we have less kernels
    krnl_object cnn_obj[5];
    cnn_obj[0].index = 0;
    cnn_obj[0].name = "krnl_cnn_layer0";

    cnn_obj[1].index = 1;
    cnn_obj[1].name = "krnl_cnn_layer1";

    cnn_obj[2].index = 2;
    cnn_obj[2].name = "krnl_cnn_layer2";

    cnn_obj[3].index = 3;
    cnn_obj[3].name = "krnl_cnn_layer3";

    cnn_obj[4].index = 4;
    cnn_obj[4].name = "krnl_cnn_layer4";


#ifdef __VITIS_CL__
    cout << "===== Initialize device ======" << endl;
    initialize_device(cl_obj);

    cout << "===== Reading xclbin ======" << endl;
    // Read cnn
    // TODO modify when we have less kernels
    read_xclbin(xclbinFilename[0], cl_obj.bins);
    read_xclbin(xclbinFilename[1], cl_obj.bins);
    read_xclbin(xclbinFilename[2], cl_obj.bins);
    read_xclbin(xclbinFilename[3], cl_obj.bins);
    read_xclbin(xclbinFilename[4], cl_obj.bins);

    cout << "\n===== Programming kernel ======" << endl;
    program_kernel(cl_obj, cnn_obj[0]);
#endif

    cout << "\n===== Allocating buffers ======" << endl;
    uint64_t bufIdx=0;
    allocate_readonly_mem(cl_obj, (void**) &ptr_input, bufIdx++,
                          num_elem_inputs * sizeof(cnndata_t));
    for (int i = 0; i < 5; i++) {
        allocate_readonly_mem(cl_obj, (void**) &ptr_weight[i], bufIdx++,
                              num_elem_weights[i] * sizeof(cnndata_t));
        allocate_readwrite_mem(cl_obj, (void**) &ptr_output[i], bufIdx++,
                              num_elem_outputs[i] * sizeof(cnndata_t));       
    }

    MALLOC_CHECK(ref_input = new cnndata_t[num_elem_inputs]);
    for (int i = 0; i < 5; i++) {
        MALLOC_CHECK(ref_weight[i] = new cnndata_t[num_elem_weights[i]]);
        MALLOC_CHECK(ref_output[i] = new cnndata_t[num_elem_outputs[i]]);
    }

    // Set randomized inputs in reference copy
    initialize_buffer(ref_input, num_elem_inputs, true);
    for (int i = 0; i < 5; i++)
        initialize_buffer(ref_weight[i], num_elem_weights[i], true);

    // copy ref copy to kernel use copy, converting to kernel expected layout
    // TODO modify when we have less kernels, only mod ARRAYw_X variable
    COPY_BUF4D(ref_input, ARRAY4, ptr_input, ARRAYi_0,
               BATCH_SIZE, N_IFM(0), R_IFM(0),  C_IFM(0));
    COPY_BUF4D(ref_weight[0], ARRAY4, ptr_weight[0], ARRAYw_0,
               M_OFM(0), N_IFM(0), K_WTS, K_WTS);
    COPY_BUF4D(ref_weight[1], ARRAY4, ptr_weight[1], ARRAYw_1,
               M_OFM(1), N_IFM(1), K_WTS, K_WTS);
    COPY_BUF4D(ref_weight[2], ARRAY4, ptr_weight[2], ARRAYw_2,
               M_OFM(2), N_IFM(2), K_WTS, K_WTS);
    COPY_BUF4D(ref_weight[3], ARRAY4, ptr_weight[3], ARRAYw_3,
               M_OFM(3), N_IFM(3), K_WTS, K_WTS);
    COPY_BUF4D(ref_weight[4], ARRAY4, ptr_weight[4], ARRAYw_4,
               M_OFM(4), N_IFM(4), K_WTS, K_WTS);

    // Random initialize output for kernel use
    for (int i = 0; i < 5; i++)
        initialize_buffer(ptr_output[i], num_elem_outputs[i], true); // cannot assume 0'ed

    cout << "\n===== Execution and Timing starts ======" << endl;
    gettimeofday(&start_time, NULL);

#ifdef __VITIS_CL__
    cnn_run_kernel(cl_obj, cnn_obj);
#else
    // TODO change this when we have less kernels
    krnl_cnn_layer0(ptr_input, ptr_weight[0], ptr_output[0], BATCH_SIZE);
    krnl_cnn_layer1(ptr_output[0], ptr_weight[1], ptr_output[1], BATCH_SIZE);
    krnl_cnn_layer2(ptr_output[1], ptr_weight[2], ptr_output[2], BATCH_SIZE);
    krnl_cnn_layer3(ptr_output[2], ptr_weight[3], ptr_output[3], BATCH_SIZE);
    krnl_cnn_layer4(ptr_output[3], ptr_weight[4], ptr_output[4], BATCH_SIZE);
#endif

    gettimeofday(&end_time, NULL);
    cout << "Execution and Timing finished!\n" << endl;

    cout << "===== Verification starts ======" << endl;
    mismatch[0] = cnn_check(ptr_input, ptr_weight[0], ptr_output[0], ref_input,
                            ref_weight[0], ref_output[0], 0);
    cout << "CNN layer 0 TEST " << (mismatch[0] ? "FAILED" : "PASSED")
         << "\n" << endl;
    for (int i = 1; i < 5; i++) {
        mismatch[i] = cnn_check(ptr_output[i-1], ptr_weight[i], ptr_output[i],
                                ref_output[i-1], ref_weight[i], ref_output[i], i);
        cout << "CNN layer " << i << " TEST " << (mismatch[i] ? "FAILED" : "PASSED")
             << "\n" << endl;
    }

    bufIdx = 0;
    delete[] ref_input;
    deallocate_mem(cl_obj, ptr_input, bufIdx++);
    for (int i = 0; i < 5; i++) {
        delete[] ref_weight[i];
        delete[] ref_output[i];
        deallocate_mem(cl_obj, ptr_weight[i], bufIdx++);
        deallocate_mem(cl_obj, ptr_output[i], bufIdx++);
    }

    cout << "===== Reporting measured throughput ======" << endl;
    float timeusec = (end_time.tv_sec - start_time.tv_sec)*1e6
                     + (end_time.tv_usec - start_time.tv_usec);
    printf("Runtime = %0.1f (microsec) \n\n", timeusec);
    double num_ops[5] = {
        BATCH_SIZE * (double)2.0 * M_OFM(0) * R_OFM(0) * C_OFM(0) * N_IFM(0) * K_WTS * K_WTS,
        BATCH_SIZE * (double)2.0 * M_OFM(1) * R_OFM(1) * C_OFM(1) * N_IFM(1) * K_WTS * K_WTS,
        BATCH_SIZE * (double)2.0 * M_OFM(2) * R_OFM(2) * C_OFM(2) * N_IFM(2) * K_WTS * K_WTS,
        BATCH_SIZE * (double)2.0 * M_OFM(3) * R_OFM(3) * C_OFM(3) * N_IFM(3) * K_WTS * K_WTS,
        BATCH_SIZE * (double)2.0 * M_OFM(4) * R_OFM(4) * C_OFM(4) * N_IFM(4) * K_WTS * K_WTS
    };
    printf("# of operations = %.0f + %.0f + %.0f + %.0f + %.0f\n", num_ops[0], num_ops[1],
           num_ops[2], num_ops[3], num_ops[4]);
    printf("Throughput: %.5f GigaOP/sec\n",
           (double)1.0e-3 * (num_ops[0] + num_ops[1] + num_ops[2] + num_ops[3] + num_ops[4]) / timeusec);

    cout << "\n===== Exiting ======" << endl;

    return (mismatch[0] || mismatch[1] || mismatch[2] || mismatch[3] || mismatch[4]);
}




