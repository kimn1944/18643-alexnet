import math
import sys

B = 50
R = 130 #64 # 130
C = 130 #64 # 130
M = 64 #16 # 64
N = 4 #64 # 4
K = 4
S = 2

TR = 0
TC = 0
TM = 0
TN = 0
BufW = 0
BufI = 0
BufO = 0

def W_size():
    return TM * TN * K * K

def I_size():
    return (TR*S + K - S)*(TC*S + K - S) * TN

def O_size():
    return TR * TC * TM

def set_buf_sizes():
    global BufW, BufI, BufO

    BufW = W_size()
    BufI = I_size()
    BufO = O_size()

def set_Ts(tr, tc, tm, tn):
    global TR, TC, TM, TN

    TR = tr
    TC = tc
    TM = tm
    TN = tn

    set_buf_sizes()

def get_W_total_accesses():
    BufW_acc = math.ceil(M * N / (TM * TN)) * BufW
    BufI_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufI
    BufO_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufO

    print("Weights loop ordering intermediaries")
    print("BufW accesses:" + f'{BufW_acc:,}')
    print("BufI accesses:" + f'{BufI_acc:,}')
    print("BufO accesses:" + f'{BufO_acc:,}')

    return BufW_acc + BufI_acc + BufO_acc * 2

def get_I_total_accesses():
    BufW_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufW
    BufI_acc = math.ceil(N * B * R * C / (TN * TR * TC)) * BufI
    BufO_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufO

    print("Input loop ordering intermediaries")
    print("BufW accesses:" + f'{BufW_acc:,}')
    print("BufI accesses:" + f'{BufI_acc:,}')
    print("BufO accesses:" + f'{BufO_acc:,}')

    return BufW_acc + BufI_acc + BufO_acc * 2

def get_O_total_accesses():
    BufW_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufW
    BufI_acc = math.ceil(M * N * B * R * C / (TM * TN * TR * TC)) * BufI
    BufO_acc = math.ceil(M * B * R * C / (TM * TR * TC)) * BufO

    print("Output loop ordering intermediaries")
    print("BufW accesses:" + f'{BufW_acc:,}')
    print("BufI accesses:" + f'{BufI_acc:,}')
    print("BufO accesses:" + f'{BufO_acc:,}')

    return BufW_acc + BufI_acc + BufO_acc

def get_ideal_accesses():
    W_acc = M * N * K * K
    I_acc = N * (R * S + K - S) * (C * S + K - S) * B
    O_acc = M * R * C * B

    return W_acc + I_acc + O_acc

def get_total_accesses(_type):
    if _type == "W":
        return get_W_total_accesses()
    elif _type == "I":
        return get_I_total_accesses()
    elif _type == "O":
        return get_O_total_accesses()
    elif _type == "Ideal":
        return get_ideal_accesses()
    else:
        assert False

def get_total_bytes(TR, TC, TM, TN):

    BufWNumCopies = math.ceil(M * N / (TM * TN))
    BufWBytesRead = TN * TM * K * K 
    BufWTotalRead = BufWNumCopies * BufWBytesRead
    
    #print("BufWNumCopies: " + str(BufWNumCopies))
    #print("BufWBytesRead: " + str(BufWBytesRead))
    #print("BufWTotalRead: " + str(BufWTotalRead))
    
    BufINumCopies = math.ceil(B * R * C * M * N / (TR * TC * TM * TN))
    BufIBytesRead = TN * (TR * S + K - S) * (TC * S + K - S)
    BufITotalRead = BufINumCopies * BufIBytesRead 
    
    #print("BufINumCopies: " + str(BufINumCopies))
    #print("BufIBytesRead: " + str(BufIBytesRead))
    #print("BufITotalRead: " + str(BufITotalRead))
    
    BufONumCopies = math.ceil(B * R * C * M * N / (TR * TC * TM * TN))
    BufOBytesWrite = TM * TR * TC
    BufOTotalRead = BufONumCopies * BufOBytesWrite 
    
    #print("BufONumCopies: " + str(BufONumCopies))
    #print("BufOBytesWrite: " + str(BufOBytesWrite))
    #print("BufOTotalRead: " + str(BufOTotalRead))
    
    TotalAccesses = BufOTotalRead + BufITotalRead + BufWTotalRead
    print(str(TR) + "," + str(TC) + "," +  str(TM) + "," +  str(TN) + "," + str(BufWNumCopies) + "," +  str(BufWBytesRead) + "," + str(BufINumCopies) + "," + str(BufIBytesRead) + "," + str(BufONumCopies) + "," + str(BufOBytesWrite) + "," + str(TotalAccesses))

"""
print("TR,TC,TM,TN,BufWNumCopies,BufWBytesRead,BufINumCopies,BufIBytesRead,BufONumCopies,BufOBytesWrite,TotalAccesses")

TR_array = range(1,13) #4, 8, 
TC_array = range(1,13) #4, 8, 
TM_array = range(1,17) #4, 8, 
TN_array = range(1,6) #4, 8, 

for TR in TR_array:
    for TC in TC_array:
        for TM in TM_array:
            for TN in TN_array:
                get_total_bytes(TR, TC, TM, TN)
"""

def _str(acc):
    return f'{acc:,}'

def csv():
    print("TR,TC,TM,TN,WOrd,IOrd,OOrd")

    TR_array = range(1,65) #4, 8, 
    TC_array = range(1,65) #4, 8, 
    TM_array = range(1,15) #4, 8, 
    TN_array = range(1,6) #4, 8, 

    minima = 100000000000
    T_mins = [0, 0, 0, 0]
    for TR in TR_array:
        for TC in TC_array:
            for TM in TM_array:
                for TN in TN_array:
                    set_Ts(TR, TC, TM, TN)
                    W_acc = get_total_accesses("W")
                    I_acc = get_total_accesses("I")
                    O_acc = get_total_accesses("O")


                    new_minima = min(W_acc, I_acc, O_acc)
                    if new_minima < minima:
                        minima = new_minima
                        T_mins = [TR, TC, TM, TN]

                    # print(str(TR) + "," + str(TC) + "," +  str(TM) + "," +  str(TN) + "," + _str(W_acc) + "," + _str(I_acc) + "," + _str(O_acc))

    print("Minima: " + str(T_mins[0]) + " " + str(T_mins[1]) + " " + str(T_mins[2]) + " " + str(T_mins[3]) + " " + _str(minima))


def main():
    if len(sys.argv) < 5:
        print("Usage: prog <TR> <TC> <TM> <TN>")
        return

    global TR, TC, TM, TN
    
    TR = int(sys.argv[1])
    TC = int(sys.argv[2])
    TM = int(sys.argv[3])
    TN = int(sys.argv[4])
    
    set_buf_sizes()

    total_W_accesses = get_total_accesses("W")
    total_I_accesses = get_total_accesses("I")
    total_O_accesses = get_total_accesses("O")
    ideal = get_total_accesses("Ideal")

    print("Optimizing for Weights:" + f'{total_W_accesses:,}')
    print("Optimizing for Inputs:" + f'{total_I_accesses:,}')
    print("Optimizing for Output:" + f'{total_O_accesses:,}')
    print("Ideal:" + f'{ideal:,}')


main()
# csv()
