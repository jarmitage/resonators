#### utilities for working with resonator models
# 
# globals:
#   indianDrumModels: pd.Series of drum model names
#
# import and export functions:
#   pd2model (file)
#   bela2model (file)
#   model2Pd
#   model2Bela (model, file)
#
# plotting functions:
#   plotModel (model, model_name, save)
#
# working with models:
#   modelGen (freq, gain, decay, resonators)
# 
# other:
#   scp_bela
# 
# logging:
#   binary2df (path, datatype, separator)
#   text2df   (path, skiprows, separator)
#   log2wav   (path, sampleRate, values)
#

import math
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import scipy.io.wavfile as spiowav
import IPython.display as ipd
import datetime
import subprocess
import json

from paramiko import SSHClient, SSHConfig, ProxyCommand, Transport
from scp import SCPClient
import sys, os

localModelsPath = '../data/models/'
localPlotsPath = '../data/plots/'

indianDrumModels = pd.Series(['Chakoa-8','Chakoa-9','Dhalki-2','Duggi-4','Hand-Dhal','khol','Madal','Manjeera-1','Mirdangam-1','Mirdangam-4','Mirdangam-15','Mirdangam-low-1','Stick-Dhal-1','Stick-Dhal-8'])
indianDrumModels = '../models/csv/indian-roots_' + indianDrumModels

def coll2df (file):
    # this function will read a `resonators~` `.coll` model file into a Pandas DataFrame
    directory, ext = localModelsPath+"pd/", '.coll' # path and extension
    df = pd.read_csv(directory+file+ext, sep=",",index_col=0,header=None) # load model
    df[1] = df[1].str.strip() # strip whitespace
    df = pd.DataFrame(df[1].str.split(' ',n=3,expand=True)) # split values into columns
    df.index.name = '' # remove index name
    df.columns = ['freq','gain','decay'] # name columns
    df['Decay'] = df['Decay'].str[:-1] # remove `;` from Decay column values
    df = df.apply(pd.to_numeric) # convert from string to float
    return df

def df2coll (df, file):
    pdListMessage  = ' ' + df['Frequency'].astype(str)
    pdListMessage += ' ' + df['Gain'].astype(str)
    pdListMessage += ' ' + df['Decay'].astype(str) + ';'
    pdModel = pd.Series(pdListMessage.values)
    pdModel.to_csv(localModelsPath+file+'.coll', header=False)

def csv2df (file):
    df = pd.read_csv(localModelsPath+file, sep=',', header=None)
    df.columns = ['freq','gain','decay']
    return df

def df2csv (df, file):
    df.to_csv('../data/models/csv/'+file, header=False, index=False)

def model2json(model, file):
    metadata = model["metadata"].T.to_json()
    metadata = metadata[0:len(md_str)-1]+','
    resonators = '"resonators": '+model["resonators"].to_json(orient='records')+'}'
    json_str = metadata+resonators
    json_file = open(localModelsPath+'json/'+file+'.json', "w")
    json_file.write(json_str)
    json_file.close()
    return json_str
    
def json2model(file):
    path, ext = localModelsPath+'json/', '.json'
    data = json.load(open(path+file+ext))
    metadata = pd.DataFrame(data['metadata'],index=["metadata"])
    resonators = pd.DataFrame(data['resonators'])
    cols = resonators.columns.tolist()
    resonators = resonators[cols[1:] + cols[:-2]]
    model = {"metadata": metadata, "resonators": resonators}
    return model

def metadata2df(name, fundamental, resonators):
    return pd.DataFrame([[name,fundamental,resonators]],index=["metadata"],columns=["name","fundamental","resonators"])

def plotDecayRateVsFreq (model, model_name, save):
    freq = model['freq']
    gain = model['gain']
    decay = pd.Series(np.exp(-model['decay'] * (1/44100)))
    xlabel, ylabel = 'Decay Rate', 'Frequency'
    xlim, ylim = (0.9995, 1.0), (20,20000)
    fig, axis = plt.subplots(figsize=(12,6))
    axis.barh(freq, decay, fill=False, linestyle="-", linewidth=gain.values*50, edgecolor=(0,0,1,0.5))
    axis.set_yscale('log')
    axis.set_yticks ([10, 100, 1000, 10000])
    axis.set_xlim (xlim)
    axis.set_ylim (ylim)
    axis.set_xlabel (xlabel)
    axis.set_ylabel (ylabel)
    plt.title ('Model \'' + str (model_name) + '\' has ' + str (len (model)) + ' resonances')
    if save == 1:
        axis.get_figure().savefig(localPlotsPath+'decay-vs-freq_'+model_name+'.pdf')

def plotFreqVsGD (model, model_name, save):
    freq = model['freq']
    gain = model['gain']
    decay = pd.Series(np.exp(-model['decay'] * (1/44100)))
    xlabel, ylabel1, ylabel2 = 'Frequency', 'Gain', 'Decay Rate'
    xlim, ylim1, ylim2 = (20,20000), (0, 0.3), (0.9995, 1.0)
    fig, axis1 = plt.subplots(figsize=(13,6))
    # freq vs. gain
    axis1.bar (freq, gain, linewidth=10, edgecolor=(1,0,0,1))
    axis1.set_xscale ('log')
    axis1.set_xticks ([10, 100, 1000, 10000])
    axis1.set_xlabel (xlabel)
    axis1.set_xlim (xlim)
    axis1.set_ylim (ylim1)
    axis1.set_ylabel (ylabel1, color='r')
    axis1.tick_params('y', colors='r')
    # freq vs. decay
    axis2 = axis1.twinx()
    #axis2.plot (freq, decay, 'sb')
    axis2.bar (freq, decay, linewidth=5, edgecolor=(0,0,1,0.25))
    axis2.set_ylim (ylim2)
    axis2.set_ylabel (ylabel2, color='b')
    axis2.tick_params('y', colors='b')
    plt.title ('Model \'' + str (model_name) + '\' has ' + str (len (model)) + ' resonances')
    if save == 1:
        axis1.get_figure().savefig(localPlotsPath+'freq-vs-gd_'+model_name+'.pdf')

def plotFreqVsGDNorm (model, model_name, save):
    freq = model['freq']
    gain = model['gain']
    decay = model['decay']
    # decay = pd.Series(np.exp(-model['Decay'] * (1/44100)))
    xlabel, ylabel1, ylabel2 = 'Frequency', 'Gain', 'Decay Rate'
    xlim, ylim1, ylim2 = (20,20000), (0, 1), (0, 1.0)
    fig, axis1 = plt.subplots(figsize=(13,6))
    # freq vs. gain
    axis1.bar (freq, gain, linewidth=10, edgecolor=(1,0,0,1))
    axis1.set_xscale ('log')
    axis1.set_xticks ([10, 100, 1000, 10000])
    axis1.set_xlabel (xlabel)
    axis1.set_xlim (xlim)
    axis1.set_ylim (ylim1)
    axis1.set_ylabel (ylabel1, color='r')
    axis1.tick_params('y', colors='r')
    # freq vs. decay
    axis2 = axis1.twinx()
    #axis2.plot (freq, decay, 'sb')
    axis2.bar (freq, decay, linewidth=5, edgecolor=(0,0,1,0.25))
    axis2.set_yscale ('log')
    axis2.set_ylim (ylim2)
    axis2.set_ylabel (ylabel2, color='b')
    axis2.tick_params('y', colors='b')
    plt.title ('Model \'' + str (model_name) + '\' has ' + str (len (model)) + ' resonances')
    if save == 1:
        axis1.get_figure().savefig(localPlotsPath+'freq-vs-gd_'+model_name+'.pdf')

def plotComparisonAndDifferential(measurements, interpModelData, differential):
    xlim, ylim1, ylim2 = (20,20000), (-100, 0), (-50,50)
    xlabel, ylabel = 'Frequency', 'Magnitude (dB)'
    fig, axs = plt.subplots(1,2,figsize=(15,6), gridspec_kw={'width_ratios': [2.5, 1.5]})
    axs[0].plot(measurements['Baseline Magnitude (dB)'], 'r')
    axs[0].plot(measurements['Comparison Magnitude (dB)'], 'b')
    axs[0].plot(interpModelData['Baseline Magnitude (dB)'], 'r', linestyle='None', marker='o')
    axs[0].plot(interpModelData['Comparison Magnitude (dB)'], 'b', linestyle='None', marker='o')
    axs[0].set_title ('Baseline & comparison measurement')
    axs[0].set_xscale ('log')
    axs[0].set_xlim (xlim)
    axs[0].set_ylim (ylim1)
    axs[0].set_xticks ([10, 100, 1000, 10000])
    axs[0].set_xlabel (xlabel)
    axs[0].set_ylabel (ylabel)
    axs[0].grid(b=True, which='both')
    axs[0].legend (measurements.columns)
    axs[1].plot(differential, 'indigo', linestyle='-', marker='o')
    axs[1].set_title ('Differential between baseline & comparison')
    axs[1].set_xscale ('log')
    axs[1].set_ylim (ylim2)
    axs[1].set_xlabel (xlabel)
    axs[1].set_ylabel (ylabel)
    axs[1].grid(b=True, which='both')
    axs[1].legend (differential.columns)

def modelGen (freq, gain, decay, resonators):
    res = pd.DataFrame ([],np.arange (0, resonators), ['freq', 'gain', 'decay'] ).fillna (0)
    res['freq'], res['gain'], res['decay'] = freq, gain, decay
    for i in range (0, resonators):
        res['freq'][i]  += np.random.randint (0, 100) + ((i*i)*10+np.random.randint(0,5))
        res['gain'][i]  += np.random.uniform (0, 0.1)
        res['decay'][i] += np.random.uniform (0, 2.0)
    return res
        
def scp_bela (host='bbb'):
    ssh_config = SSHConfig()
    ssh_config_file = os.path.expanduser('~/.ssh/config')
    if os.path.exists (ssh_config_file):
        with open (ssh_config_file) as f:
            ssh_config.parse (f)
    bbb = ssh_config.lookup (host)
    sf = Transport ((bbb['hostname'], 22))
    sf.connect (username = bbb['user'])
    sf.auth_none (bbb['user'])
    # progress callback for scp transfer
    # def progress(filename, size, sent, peername):
    #     print("%s:%s %s: %.2f%% \r" % (peername[0], peername[1], filename, float(sent)/float(size)*100))
    # return SCPClient(sf, progress = progress)
    return SCPClient(sf)

def scpModel2Bela(belaName, modelName, project):
    scpb = scp_bela(belaName)
    scpb.put (localModelsPath+'json/' + modelName, '~/Bela/projects/'+project+'/models/' +'tmp.json')
    scpb.close()

def binary2df (path, dt, sep=""):
    return pd.DataFrame (np.fromfile (path, dtype=dt, sep=sep))

def text2df (path, skip=None, sep=""):
    return pd.read_csv (path, sep=sep, skiprows=skip)

def log2wav (path, samplerate, values):
    spiowav.write (path, samplerate, values)

def normDfInRange (df, a, b):
    if (df.max().values[0] > abs(df.min().values[0])):
        a = df.min() / df.max()
    else:
        b = -df.max() / df.min()
    return (b - a) * ( (df - df.min()) / (df.max() - df.min()) ) + a

def mapDf(x, in_min, in_max, out_min, out_max):
    return pd.DataFrame((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

def modelNormalize (model, max_gain, max_decay):
    return pd.concat([model['Frequency'], mapDf(model['Gain'], 0, max_gain, 0, 1), mapDf(model['Decay'], 0, max_decay, 0, 1)], axis=1)

def datetime_str():
    return datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S")
