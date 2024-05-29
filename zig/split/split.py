import sys
import numpy as np
import data0
import data1
import pickle as pk

def firstIndex(timeline, timestamp):
    for i in range(len(timeline)-1):
        s = timeline[i]
        e = timeline[i+1]
        # print(s, e, timestamp, s < timestamp, timestamp < e)
        if s <= timestamp and timestamp < e:
            return i    
    return -1

def main():
    argc = len(sys.argv)
    print(argc)
    
    if argc < 4:
        print("引数: {指先圧力CSVのパス} {腕周りFMGのディレクトリのパス} {pickleの書き出しパス}")
        return

    ds0 = data0.load(sys.argv[1])
    ds1 = data1.load(sys.argv[2])

    ds2 = []

    for d1 in ds1:
        s = d1['time'].iloc[0]
        e = d1['time'].iloc[-1]
        print(f'Start Time  [sec]: {s}')
        print(f'End   Time  [sec]: {e}')
        si = firstIndex(ds0[0], s)
        ei = firstIndex(ds0[0], e)
        print(f"Start Index [idx]: {si}")
        print(f"End   Index [idx]: {ei}")

        d0 = np.zeros((4, ei-si))
        d0[0] = ds0[0][si:ei]
        d0[1] = ds0[1][si:ei]
        d0[2] = ds0[3][si:ei]
        d0[3] = ds0[5][si:ei]
    
        ds2.append([
            d0,
            d1
        ])
        print('---------------------------------------')

    with open(sys.argv[3], 'wb') as f:
        pk.dump(ds2, f)

    print("書き出しOK")

if __name__ == "__main__":
    main()
