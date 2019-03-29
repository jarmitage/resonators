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

from paramiko import SSHClient, SSHConfig, ProxyCommand, Transport
from scp import SCPClient
import sys, os

indianDrumModels = pd.Series(['Chakoa-8','Chakoa-9','Dhalki-2','Duggi-4','Hand-Dhal','khol','Madal','Manjeera-1','Mirdangam-1','Mirdangam-4','Mirdangam-15','Mirdangam-low-1','Stick-Dhal-1','Stick-Dhal-8'])
indianDrumModels = '../models/bela/indian-roots_' + indianDrumModels

def pd2model (file):
    # this function will read a `resonators~` `.coll` model file into a Pandas DataFrame
    directory, ext = '../data/models/pd/', '.coll' # path and extension
    model = pd.read_csv(directory+file+ext, sep=",",index_col=0,header=None) # load model
    model[1] = model[1].str.strip() # strip whitespace
    model = pd.DataFrame(model[1].str.split(' ',n=3,expand=True)) # split values into columns
    model.index.name = '' # remove index name
    model.columns = ['Frequency','Gain','Decay'] # name columns
    model['Decay'] = model['Decay'].str[:-1] # remove `;` from Decay column values
    model = model.apply(pd.to_numeric) # convert from string to float
    return model

def bela2model (file, path="../data/models/bela/"):
    model = pd.read_csv(path+file, sep=',')
    model.columns = ['Frequency','Gain','Decay']
    return model

def model2Bela (model, file):
    model.to_csv('../data/models/bela/'+file, header=False, index=False)

def model2Pd (model, file):
    pdListMessage  = ' ' + model['Frequency'].astype(str)
    pdListMessage += ' ' + model['Gain'].astype(str)
    pdListMessage += ' ' + model['Decay'].astype(str) + ';'
    pdModel = pd.Series(pdListMessage.values)
    pdModel.to_csv('../data/models/resModel_pd_'+file+'.coll', header=False)
    
def plotDecayRateVsFreq (model, model_name, save):
    freq = model['Frequency']
    gain = model['Gain']
    decay = pd.Series(np.exp(-model['Decay'] * (1/44100)))
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
        axis.get_figure().savefig('../data/plots/decay-vs-freq_'+model_name+'.pdf')

def plotFreqVsGD (model, model_name, save):
    freq = model['Frequency']
    gain = model['Gain']
    decay = pd.Series(np.exp(-model['Decay'] * (1/44100)))
    xlabel, ylabel1, ylabel2 = 'Frequency', 'Gain', 'Decay Rate'
    xlim, ylim1, ylim2 = (20,20000), (0, 0.3), (0.9995, 1.0)
    fig, axis1 = plt.subplots(figsize=(12,6))
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
        axis1.get_figure().savefig('plots/freq-vs-gd_'+model_name+'.pdf')
        
def modelGen (freq, gain, decay, resonators):
    res = pd.DataFrame ([],np.arange (0, resonators), ['Frequency', 'Gain', 'Decay'] ).fillna (0)
    res['Frequency'], res['Gain'], res['Decay'] = freq, gain, decay
    for i in range (0, resonators):
        res['Frequency'][i] += np.random.randint (0, 100) + ((i*i)*10+np.random.randint(0,5))
        res['Gain'][i]      += np.random.uniform (0, 0.1)
        res['Decay'][i]     += np.random.uniform (0, 2.0)
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

def binary2df (path, dt, sep=""):
    return pd.DataFrame (np.fromfile (path, dtype=dt, sep=sep))

def text2df (path, skip=None, sep=""):
    return pd.read_csv (path, sep=sep, skiprows=skip)

def log2wav (path, samplerate, values):
    spiowav.write (path, samplerate, values)


