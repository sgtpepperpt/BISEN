#!/bin/bash

mkdir -p build
find ../libsodium -name *.o | grep crypto_scalarmult | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep sha | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep secretbox | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_stream | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_onetimeauth | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_core/hsalsa20 | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_verify | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_sign | while read FN ; do cp $FN build/; done
find ../libsodium -name *.o | grep crypto_core/curve25519 | while read FN ; do cp $FN build/; done
