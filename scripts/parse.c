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
#define DATA_LEN 2041

int parseVolumeDescriptor(const uint8_t * const data) {
    size_t parseIdx = 0;
    size_t i = 0;
    int ret = -1;
    int allZeros = 1;
    int terminator = 0;

    printf("Volume Descriptor '");

#if 0
    if (255 == data[parseIdx]){
        terminator = 1;
    } else if (1 == data[parseIdx]){
        printf("Primary Volume Descriptor ");
    }
#endif
    printf("Type '");
    switch (data[parseIdx]){
        case 255:
        printf("Terminator");
        terminator = 1;
        break;
        case 1:
        printf("Primary Volume Descriptor");
        break;
        case 0:
        /*fall-through*/
        case 2:
        /*fall-through*/
        case 3:

        printf("%d", data[parseIdx]);
        break;
        default:
        printf("Invalid Volume Descriptor'\n");
        goto done;
    }
    printf("'\n");

    /*
     * 0	Boot record volume descriptor
     * 1	Primary volume descriptor
     * 2	Supplementary volume descriptor, or enhanced volume descriptor
     * 3	Volume partition descriptor
     * 255	Volume descriptor set terminator
     *
     */
    parseIdx += 1;

    /*TODO: According to the spec, there are only a select number of values, but some of
     * my samples have other stuff in there.  determine whether or not it is necessary / suspicious to
     * validate these strings.*/
    printf("Identifier '");
    for (i = 0; i < 5; i++){
        printf("%c", data[parseIdx++]);
    }
    printf("'\n");

    printf("Version '");
    printf("%d'\n", data[parseIdx]);
    parseIdx += 1;

    printf("Data '");
    allZeros = 1;
    for (i = 0; i < DATA_LEN; i++){
        if (0 != data[parseIdx + i]){
            allZeros = 0;
            break;
        }
    }
    if (allZeros){
        printf("All zeros");
        parseIdx += DATA_LEN;
    } else {

        for (i = 0; i < DATA_LEN; i++){
            printf("%02x ", data[parseIdx++]);
        }
    }
        printf("'\n");

    if (0 == terminator){
    ret = 0;
    }
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
