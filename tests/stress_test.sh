#!/bin/bash

generate_random_bytes() {
    dd if=/dev/urandom of=$1 bs=1 count=$2
}

failed() {
    echo "Test failed $1"
    exit 1
}

keygen=./rsakeygen
encrypt=./rsaencrypt
decrypt=./rsadecrypt

prefix=randtest_
min_test_size=100
max_test_size=3000
size_step=300
n_tests=5

# start test
# -- change directory to rsa-simple/tests
cd $1

# -- generate rsa key
keysize=256
public_key=public
private_key=private
echo "generating key with size ${keysize} bytes"
${keygen} ${keysize} ${public_key} ${private_key} || failed "error in keygen: size ${keysize}"

# -- stress testing
for test_size in `seq ${min_test_size} ${size_step} ${max_test_size}`
do
    echo "testing random messages with size ${test_size}"
    for i in `seq 1 ${n_tests}`
    do
        echo "---->testing message ${i}"
        message=${prefix}size_${i}
        message_encrypted=${message}encrypt
        message_decrypted=${message}decrypt
        generate_random_bytes ${message} ${test_size} > /dev/null 2> /dev/null
        ${encrypt} ${public_key} ${message} ${message_encrypted} || failed "error in encryption"
        ${decrypt} ${private_key} ${message_encrypted} ${message_decrypted} || failed "error in decryption"
        diff ${message} ${message_decrypted} || failed "error in rsa algorithm"
        rm ${message} ${message_encrypted} ${message_decrypted}
    done
done

rm ${public_key} ${private_key}
echo "All tests are passed"
