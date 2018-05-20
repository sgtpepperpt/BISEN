to build test_bisen: (libsodium-sgx should be installed)

```
$ make test_bisen
$ (cd build/ && ./test_bisen)
```

to update secret_key and public keys:

```
$ make keypair
$ ./keypair test_mpc/secret_key.h test_mpc/f/public_key.h
```
