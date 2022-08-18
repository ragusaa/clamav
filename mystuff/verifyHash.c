#define CBC 0
#define CTR 0
#define ECB 1
#include "aes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define FREE(ptr) if (ptr){ free(ptr); ptr = NULL; }



//#define KEY 0xe8\x1d\x06\x68\xa9\x42\xf1\x7a\x39\xe6\x9a\x50\x34\x6e\x32\xf6"


const uint8_t KEY[] = {0xe8, 0x1d, 0x06, 0x68, 0xa9, 0x42, 0xf1, 0x7a, 0x39, 0xe6, 0x9a, 0x50, 0x34, 0x6e, 0x32, 0xf6};
//Obviously this being global is a bad idea.

#if 0
int getBuffer(uint8_t ** buffer, size_t * bufferSize) {

    uint8_t * buf = NULL;
    size_t bufSize = 32; 
    int ret = -1;
    const uint8_t * 

    *buffer = NULL;
    *bufferSize = 0;

    buf = malloc[bufSize];
    if (NULL == buf){
        perror("malloc");
        goto done;
    }

    buf[0] = ;


    ret = 0;
    *buffer = buf;
    *bufferSize = bufSize;
done:
    return ret;

}

void writeAll(uint8_t * ptr, size_t bufferSize){
    size_t bytesWritten = 0;
    int err = 1;
#define FILENAME "mydecrypted.data"
    FILE * fp = fopen(FILENAME, "wb");
    if (NULL == fp){
        goto done;
    }
    while (bytesWritten < bufferSize){
        int ret = fwrite(&(ptr[bytesWritten]), 1, bufferSize - bytesWritten, fp);
        if (0 == ret){
            break;
        }
        bytesWritten += ret;
    }
    if (bytesWritten != bufferSize){
        goto done;
    }

    err = 0;
done:
    if (fp){
        fclose(fp);
    }
    if (err){
        unlink(FILENAME);
    }

}
#endif

void print(uint8_t * buffer, size_t startIdx, size_t endIdx){
    size_t i;
    int first = 1;
    for (i = startIdx; i < endIdx; i++){ 
        if ((0 == first) && (0 == i%16)){
            printf("\n");
        }
        first = 0;

        printf("%02x ", buffer[i]); 
    } 
    printf("\n");
}

unsigned char *cl_sha1(const void *buf, size_t len, unsigned char *obuf, unsigned int *olen);

#define SHA1_HASH_SIZE  20



typedef struct {
    uint32_t saltSize;
    uint8_t salt[16];
    uint8_t encryptedVerifier[16];
    uint32_t verifierHashSize;
    uint8_t encryptedVerifierHash[1]; //variable length.  Depends on algorithm.

} EncryptionVerifier;

/*Returns 1 on encryption verified, 0 no encryption not verified, -1 on error*/
int verifyEncryption(EncryptionVerifier * ev){
    int ret = -1;
    size_t i = 0;
    struct AES_ctx evCtx = {0};
    struct AES_ctx evhCtx = {0};
    uint8_t sha[SHA1_HASH_SIZE ] = {0};
    const uint32_t encryptedVerifierHashLen = 32; /*TODO: figure this out based on the algorithm.*/

    AES_init_ctx(&evCtx, KEY);
    AES_init_ctx(&evhCtx, KEY);


    for (i = 0; i < sizeof(ev->encryptedVerifier); i+= 16){
        AES_ECB_decrypt(&evCtx, &(ev->encryptedVerifier[i]));
    }

    cl_sha1(ev->encryptedVerifier, sizeof(ev->encryptedVerifier), sha, NULL);

    for (i = 0; i < encryptedVerifierHashLen; i+= 16){
        AES_ECB_decrypt(&evhCtx, &(ev->encryptedVerifierHash[i]));
    }

    if (0 == memcmp(sha, ev->encryptedVerifierHash, ev->verifierHashSize)){
        ret = 1;
        printf("Hashes match, this is teh key!!!\n");
    } else {
        ret = 0;
        printf("Hashes do NOT match, this is not the key\n");
    }

done:
    return ret;
}




int main(){

#if 0
    size_t i;
    struct AES_ctx ctx = {0};
    struct AES_ctx ctx2 = {0};
    uint8_t sha[SHA1_HASH_SIZE ];


    uint8_t encryptedVerifierHash[] = {0xd2, 0x20, 0xa4, 0xa9, 0x5c, 0x37, 0x0b, 0xc2, 0xe6, 0x15, 0xdf, 0x70, 0xb5, 0x69, 0x78, 0x36, 0x8b, 0x82, 0x51, 0xa6, 0x96, 0xf9, 0x1d, 0x89, 0x1f, 0xf6, 0x9c, 0x8a, 0x13, 0x01, 0x04, 0x7d};

    uint8_t encryptedVerifier[] = {0x46, 0x47, 0x7b, 0xc3, 0xb4, 0x87, 0x01, 0x44, 0x83, 0xe4, 0x4f, 0x7e, 0x60, 0xa4, 0xa4, 0x6a};


    printf("Calling print before decrypt\n");
    print(encryptedVerifier, 0, sizeof(encryptedVerifier));
    printf("After print before decrypt\n");

    AES_init_ctx(&ctx, KEY);

    for (i = 0; i < sizeof(encryptedVerifier); i+= 16){
        AES_ECB_decrypt(&ctx, &(encryptedVerifier[i]));
    }

    printf("Calling print after decrypt\n");
    print(encryptedVerifier, 0, sizeof(encryptedVerifier));
    printf("After print after decrypt\n");

    cl_sha1(encryptedVerifier, sizeof(encryptedVerifier), sha, NULL);


    printf("Printing sha\n");
    print(sha, 0, SHA1_HASH_SIZE);


    AES_init_ctx(&ctx2, KEY);
    for (i = 0; i < sizeof(encryptedVerifierHash); i+= 16){
        AES_ECB_decrypt(&ctx2, &(encryptedVerifierHash[i]));
    }
    print(encryptedVerifierHash, 0, sizeof(encryptedVerifierHash));


    if (0 == memcmp(sha, encryptedVerifierHash, SHA1_HASH_SIZE)){
        printf("Hashes match, this is teh key!!!\n");
    } else {
        printf("Hashes do NOT match, this is not the key\n");
    }




done:
#else
    uint8_t ary[512];
    EncryptionVerifier * ev = (EncryptionVerifier*) ary;
    uint8_t encryptedVerifierHash[] = {0xd2, 0x20, 0xa4, 0xa9, 0x5c, 0x37, 0x0b, 0xc2, 0xe6, 0x15, 0xdf, 0x70, 0xb5, 0x69, 0x78, 0x36, 0x8b, 0x82, 0x51, 0xa6, 0x96, 0xf9, 0x1d, 0x89, 0x1f, 0xf6, 0x9c, 0x8a, 0x13, 0x01, 0x04, 0x7d};

    uint8_t encryptedVerifier[] = {0x46, 0x47, 0x7b, 0xc3, 0xb4, 0x87, 0x01, 0x44, 0x83, 0xe4, 0x4f, 0x7e, 0x60, 0xa4, 0xa4, 0x6a};

    memcpy(ev->encryptedVerifierHash, encryptedVerifierHash, sizeof(encryptedVerifierHash));
    memcpy(ev->encryptedVerifier, encryptedVerifier, sizeof(encryptedVerifier));
    ev->verifierHashSize = 20;

    printf ("verifyEncryption returned %d\n", verifyEncryption(ev));



#endif

    return 0;
}

