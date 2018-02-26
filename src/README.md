# BISEN source code

## Dependencies
To run BISEN, you will need to install libsodium-sgx, along with Intel SGX drivers and SDK.

## Compiling and running
```
export DATASET_DIR="Data/parsed/1000/"

(cd build && cmake .. && make clean && make)

(cd bin && ./Test)

(cd bin && ./Server &)

(cd src/tsgx && make clean && make && cd build/ && ./test_bisen)
```


## Enron dataset parsing
1. Extract the Enron dataset into the folder _Data/enron_.
1. Generate the benchmark dataset: ```(cd Data && python parser.py <number-of-docs>)```


## Tips
**Note:** config flags may be set-up in the Test Makefile.
-DVERBOSE enables verbose output; -DLOCALTEST enables an SGX simulation by running the generated _./main_.


# TODO Legacy README below, needs revising
## Directory descriptions
* **Iee** is the trusted dir, to be executed in a trusted environment. Also contains respective ECALLS.

* **Data** is to contain the dataset to be used for benchmarking.

* **Common** provides common utility functions for both the Client and Server modules.

* **Client** generates the queries and the respective byte arrays which are used as inputs to the IEE.

* **Test** contains the benchmark generator, along with an optional
simulator to run BISEN without SGX support, as explained above.

* **tsgx** contains the MPC framework to run BISEN with SGX support.
