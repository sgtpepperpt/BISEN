to build test_mpc: (libsodium-sgx should be installed)

```
$ make test_mpc
$ (cd build/ && ./test_mpc)
```

to update secret_key and public keys:

```
$ make keypair
$ ./keypair test_mpc/secret_key.h test_mpc/f/public_key.h
```
