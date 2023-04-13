#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>



//https://en.wikipedia.org/wiki/ISO_9660
#define EMPTY_LEN 32768

#define VOLUME_DESCRIPTOR_SIZE 0x800
#define DATA_LEN 2041
//https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf

#define PRIMARY_VOLUME_DESCRIPTOR 1



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


/*
 * BEA01
 * Beginning Extended Area Descriptor
 *
 * */
void handleBEA01(const uint8_t* const data){
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.2

    printf("%s::%d::Verify All Zeros\n", __FUNCTION__, __LINE__);
}
int handleBOOT2(const uint8_t* const data){ 

    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.4


    int ret = -1;
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
    exit(1);

    size_t idx = 0;
    if (0 != data[idx]){
        printf("%s::%d::Invalid Structure type of '%d (0x%x)'\n", __FUNCTION__, __LINE__, data[idx], data[idx]);
        goto done;
    }



    ret = 0;
done:
    return ret;
}
void handleCD001(const uint8_t* const data){
    //ECMA-119
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleCDW02(const uint8_t* const data){
    //ECMA-168
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleNSR02(const uint8_t* const data){
    /*allow NSR02 in reading, but should be similar to NSR03?  Just a version number change, but don't know 
     * what else actually changed.
     */
    //section 3/9.1 of ECMA167/2
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleNSR03(const uint8_t* const data){
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf    section 3/9.1
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
/*
 * TEA01
 * 
 * Terminating Extencded Area Descriptor
 */
void handleTEA01(const uint8_t* const data){
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.3
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}


void handlePrimaryVolumeDescriptor( const uint8_t * const data) {
    size_t i;
    for (i = 0; i < VOLUME_DESCRIPTOR_SIZE ; i++){
        printf("%02x ", data[i]);
        }
    printf("\n");
}



int parseVolumeDescriptor(const uint8_t * const data, size_t sectorNumber) {
    size_t parseIdx = 0;
    size_t i = 0;
    int ret = -1;
    int allZeros = 1;
    int terminator = 0;
    char identifier[6];
    uint8_t volumeDescriptorType = data[parseIdx];


    /**/
    if (PRIMARY_VOLUME_DESCRIPTOR == volumeDescriptorType){
        handlePrimaryVolumeDescriptor(data);
    }




    printf("Volume Descriptor %lu\n", sectorNumber);

#if 0
    if (255 == data[parseIdx]){
        terminator = 1;
    } else if (1 == data[parseIdx]){
        printf("Primary Volume Descriptor ");
    }
#endif
    printf("Type '");
    switch (volumeDescriptorType){
        case 255:
            printf("Terminator");
            terminator = 1;
            break;
        case 1:
            printf("Primary Volume Descriptor");
            break;
        case 0:
            /*Used in section 2.*/
            /*fall-through*/
        case 2:
            /*fall-through*/
        case 3:

            printf("%d", volumeDescriptorType);
            break;
        default:
            printf("Invalid Volume Descriptor '%d (0x%x)\n", volumeDescriptorType , volumeDescriptorType);
#if 0
            goto done;
#else
            printf("NOT going to done so that I can figure out what is going on here\n");
#endif
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
    for (i = 0; i < 5; i++){
        identifier[i] = data[parseIdx++];
    }
    identifier[5] = 0;
    printf("Identifier '%s'\n", identifier);
    if (0 == strcmp("BEA01", identifier)){
        handleBEA01(data);
    } else if (0 == strcmp("BOOT2", identifier)){
        handleBOOT2(data);
    } else if (0 == strcmp("CD001", identifier)){
        handleCD001(data);
    } else if (0 == strcmp("CDW02", identifier)){
        handleCDW02(data);
    } else if (0 == strcmp("NSR02", identifier)){
        handleNSR02(data);
    } else if (0 == strcmp("NSR03", identifier)){
        handleNSR03(data);
    } else if (0 == strcmp("TEA01", identifier)){
        handleTEA01(data);
    }

    printf("Version (Should always be one) '");
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
    printf("\n");
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
        if (parseVolumeDescriptor(&(data[i ]), (i - EMPTY_LEN)/VOLUME_DESCRIPTOR_SIZE)){
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
