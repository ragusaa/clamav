#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int readData(const char * const fileName, uint8_t ** data, size_t * dataLen){
    int ret = -1;
    FILE * fp = NULL;
    size_t bytesRead = 0;

    uint8_t* dRet = NULL;
    size_t dRetLen = 0;
    *data = NULL;
    *dataLen = 0;

    fp = fopen(fileName, "rb");

    if (NULL == fp){
        perror("fopen");
        goto done;
    }

    fseek(fp, 0, SEEK_END);

    dRetLen = ftell(fp);
    rewind (fp) ;

    dRet = malloc(dRetLen);
    if (NULL == dRet){
        perror("malloc");
        goto done;
    }

    while (bytesRead < dRetLen){
        int ret = fread(&(dRet[bytesRead]), 1, dRetLen - bytesRead, fp);
        if (ret > 0){
            bytesRead += ret;
        } else {
            break;
        }
    }

    if (bytesRead != dRetLen){
        goto done;
    }

    ret = 0;
    *data = dRet;
    *dataLen = dRetLen;

done:
    if (ret){
        free(dRet);
    }
    return ret;
}

//https://en.wikipedia.org/wiki/ISO_9660
#define EMPTY_LEN 32768

#define VOLUME_DESCRIPTOR_SIZE 0x800

int parseVolumeDescriptor(const uint8_t * const data) {
    size_t parseIdx = 0;
    size_t i = 0;
    int ret = -1;

    printf("Volume Descriptor\n");

    printf("Type\n");
    printf("%d\n", data[parseIdx]);
    /*
     * 0	Boot record volume descriptor
     * 1	Primary volume descriptor
     * 2	Supplementary volume descriptor, or enhanced volume descriptor
     * 3	Volume partition descriptor
     * 255	Volume descriptor set terminator
     *
     */
    parseIdx += 1;

    printf("Identifier\n");
    for (i = 0; i < 5; i++){
        printf("%c", data[parseIdx++]);
    }
    printf("\n");

    printf("Version\n");
    printf("%d\n", data[parseIdx]);
    parseIdx += 1;

    printf("Data\n");
    for (i = 0; i < 2041; i++){
        if (0 != data[parseIdx]){
            printf("idx = '%ld', %02x\n", parseIdx, data[parseIdx]);
            goto done;
        }

        parseIdx++;
    }
    printf("All zeros\n");

    ret = 0;
done:
    return ret;


}


int parseFile(const char * const fileName){

    uint8_t * data = NULL;
    size_t dataLen = 0;
    int ret = -1;
    size_t i, parseIdx = EMPTY_LEN;

    if (readData(fileName, &data, &dataLen)){
        printf("%s::%d::Unable to read file\n", __FUNCTION__, __LINE__);
        goto done;
    }
    if (dataLen < EMPTY_LEN){
        printf("%s::%d::ERROR\n", __FUNCTION__, __LINE__);
        goto done;
    }

    printf("dataLen = %ld\n", dataLen);


#if 0
    int andy = 0;
#endif
    for (i = EMPTY_LEN; i < dataLen; i+= VOLUME_DESCRIPTOR_SIZE){
        if (parseVolumeDescriptor(&(data[i ]))){
            goto done;
        }

#if 0
        if (1 == andy){
            break;
        }
        andy++;
#endif
    }








    printf("Success\n");




ret = 0;

done:
if (data){
    free(data);
}
return ret;
}

int main(int argc, char ** argv){

    int i;
    for (i = 1; i < argc; i++){
        parseFile(argv[i]);
    }


    return 0;
}
