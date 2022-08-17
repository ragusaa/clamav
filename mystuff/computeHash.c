#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>


unsigned char *cl_sha1(const void *buf, size_t len, unsigned char *obuf, unsigned int *olen);


#define FREE(__ptr__) if (__ptr__){ free(__ptr__); __ptr__ = NULL; }

//https://msdn.microsoft.com/en-us/library/dd910568(v=office.12).aspx
//This is stored in EncryptionVerifier.  Salt is located at offset 2204 (in this document).  Can't find out how to find teh salt based on teh spec.  Still looking.
/*TODO: figure out where salt comes from.*/
const uint8_t SALT[] = { 0xc3, 0xfd, 0x4d, 0x25, 0x33, 0x01, 0xa6, 0x79, 0x11, 0xe2, 0x6d, 0xcf, 0x9c, 0xfc, 0xb3, 0x98};
#define SALT_LEN 16

//#define SHA1_HASH_SIZE 16
#define SHA1_HASH_SIZE 20

void dump(const uint8_t * const buffer, const size_t bufLen){
    size_t i;

    for (i = 0; i < bufLen; i++){
        printf("%02x ", buffer[i]);
    }
    printf("\n");

    for (i = 0; i < bufLen; i++){
        uint8_t ui = buffer[i];
        if (ui>= 32 && ui <= 126){
            printf(" %c ", ui);
        } else {
            printf("%02x ", ui);
        }
    }
    printf("\n");

}

/*Want to specificially get LE value.*/
uint32_t getLEUint32(uint32_t val){
    val = htonl(val);

    val = ((val>>24)&0xff) | // move byte 3 to byte 0
                    ((val<<8)&0xff0000) | // move byte 1 to byte 2
                    ((val>>8)&0xff00) | // move byte 2 to byte 1
                    ((val<<24)&0xff000000); // byte 0 to byte 3
    return val;

}


int computeHash(const char * const password, const uint32_t keyLength){
    uint8_t * buffer = NULL;
    size_t bufLen = 0;
    int ret = -1;
    uint32_t i = 0;
    uint8_t sha1[sizeof(uint32_t) + SHA1_HASH_SIZE + sizeof(uint32_t)] = {0};
    uint8_t * sha1Dst = &(sha1[sizeof(uint32_t)]);
    uint8_t buf1[64];
    uint8_t buf2[64];
    uint8_t doubleSha[SHA1_HASH_SIZE * 2];

#define ITER_COUNT 50000

    bufLen = SALT_LEN + (strlen(password) * 2);

    buffer = calloc(bufLen, 1);
    if (NULL == buffer){
        perror("calloc");
        goto done;
    }

    memcpy (buffer, SALT, SALT_LEN);

    /*Convert to UTF16-LE*/
    for (i = 0; i < (uint32_t) strlen(password); i++){
        buffer[SALT_LEN + (i * 2)] = password[i];
    }

    cl_sha1(buffer, bufLen, sha1Dst, NULL);

    //dump(sha1Dst, SHA1_HASH_SIZE);
    for (i = 0; i < ITER_COUNT; i++){
        uint32_t eye = getLEUint32(i);

        memcpy(sha1, &eye, sizeof(eye));
        cl_sha1(sha1, SHA1_HASH_SIZE + sizeof(uint32_t), sha1Dst, NULL);
    }

    memset(&(sha1Dst[SHA1_HASH_SIZE]), 0, sizeof(uint32_t));

    cl_sha1(sha1Dst, SHA1_HASH_SIZE + sizeof(uint32_t), sha1Dst, NULL);

    //dump(sha1Dst, SHA1_HASH_SIZE); //'hfinal = ' in the python code

    memset(buf1, 0x36, sizeof(buf1));
    for (i = 0; i < SHA1_HASH_SIZE; i++){
        buf1[i] = buf1[i] ^ sha1Dst[i];
    }
    //dump(buf1, sizeof(buf1));

    //now sha1 buf1
    cl_sha1(buf1, sizeof(buf1), doubleSha, NULL);


    memset(buf2, 0x5c, sizeof(buf2));
    for (i = 0; i < SHA1_HASH_SIZE; i++){
        buf2[i] = buf2[i] ^ sha1Dst[i];
    }
    //dump(buf2, sizeof(buf2));

    cl_sha1(buf2, sizeof(buf2), &(doubleSha[SHA1_HASH_SIZE]), NULL);


    for (i = 0; i < keyLength; i++){
        printf("%02x ", doubleSha[i]);
    }
    printf("\n");



    ret = 0;
done:
    FREE(buffer);

    return ret;
}




int main(int argc, char ** argv){

    int i = 0;

    if (1 == argc){
        computeHash("VelvetSweatshop", 16);
    } else {
        for (i = 1; i < argc; i++){
            computeHash(argv[i], 16);
        }
    }


    return 0;
}



