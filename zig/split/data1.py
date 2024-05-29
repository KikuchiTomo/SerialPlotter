import sys
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from datetime import datetime as dt
from scipy import signal
import os

def butterLowpass(lowcut, fs, order=4):

    nyq = 0.5 * fs
    low = lowcut / nyq
    b, a = signal.butter(order, low, btype='low')
    return b, a


def lowpass(x, lowcut, fs, order=4):
    b, a = butterLowpass(lowcut, fs, order=order)
    y = signal.filtfilt(b, a, x)
    return y

def calcSamplingRate(a):
    x = a[0]
    n = 0.0
    for b in a[1:]:
        n += b - x
        x = b
    d = n / len(a) 
    return 1.0 / d

def appendPath(f0, f1):
    if f0[-1] == '/':
        return f0 + f1
    else:
        return f0 + '/' + f1

def isHeadChFirst(df):
    n = df['ch'][0]
    return n == 0
    
def dropHead(df):
    tdf = df
    while True:
        if isHeadChFirst(tdf):
            break
        tdf = tdf.drop(index=tdf.index[[0]]).reset_index(drop=True)        
    return tdf

def separeteByCh(df, use_chs=[0,1,2,3,4,5,6,7,8,9,10,11,12,13]):
    dfs = []
    for ch in use_chs:
        tdf = df[df['ch'] == ch].reset_index(drop=True).rename(columns={'value': ch}).drop('ch', axis=1)
        dfs.append(tdf)        
    return dfs

def mergeWithTime(dfs, name):
    tdatetime = dt.strptime(name.split('/')[-1].split('_')[-1].split('.')[0], '%Y-%m-%d-%H-%M-%S')
    n = pd.concat(dfs, axis=1).dropna(how='any')
    t = n['time'].iloc[:, :].mean(axis=1)
    n['time'] = t + tdatetime.timestamp() - t.iloc[0]
    n = n.loc[:,~n.columns.duplicated()]
    return n
    
def load(fpath):
    #### 読み返す気も失せる可読性ゼロの関数呼び出してる邪道すぎるワンライナー
    ####  * 与えられたファイルパスでlsして，フルパスにしてからpandasで読み込み, 先頭行のchが0になるまで削除し, チャンネルごとに分割して, indexを振り直し, indexで結合し, NaNがある行を削除し, 各行の各チャンネルの測定時刻で平均をとってそれを時間列に代入，重複した時間列を削除して，時間列に測定開始時間のunixタイムをたす
    dfs = list(map(lambda x: mergeWithTime(separeteByCh(dropHead(pd.read_csv(x, names=['time', 'ch', 'value']))), x), [fpath for fpath in list(map(lambda p: appendPath(fpath, p), os.listdir(fpath))) if (('.csv' in fpath) or ('.CSV' in fpath)) ]))    
    return dfs

