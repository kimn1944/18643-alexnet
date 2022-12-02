#!/bin/bash

cd cnn_system/Hardware/package/sd_card

tar -czvf $1.tgz binary_container_0.xclbin binary_container_1.xclbin binary_container_2.xclbin binary_container_3.xclbin binary_container_4.xclbin cnn

scp -i ~/.ssh/id_ed25519 $1.tgz root@172.26.173.89:/home/root/alexnet-binaries

cd ../../../..

ssh -i ~/.ssh/id_ed25519 root@172.26.173.89 "./run_binary.sh $1"

#cp alexnet-binaries/$1.tgz alexnet-run

#cd alexnet-run

#tar -xvzf $1.tgz

#rm $1.tgz

#./cnn binary_container_0.xclbin binary_container_1.xclbin binary_container_2.xclbin binary_container_3.xclbin binary_container_4.xclbin > ../alexnet-outputs/$1.out

#cat ../alexnet-outputs/$1.tgz

#exit


