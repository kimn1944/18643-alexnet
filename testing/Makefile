
INCLUDE_KERNEL=/scratch/643_vitis_sbernal/18643-alexnet/cnn_kernels/src
INCLUDE_CNN=/scratch/643_vitis_sbernal/18643-alexnet/cnn/src

SRC := $(wildcard $(INCLUDE_KERNEL)/*.cpp) $(wildcard $(INCLUDE_CNN)/*.cpp)
CFLAGS=-I$(INCLUDE_KERNEL) -I$(INCLUDE_CNN)

csim: $(SRC) $(INCLUDE_CNN)/util643.h
	g++ $(CFLAGS) -o csim $(SRC)

clean:
	rm main
