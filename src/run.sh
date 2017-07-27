export HOME_DIR="/home/pepper/"

#path of keys relative to home dir
export IEE_PUB_FILE="IeePub.pem"
export KCOM_FILE="Kcom"
export IEE_PRIV_FILE="IeePriv.pem"

cd Server
make
./Server &
cd ..

cd Iee
make
./Iee &
cd ..

cd Client
make
./Client
cd ..
