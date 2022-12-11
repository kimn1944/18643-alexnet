#!/bin/bash

cd /scratch/643_vitis_sbernal/18643-alexnet/cnn_system/Hardware/package/sd_card

tar -czvf $1.tgz binary_container_0.xclbin binary_container_1.xclbin binary_container_2.xclbin binary_container_3.xclbin binary_container_4.xclbin cnn

mv $1.tgz /scratch/643_vitis_sbernal/18643-alexnet/alexnet-binaries

