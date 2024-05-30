import matplotlib.pyplot as plt
import numpy as np
import sys
import os
import pickle as pk

index = 4

# 二つ別々にプロット
# with open(sys.argv[1], "rb") as f:
#     d = pk.load(f)[index]

#     fig = plt.figure()
#     a0 = fig.add_subplot(2, 1, 1)
#     t0 = d[0][0] - d[0][0][0]
#     a0.plot(t0, d[0][1], label="z")
#     a0.plot(t0, d[0][2], label="x")
#     a0.plot(t0, d[0][3], label="y")
#     a0.set_ylim([-0.7, 0.7])
#     a0.legend()
#     plt.grid()
    
#     a1 = fig.add_subplot(2, 1, 2)
#     t1 = d[1]['time'].to_numpy()
#     t1 = t1 - t1[0]

#     d1 = d[1].iloc[:, 1:].to_numpy().T
#     for i in range(int(d1.shape[0])):
#         if i != 4:
#             a1.plot(t1, d1[i], label=f'{i}')
#     #a1.set_ylim([0.0, 1.0])
#     a1.legend()
#     plt.grid()

#     plt.legend()
#     plt.show()

def norm(d):
    return (d - np.min(d)) / (np.max(d) - np.min(d))

# 一緒にプロット
with open(sys.argv[1], "rb") as f:
    d = pk.load(f)[index]

    fig = plt.figure(figsize=(15,10))
    a0 = fig.add_subplot(1, 1, 1)
    t0 = d[0][0] - d[0][0][0]
    a0.plot(t0, d[0][1], label="z")
    a0.plot(t0, d[0][2], label="x")
    a0.plot(t0, d[0][3], label="y")
    #a0.set_ylim([-0.7, 0.7])
    #a0.legend()
    #plt.grid()
    
    t1 = d[1]['time'].to_numpy()
    t1 = t1 - t1[0]

    d1 = np.delete(d[1].iloc[:, 1:].to_numpy().T, 4, 0) 
    d1 = norm(d1)
    for i in range(int(d1.shape[0])):
        a0.plot(t1, d1[i], label=f'{i}')
    #a1.set_ylim([0.0, 1.0])
    a0.legend()
    a0.set_xlabel("time [sec]")
    a0.set_ylabel("amplitude")
    plt.grid()

    plt.legend()
    plt.show()
