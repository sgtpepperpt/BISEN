# BISEN source code

## Dependencies
To run BISEN, you will need to install libsodium-sgx, along with Intel SGX drivers.


## Compiling and running
1. Extract the Enron dataset into the folder _Data/enron_.
1. Generate the benchmark dataset: ```(cd Data && python parser.py <number-of-docs>)```
1. Go to folder Server and execute: ```make && ./Server # leave the server always running in background while running the tests```
1. Go to folder Test and execute: ```make clean && make && ./main```
  1. If you specified a different _<number-of-docs>_ in 2., do this before compiling: ```export DATASET_SIZE=<number-of-docs>```
1. Go to folder tsgx and execute: ```make clean && make && (cd build && ./test_bisen)```

**Note:** config flags may be set-up in the Test makefile.
-DVERBOSE enables verbose output; -DLOCALTEST enables an SGX simulation by running the generated _./main_.


## Directory descriptions
* **Iee** is the trusted dir, to be executed in a trusted environment. Also contains respective ECALLS.

* **Data** is to contain the dataset to be used for benchmarking.

* **Common** provides common utility functions for both the Client and Server modules.

* **Client** generates the queries and the respective byte arrays which are used as inputs to the IEE.

* **Test** contains the benchmark generator, along with an optional 
simulator to run BISEN without SGX support, as explained above.

* **tsgx** contains the mpc framework to run BISEN with SGX support.
