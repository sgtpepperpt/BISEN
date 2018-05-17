# Compilation Overview 

cd ../libsodium
./configure --enable-static --disable-libtool-lock --without-pthreads --enable-minimal
make && make check

cd ../libsodium-sgx
mkdir build

find . -name *.o ../libsodium | grep crypto_scalarmult | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep sha | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep secretbox | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_stream | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_onetimeauth | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_core/hsalsa20 | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_verify | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_sign | while read fn ; do cp $fn ../build/; done
find . -name *.o ../libsodium | grep crypto_core/curve25519 | while read fn ; do cp $fn ../build/; done

gcc -Wall -c src/utils.c -o build/utils.o

ar rcs libsodium.a build/*.o


