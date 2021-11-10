# TCP-Samples

Ubuntu Openssl- 

# Install Library from apt

sudo apt install libssl-dev
gcc -o aes aesni-eval.c -lssl -lcrypto


# Build OpenSSL from source

cd openssl-3.0.0

./config shared --prefix=/home/dimuthu/Downloads/openssl-3.0.0/build

make

make install

gcc -I/home/dimuthu/Downloads/openssl-3.0.0/build/include -o aes -L/home/dimuthu/Downloads/openssl-3.0.0/build/lib64 aesni-eval.c -lcrypto


# Build clients
gcc -o client simple-client.c -lssl -lcrypto

# Build servers
gcc -o server simple-server.c -lssl -lcrypto
