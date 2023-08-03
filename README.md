# Overview


Various files/directories purpose:
    
    html/                 - Exported HTML copy of the Jupyter Notebook in case you don't want to use Jupyter
    notebooks/            - Jupyter notebooks for the bulk of data/analysis and for creating the keyboard GIFs
    notebooks/images/     - Animated GIFs used in the Jupyter Notebook
    notebooks/wordlists/  - The wordlists used by the Jupyter Notebooks
    keywalk.c             - The C version to evaluate wordlists and output the CSVs used by the Jupyter Notebooks
    keywalk.py            - Original version I started with to evaluate things, but let's face it: python is slow

## Setting up the python environment

Unfortunately the rockyou.txt and rockyou.txt_results file are above the warning and error thresholds for github so right now they are missing from this archive.  It should be easy enough to recreate them using the steps below.

bunzip2 the wordlists and result wordlists:

```
    (jh) [jeremy@devone keywalk]$ find notebooks/wordlists/ -type f -print0 | xargs -0 -i -P 4 bunzip2 {}
```


Roughly create a venv for jupyterhub and use pip3 to install the various necessary packages.  Certainly other python environments might work.

```
    [jeremy@devone ~]$ python -m venv create jh
    [jeremy@devone ~]$ cd jh
    [jeremy@devone jh]$ pip3 install jupyter numpy pandas matplotlib
    Defaulting to user installation because normal site-packages is not writeable
    ...
    [jeremy@devone jh]$ ls
    bin  include  lib  lib64  pyvenv.cfg
    [jeremy@devone jh]$ . bin/activate
    (jh) [jeremy@devone jh]$ jupyter notebook
```


## Processing wordlists

1. Compile the keywalk code:

```
    [jeremy@devone pam_keywalk]$ gcc -O2 keywalk.c -o keywalk
```

2. Process the wordlists (output is standard output so redirect stdout to save to a file):
```
    [jeremy@devone pam_keywalk]$ time ./keywalk wordlists/rockyou.txt  > wordlists/results/rockyou_results
    real	0m14.983s
    user	0m14.869s
    sys	0m0.098s
```


## Questions

**Why a Jupyter Notebook?**

I don't like powerpoint and so I usually export to PDF.  However, in PDF form you will be missing the animated GIFs which loses some of the value here.  Plus when presenting I figured it was better to have the data ready available in case there were questions.


**Would it be better to just generate a wordlist**

It's likely a simple algorithm can be significantly smaller and more accurate that a giant wordlist of keyboard walks (at least from my current analysis).  I'm more then willing to see data to the contrary and change my tune.

