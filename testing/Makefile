
INCLUDE_KERNEL=/home/fpga/workspace/alexnet/cnn_kernels/src
INCLUDE_CNN=/home/fpga/workspace/alexnet/cnn/src

SRC := $(wildcard $(INCLUDE_KERNEL)/*.cpp) $(wildcard $(INCLUDE_CNN)/*.cpp)
CFLAGS=-I$(INCLUDE_KERNEL) -I$(INCLUDE_CNN)

csim: $(SRC) $(INCLUDE_CNN)/util643.h
	g++ $(CFLAGS) -o csim $(SRC)

clean:
	rm main
