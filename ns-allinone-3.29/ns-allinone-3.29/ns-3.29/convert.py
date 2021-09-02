import pandas as pd
import numpy as np

data = np.loadtxt('log.dat', delimiter='\t', dtype=np.float)
data_new = data[:,1:]
data_idx = data[:,0]
data_df = pd.DataFrame(data, index=data_idx, columns=["Label", "Time", "Packet"])

flow1 = data_df[data_df["Label"] == 1.0]
flow1 = flow1["Packet"]
flow2 = data_df[data_df["Label"] == 2.0]
flow2 = flow2["Packet"]
default = data_df[data_df["Label"] == 3.0]
default = default["Packet"]

flow1_mean = flow1.rolling(window=100, center=True, min_periods=1).mean()
flow2_mean = flow2.rolling(window=100, center=True, min_periods=1).mean()
defualt_mean = default.rolling(window=100, center=True, min_periods=1).mean()

tmp = data_df[data_df["Label"] == 1.0]
flow1 = pd.DataFrame({'Time' : tmp['Time'], 'Packet' : flow1_mean})
tmp = data_df[data_df["Label"] == 2.0]
flow2 = pd.DataFrame({'Time' : tmp['Time'], 'Packet' : flow2_mean})
tmp = data_df[data_df["Label"] == 3.0]
default = pd.DataFrame({'Time' : tmp['Time'], 'Packet' : defualt_mean})
cnt = 0

flow1.to_csv('flow1.dat', header=None, index=False, sep='\t')
flow2.to_csv('flow2.dat', header=None, index=False, sep='\t')
default.to_csv('default.dat', header=None, index=False, sep='\t')
