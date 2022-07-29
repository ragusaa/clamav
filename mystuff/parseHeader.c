#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FREE(var) if (NULL != var) { free(var); var = NULL; }

size_t getFileSize(FILE * fp){
    size_t fileSize = 0;

    if (fseek(fp, 0, SEEK_END)){
        perror("fseek");
        goto done;
    }

    fileSize = ftell(fp);

    if (fseek(fp, 0, SEEK_SET)){
        perror("fseek");
        goto done;
    }

done:
    return fileSize;

}

int readWholeFile(const char * const fileName, uint8_t ** buffer, size_t * bufferSize){

    int ret = -1;
    FILE * fp = NULL;
    size_t bytesRead = 0;
    size_t fileSize = 0;
    uint8_t * buf = NULL;

    *buffer = NULL;
    *bufferSize = 0;

    fp = fopen(fileName, "rb");
    if (NULL == fp){
        perror("fopen");
        goto done;
    }

    if (0 == (fileSize = getFileSize(fp))){
        goto done;
    }

    buf = malloc (fileSize);
    if (NULL == buf){
        perror("malloc");
        goto done;
    }

    while (bytesRead < fileSize){
        int ret = fread(&(buf[bytesRead]), 1, fileSize - bytesRead, fp);
        if (ret){
            bytesRead += ret;
        } else {
            break;
        }
    }

    if (bytesRead != fileSize){
        goto done;
    }

    ret = 0;
    *buffer = buf;
    *bufferSize = fileSize;
    buf = NULL;
done:
    FREE(buf);
    return ret;
}

static void printWide(uint8_t * buffer, uint32_t max){
    size_t idx;
    for (idx = 0; !((buffer[idx] == 0) && (buffer[idx+1] == 0)); idx+=2){
        printf("%c", buffer[idx]);
        if (idx == (max - 1)){
            break;
        }
    }

}




//https://winprotocoldoc.blob.core.windows.net/productionwindowsarchives/MS-CFB/%5bMS-CFB%5d.pdf
const uint8_t EXPECTED_SIGNATURE[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
#define EXPECTED_MINOR 0x003e
#define EXPECTED_BYTEORDER 0xFFFE
typedef struct __attribute__((packed)) {
    uint8_t signature[8]; // 0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1
    uint8_t clsid[16];    //all zeros
    uint16_t minorVersion;
    uint16_t majorVersion; 
    uint16_t byteOrder;
    uint16_t sectorShift;
    uint16_t miniSectorShift;
    uint8_t reservedZeros[6];    //all zeros
    uint32_t numDirectorySectors;
    uint32_t FATSectors;
    uint32_t firstDirectorySectorLocation; //sector number, not offset
    uint32_t transactionSignatureNumber; 
    uint32_t miniStreamCutoffSize; //MUST BE 0x00001000 
    uint32_t firstMiniFATSectorLocation;
    uint32_t numMiniFATSectors;

    uint32_t firstDIFATSectorLocation;
    uint32_t numDIFATSectors;

    uint32_t DIFAT[109];

} CompoundFileHeader;

#define ERRDONE { printf("%s::%d::Invalid byte\n", __FUNCTION__, __LINE__); goto done; }
#define DEBUG32(__var__) printf("%s::%d::%s = %x\n", __FUNCTION__, __LINE__, #__var__, __var__)
#define VERIFY_ALL_ZEROS(val, size) { size_t idx; for (idx = 0; idx < size; idx++){ if (0 != val[idx]) { goto done; }}}
#define VERIFY_VALUE(val, expected) { if (expected != val) { ERRDONE    }}
#define EXPECTED_MINI_STREAM_CUTOFF_SIZE 0x00001000

#define FREESECT 0xffffffff
#define ENDOFCHAIN 0xfffffffe
#define FATSECT 0xfffffffd
#define DIFSECT 0xfffffffc


typedef struct __attribute__((packed)) {
    uint8_t name[64];
    uint16_t nameLength;
    uint8_t objectType;
    uint8_t color;
    uint32_t leftSibling;
    uint32_t rightSibling;
    uint32_t child;

    uint8_t clsid[16];      //GUID
    uint32_t state;
    uint64_t creationTime;
    uint64_t modifiedTime;

    uint32_t startingSector;
    uint64_t streamSize;

} DirectoryEntry;

static void printDirectoryEntry(DirectoryEntry * de){

    size_t idx;
    printf("\n");

    printf("Printing Name: '");
    printWide(de->name, sizeof(de->name));
    printf("'\n");
    printf("Name length = 0x%x\n", de->nameLength);
    printf("Object type = 0x%x\n", de->objectType);       //0 unknown
                                                        //1 storage object
                                                        //2 stream object
                                                        //5 root storage object
    printf("Color flag = 0x%x\n", de->color);             //0 red
                                                        //1 black
    printf("Left Sibling ID = 0x%x\n", de->leftSibling);
    printf("Right Sibling ID = 0x%x\n", de->rightSibling);
    printf("Child ID = 0x%x\n", de->child);

    printf("clsid = '");
    for (idx = 0; idx < sizeof(de->clsid); idx++){
        printf("%02x ", de->clsid[idx]);
    }
    printf("'\n");
    printf("State = 0x%x\n", de->state);
    printf("Creation Time = %zx\n", de->creationTime);
    printf("Modified Time = %zx\n", de->modifiedTime);


    printf("Starting Sector Location = 0x%x\n", de->startingSector);
    printf("Stream Size = %zx\n", de->streamSize);
    printf("\n");



}


static void printFAT(uint32_t * fatPointer, size_t numEntries, int breakFirstEmpty){
    size_t idx;
    for (idx = 0; idx < numEntries; idx++){
        uint32_t curr = fatPointer[idx];
        if (breakFirstEmpty && (FREESECT == curr)) {
            break;
        }
        switch (curr){
            case FREESECT:
                printf("%zx::FREESECT\n", idx);
                break;
            case ENDOFCHAIN:
                printf("%zx::ENDOFCHAIN\n", idx);
                break;
            case FATSECT:
                printf("%zx::FATSECT\n", idx);
                break;
            case DIFSECT:
                printf("%zx::DIFSECT\n", idx);
                break;
            default:
                printf("%zx::%x\n", idx, curr);
                break;
        }
    }
}

//https://docs.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/dca653b5-b93b-48df-8e1e-0fb9e1c83b0f
typedef struct __attribute__((packed)) {

    uint32_t flags;
    uint32_t sizeExtra; //must be 0
uint32_t algorithmID;
uint32_t algorithmIDHash;
uint32_t keySize;
uint32_t providerType;
uint32_t reserved1;
uint32_t reserved2; //MUST be 0

uint8_t cspName[512 - 56]; //really a wide char

} EncryptionInfo;

typedef struct __attribute__((packed)) {
    uint16_t versionMajor;
    uint16_t versionMinor;
    uint32_t flags; //https://docs.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/200a3d61-1ab4-4402-ae11-0290b28ab9cb

    uint32_t size;

    EncryptionInfo encryptionInfo;

} EncryptionInfoStreamStandard;
int validateEncryptionHeader(uint8_t * buffer){

    size_t idx;
    EncryptionInfoStreamStandard * headerPtr = (EncryptionInfoStreamStandard *) buffer;
    int ret = -1;
    printf("Mini Stream Sector = '%p'\n", buffer);
    for (idx = 0; idx < 24; idx++) {
        printf("%02x ", buffer[idx]);
    }
    printf("\n");

    printf("Major Version   = 0x%x\n", headerPtr->versionMajor);
    printf("Minor Version   = 0x%x\n", headerPtr->versionMinor);
    printf("Flags           = 0x%x\n", headerPtr->flags);

    /*Bit 0 and 1 must be 0*/
    if (1 & headerPtr->flags){
        printf("Invalid, first bit must be 0\n");
        goto done;
    }

    if ((1 << 1) & headerPtr->flags){
        printf("Invalid, first bit must be 0\n");
        goto done;
    }

#define SE_HEADER_FCRYPTOAPI (1 << 2)
#define SE_HEADER_FEXTERNAL (1 << 4)
#define SE_HEADER_FDOCPROPS (1 << 3)

    //https://docs.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/200a3d61-1ab4-4402-ae11-0290b28ab9cb
    //TODO: Figure out what fdocprops are really supposed to be.  
    if ((SE_HEADER_FDOCPROPS & headerPtr->flags)){
#if 1
        printf("Unsupported document properties encrypted\n");
        exit(11);
#else
        printf("WARNING: Verify that this is set to the correct values.\n");
#endif
    }

    if ((SE_HEADER_FEXTERNAL & headerPtr->flags) && 
            (SE_HEADER_FEXTERNAL != headerPtr->flags)){
        printf("Invalid fExternal flags.  If fExternal bit is set, nothing else can be\n");
        goto done;
    }

#define SE_HEADER_FAES (1 << 5)
    if (SE_HEADER_FAES & headerPtr->flags){
        if (!(SE_HEADER_FCRYPTOAPI & headerPtr->flags)){
            printf("Invalid combo of fAES and fCryptoApi flags\n");
            goto done;
        }

        printf("Flags: AES\n");
    }

    printf("Size            = 0x%x\n", headerPtr->size);

    if (headerPtr->flags != headerPtr->encryptionInfo.flags){
        printf("Flags must match\n");
        goto done;
    }

    if (0 != headerPtr->encryptionInfo.sizeExtra){
        printf("Size Extra must be 0\n");
        goto done;
    }

#define SE_HEADER_EI_AES128 0x0000660e
#define SE_HEADER_EI_AES192 0x0000660f
#define SE_HEADER_EI_AES256 0x00006610

#define SE_HEADER_EI_RC4    0x00006801

    switch (headerPtr->encryptionInfo.algorithmID){
        case SE_HEADER_EI_AES128:
            break;
        case SE_HEADER_EI_AES192:
            break;
        case SE_HEADER_EI_AES256:
            break;
        case SE_HEADER_EI_RC4:
            break;
        default:
            printf("Invalid Algorithm ID: 0x%x\n", headerPtr->encryptionInfo.algorithmID);
            goto done;
    }

#define SE_HEADER_EI_SHA1 0x00008004
    if (SE_HEADER_EI_SHA1 != headerPtr->encryptionInfo.algorithmIDHash){
        printf("Invalid Algorithm ID Hash: 0x%x\n", headerPtr->encryptionInfo.algorithmIDHash);
        goto done;
    }

#define SE_HEADER_EI_AES128_KEYSIZE 0x00000080
#define SE_HEADER_EI_AES192_KEYSIZE 0x000000c0
#define SE_HEADER_EI_AES256_KEYSIZE 0x00000100

    switch (headerPtr->encryptionInfo.keySize){
        case SE_HEADER_EI_AES128_KEYSIZE:
            break;
        case SE_HEADER_EI_AES192_KEYSIZE:
            break;
        case SE_HEADER_EI_AES256_KEYSIZE:
            break;
        default:
            printf("Invalid key size: 0x%x\n", headerPtr->encryptionInfo.keySize);
            goto done;
    }
    printf("KeySize = 0x%x\n", headerPtr->encryptionInfo.keySize);

#define SE_HEADER_EI_AES_PROVIDERTYPE  0x00000018
    if (SE_HEADER_EI_AES_PROVIDERTYPE != headerPtr->encryptionInfo.providerType){
        printf("WARNING: Provider Type should be '0x%x', is '0x%x'\n", 
                SE_HEADER_EI_AES_PROVIDERTYPE,  headerPtr->encryptionInfo.providerType);
    }

    printf("Reserved1:  0x%x\n", headerPtr->encryptionInfo.reserved1);

    if (0 != headerPtr->encryptionInfo.reserved2){
        printf("Reserved 2 must be zero, is 0x%x\n", headerPtr->encryptionInfo.reserved2);
        goto done;
    }

    printf("CPSPName: ");
    //Expected either 
    //'Microsoft Enhanced RSA and AES Cryptographic Provider'
    //or
    //'Microsoft Enhanced RSA and AES Cryptographic Provider (Prototype)'
    printWide(headerPtr->encryptionInfo.cspName, sizeof( headerPtr->encryptionInfo.cspName));
    printf("\n");

    ret = 0;
done:
    return ret;

}

typedef struct __attribute__((packed)) {
    uint64_t streamSize;

    //Not actually 1, but stream size
    uint8_t data[1];

} EncryptedPackage;

static int getEncryptedBlob(CompoundFileHeader * header, DirectoryEntry * de, EncryptionInfoStreamStandard * encStream){
    int ret = -1;
    uint32_t sector = de->startingSector + 1;
    uint32_t offset = sector * (1 << header->sectorShift);

    /* Since header is just a pointer to the beginning of the file, we can
     * compute the new offset and cast it to the correct offset, and get
     * the data without a bunch of copies.*/
    EncryptedPackage * encryptedPackage = (EncryptedPackage *) &(((uint8_t *) header)[offset]);

    printf("Starting sector = %x\n", sector);
    printf("Starting offset = %x\n", offset);
    printf("Number of bytes = %zx\n", encryptedPackage->streamSize);

    //Stream size is different from what is in Directory stream, potentially due
    //to block size
    //https://docs.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/b60c8b35-2db2-4409-8710-59d88a793f83

    FILE * fp = fopen("andy.data", "wb");
    size_t bytesWritten = 0;
    size_t bytesToWrite = encryptedPackage->streamSize;

    bytesToWrite = de->streamSize - sizeof(encryptedPackage->streamSize);


    while (bytesWritten < bytesToWrite){
        int ret = fwrite(&(encryptedPackage->data[bytesWritten]), 1, 
                bytesToWrite - bytesWritten, fp);
        if (ret){
            bytesWritten += ret;
        } else {
            break;
        }
    }
    if (bytesWritten != bytesToWrite) {
        printf("ERROR writing data\n");
        goto done;
    }

#if 0
    This can be decrypted with the following command.  It is odd that it also succeeds using aes-256-=... 
    openssl aes-128-ecb -pbkdf2 -d -pass file:../password -in andy.data -out a.128 -nopad -nosalt -md sha1
#endif





    ret = 0;
done:
    fclose(fp);
    return ret;
}


int parse_header(const char * const fileName){
    int ret = -1;

    uint8_t * buffer = NULL;
    size_t bufferSize = 0;
    size_t idx = 0;
    size_t loop = 0;
    CompoundFileHeader * header = NULL;

    if (readWholeFile(fileName, &buffer, &bufferSize)){
        goto done;
    }

    if (bufferSize < sizeof(CompoundFileHeader)){
        ERRDONE
    }

    header = (CompoundFileHeader * )buffer;

    if (memcmp(header->signature, EXPECTED_SIGNATURE, sizeof(EXPECTED_SIGNATURE))){
        ERRDONE
    }

    VERIFY_ALL_ZEROS(header->clsid, sizeof(header->clsid));

    if (EXPECTED_BYTEORDER != header->byteOrder){
        ERRDONE
    }

    if (EXPECTED_MINOR != header->minorVersion){
        printf("WARNING::THIS IS A SHOUND, NOT A MUST, SO DON'T ERROR\n");
    }

    if (0x0003 == header->majorVersion){
        if (0x0009 != header->sectorShift){
            fprintf(stderr, "ERROR: Invalid major version + sector shift combo\n");
            goto done;
        }
        if (0 != header->numDirectorySectors){
            fprintf(stderr, "ERROR: Invalid major version + directory sector combo\n");
        }
    } else if (0x0004 == header->majorVersion){
        if (0x000c != header->sectorShift){
            fprintf(stderr, "ERROR: Invalid major version + sector shift combo\n");
            goto done;
        }
    } else {
        fprintf(stderr, "ERROR: Invalid major version\n");
    }

    VERIFY_VALUE(header->miniSectorShift, 0x0006);

    VERIFY_ALL_ZEROS(header->reservedZeros, sizeof(header->reservedZeros));

    VERIFY_VALUE(header->miniStreamCutoffSize, EXPECTED_MINI_STREAM_CUTOFF_SIZE);


    printf("Sector 0\n");

    printf("Sector shift %d\n", header->sectorShift);

    printf("Directory sectors %d\n", header->numDirectorySectors);
    printf("FAT sectors %d\n", header->FATSectors);
    printf("First directory sector location %d\n", header->firstDirectorySectorLocation);
    printf("Transaction signature number %d\n", header->transactionSignatureNumber);
    printf("First mini FAT sector location %d\n", header->firstMiniFATSectorLocation);
    printf("Num mini FAT sectors %d\n", header->numMiniFATSectors);

    printf("First DIFAT sector location %d (0x%x)\n", header->firstDIFATSectorLocation, header->firstDIFATSectorLocation);
    printf("Num DIFAT sectors %d\n", header->numDIFATSectors);


    printf("Major Version %d\n", header->majorVersion);


    printf("Sector 1\n");
    printf("PRINTING DIFAT\n");
    printFAT(header->DIFAT, 109, 1);
    printf("DONE PRINTING DIFAT\n");


    {
    uint32_t * fatPointer = (uint32_t*) (buffer + sizeof(*header));

#if 0
        printf("buffer = %p\n", buffer);
        printf("sizeof *header = %lx\n", sizeof(CompoundFileHeader));
        printf("fatPointer = %p\n", fatPointer);
#endif

    printf("Sector 2\n");
    printf("PRINTING FAT\n");
    printFAT(fatPointer, 128, 0);
    printf("END of FAT\n");

    DirectoryEntry * de = (DirectoryEntry*) &(buffer[512 * 2]); //TODO: section size based on version and looked up in FAT.

    printf("Root directory entry = '%p'\n", de);

    //4 because that is how many DirectoryEntry's will fit in a sector
    for (idx = 0; idx < 4; idx++){
    //    printf("Sector '%ld'\n", (((uint8_t*) &(de[idx]))-buffer)/512);
        printDirectoryEntry(&(de[idx]));
    }

    fatPointer = (uint32_t*) (buffer + (3 * sizeof(*header)));
    printf("Sector 3\n");
    printf("mini FAT = %p\n", fatPointer);
    printf("Printing MiniFAT\n");
    printFAT(fatPointer, 128, 1);
    printf("Done printing MiniFAT\n");



    /*Based on Root Entry, the first offset is sector 3 ((sector + 1) * 512)*/
    //TODO: Actually pull this from the RootEntry.  Still don't know what to
    //do if the encrypted payload is larger than 4k ( or whatever the
    //threshold is that makes it NOT in the mini stream)
    if (validateEncryptionHeader(&(buffer[4 * sizeof(*header)]))){
        goto done;
    }



    getEncryptedBlob(header, &de[2], (EncryptionInfoStreamStandard *) &(buffer[4 * sizeof(*header)]));






    }


    printf("Looking good, sizeof header = %lu\n", sizeof(*header));

    printf("Size of DirectoryEntry = %lu\n", sizeof(DirectoryEntry));


    ret = 0;
done:

    FREE(buffer);
    return ret;
}

int main(int argc, char ** argv){

    const char * inFile = "documents.xlsx";
    if (1 < argc){
        inFile = argv[1];
    }

    parse_header(inFile);

    return 0;
}
