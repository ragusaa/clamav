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
//Obviously this is a bad ideal.
size_t bufferSize = 0;

uint8_t * getBuffer() {
    uint8_t * ret = NULL;
    int err = 1;
    size_t size = 0;
    size_t bytesRead = 0;

    FILE * fp = fopen("andy.data", "rb");
    if (NULL == fp){
        goto done;
    }

    if (fseek(fp, 0, SEEK_END)){
        goto done;
    }

    size = ftell(fp);
    rewind(fp);

    printf("size = %ld\n", size);
    //printf("size = %ld\n", ret[90]);

    ret = malloc(size);
    if (NULL == ret){
        goto done;
    }

    while (bytesRead < size){
        int br = fread( &(ret [bytesRead]), 1, size - bytesRead, fp);
        if (0 == br){
            break;
        }
        bytesRead += br;
    }

    if (bytesRead != size){
        goto done;
    }

    err = 0;
    bufferSize = size;

done:
    if (fp){
        fclose(fp);
    }
    if (err){
        FREE(ret);
    }
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

int main(){

    size_t i;
    struct AES_ctx ctx = {0};
    uint8_t * buffer = getBuffer();
    if (NULL == buffer){
        goto done;
    }

    AES_init_ctx(&ctx, KEY);

    print(buffer, 0, 16);

    print(buffer, bufferSize - 16, bufferSize);
    printf("\n");

    for (i = 0; i < 74448; i+= 16){
        AES_ECB_decrypt(&ctx, &(buffer[i]));
    }

    writeAll(buffer, bufferSize - 12);

    print(buffer, 0, 16);
    print(buffer, bufferSize - 16, bufferSize);
done:

    FREE(buffer);
    return 0;
}

