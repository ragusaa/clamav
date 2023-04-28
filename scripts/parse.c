#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <arpa/inet.h>

//https://en.wikipedia.org/wiki/ISO_9660
#define EMPTY_LEN 32768

#define VOLUME_DESCRIPTOR_SIZE 0x800
#define DATA_LEN 2041
//https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf

#define PRIMARY_VOLUME_DESCRIPTOR 1

int readData(const char* const fileName, uint8_t** data, size_t* dataLen)
{
    int ret          = -1;
    FILE* fp         = NULL;
    size_t bytesRead = 0;

    uint8_t* dRet  = NULL;
    size_t dRetLen = 0;
    *data          = NULL;
    *dataLen       = 0;

    fp = fopen(fileName, "rb");

    if (NULL == fp) {
        perror("fopen");
        goto done;
    }

    fseek(fp, 0, SEEK_END);

    dRetLen = ftell(fp);
    rewind(fp);

    dRet = malloc(dRetLen);
    if (NULL == dRet) {
        perror("malloc");
        goto done;
    }

    while (bytesRead < dRetLen) {
        int ret = fread(&(dRet[bytesRead]), 1, dRetLen - bytesRead, fp);
        if (ret > 0) {
            bytesRead += ret;
        } else {
            break;
        }
    }

    if (bytesRead != dRetLen) {
        goto done;
    }

    ret      = 0;
    *data    = dRet;
    *dataLen = dRetLen;

done:
    if (ret) {
        free(dRet);
    }
    return ret;
}

/*
 * BEA01
 * Beginning Extended Area Descriptor
 *
 * */
void handleBEA01(const uint8_t* const data)
{
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.2

    printf("%s::%d::Verify All Zeros\n", __FUNCTION__, __LINE__);
}
int handleBOOT2(const uint8_t* const data)
{

    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.4

    int ret = -1;
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
    exit(1);

    size_t idx = 0;
    if (0 != data[idx]) {
        printf("%s::%d::Invalid Structure type of '%d (0x%x)'\n", __FUNCTION__, __LINE__, data[idx], data[idx]);
        goto done;
    }

    ret = 0;
done:
    return ret;
}
void handleCD001(const uint8_t* const data)
{
    //ECMA-119
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleCDW02(const uint8_t* const data)
{
    //ECMA-168
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleNSR02(const uint8_t* const data)
{
    /*allow NSR02 in reading, but should be similar to NSR03?  Just a version number change, but don't know 
     * what else actually changed.
     */
    //section 3/9.1 of ECMA167/2
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
void handleNSR03(const uint8_t* const data)
{
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf    section 3/9.1
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}
/*
 * TEA01
 * 
 * Terminating Extencded Area Descriptor
 */
void handleTEA01(const uint8_t* const data)
{
    //https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf section 2/9.3
    printf("%s::%d::Unimplemented\n", __FUNCTION__, __LINE__);
}

/* 
 * https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf
 * section 3/7.2 */
typedef struct __attribute__((packed)) {
    uint16_t tagId;
    uint16_t descriptorVersion;
    uint8_t checksum;
    uint8_t reserved;
    uint16_t serialNumber;
    uint16_t descriptorCRC;
    uint16_t descriptorCRCLength;
    uint32_t tagLocation;
} DescriptorTag;

void copyTag(DescriptorTag* dst, DescriptorTag* src)
{
#if 0
    dst->tagId = ntohs(src->tagId);
    dst->descriptorVersion = ntohs(src->descriptorVersion);
    dst->checksum = src->checksum;
    dst->reserved = src->reserved;
    dst->serialNumber = ntohs(src->serialNumber);
    dst->descriptorCRC = ntohs(src->descriptorCRC);
    dst->descriptorCRCLength = ntohs(src->descriptorCRCLength);
    dst->tagLocation = ntohl(src->tagLocation);
#else
    printf("Byte order is Little Endian, handle byte order\n");
    memcpy(dst, src, sizeof(DescriptorTag));
#endif
}

void dumpTag(DescriptorTag* dt)
{
    printf("TagId = %d (0x%x)\n", dt->tagId, dt->tagId);
    printf("Version = %d (0x%x)\n", dt->descriptorVersion, dt->descriptorVersion);
    printf("Checksum = %d (0x%x)\n", dt->checksum, dt->checksum);
    printf("Serial Number = %d (0x%x)\n", dt->serialNumber, dt->serialNumber);

    printf("Descriptor CRC = %d (0x%x)\n", dt->descriptorCRC, dt->descriptorCRC);
    printf("Descriptor CRC Length = %d (0x%x)\n", dt->descriptorCRCLength, dt->descriptorCRCLength);
    printf("Tag Location = %d (0x%x)\n", dt->tagLocation, dt->tagLocation);
}

typedef struct __attribute__((packed)) {
    DescriptorTag tag;
    uint32_t volumeDescriptorSequenceNumber;
    uint32_t primaryVolumeDescriptorNumber;
    uint8_t volumeIdentifier[32];
    uint16_t volumeSequenceNumber;
    uint16_t interchangeLevel;
    uint16_t maxInterchangeLevel;
    uint32_t charSetList;
    uint8_t volumeSetIdentifier[128];
    uint8_t descriptorCharSet[64];
    uint8_t explanatoryCharSet[64];
    uint64_t volumeAbstract;
    uint64_t volumeCopyrightNotice;
    uint8_t applicationIdentifier[32];
    uint8_t recordingDateTime[12];
    uint8_t implementationIdentifier[32];
    uint8_t implementationUse[64];
    uint32_t predVolumeDescSequenceLocation;
    uint16_t flags;
    uint8_t reserved[22];

} PrimaryVolumeDescriptor;

/* https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf */
/* 4/3 */
typedef struct __attribute__((packed)) {
    uint32_t logicalBlockNumber;

    uint16_t partitionReferenceNumber;
} LBAddr;

//https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf
//section 4/23
typedef struct __attribute__((packed)) {
    uint32_t priorRecordedNumberOfDirectEntries;
    uint16_t strategyType;
    uint8_t strategyParameter[2]; /*described as 'bytes' in docs, so don't want to worry about byte order.*/
    uint16_t maxEntries;
    uint8_t reserved_must_be_zero;

    uint8_t fileType;

    LBAddr parentICBLocation;

    uint16_t flags;
} ICBTag;

#if 0

//https://www.ecma-international.org/wp-content/uploads/ECMA-167_3rd_edition_june_1997.pdf
//section 4/28
//page 98
typedef struct __attribute__((packed)) {
    DescriptorTag descriptorTag;

    ICBTag icbTag;

    uint32_t uid;
    uint32_t gid;
    uint32_t permissions;
    uint16_t fileLinkCount;
    uint8_t recordFormat;
    uint8_t recordDisplayAttributes;
    uint32_t recordLength;

    uint64_t infoLength;
    uint64_t logicalBlocksRecorded;
    timestamp accessDateTime;
    timestamp modDateTime;
    timestamp attrDateTime;
    uint32_t checkpoint;
    long_ad extendedAttributeICB;
    regid impId;
    uint64_t uniqueId;
    uint32_t extendedAttributeLength;
    uint32_t allocationDescriptorLength;





}FileEntry;
#endif

typedef struct __attribute__((packed)) {

    uint32_t blockNumber;

    uint16_t partitionReferenceNumber;

} lb_addr;

void copyLBAddr(lb_addr* dst, lb_addr* src)
{
#if 0
    dst->blockNumber = ntohl(src->blockNumber);
    dst->partitionReferenceNumber = ntohs(src->partitionReferenceNumber);
#else
    printf("Byte order is Little Endian, handle byte order\n");
    memcpy(dst, src, sizeof(lb_addr));
#endif
}

void dumpLBAddr(lb_addr* lba)
{
    printf("Block Number = %d (0x%x)\n", lba->blockNumber, lba->blockNumber);
    printf("Partition Reference Number = %d (0x%x)\n", lba->partitionReferenceNumber, lba->partitionReferenceNumber);
}

typedef struct __attribute__((packed)) {
    uint32_t length; //4/14.14.1.1
    /*30 least significant bits are length in bytes.
     *
     * 2 most significant bits are described in figure 4/42
     * 
     * 0 extent recorded and allocated
     * 1 extent NOT recorded but allocated
     * 2 extent NOT recorded and NOT allocated
     * 3 the extent is the next extent of allocation descriptors.
     * */

    lb_addr extentLocation; //logical block number.  (CAN be zero)

    uint8_t implementationUse[6];

} long_ad;

void copyLongAd(long_ad* dst, long_ad* src)
{
#if 0
    dst->length = ntohl(src->length);
#else
    printf("Byte order is Little Endian, handle byte order\n");
    dst->length = src->length;
#endif
    copyLBAddr(&dst->extentLocation, &src->extentLocation);

    memcpy(dst->implementationUse, src->implementationUse, sizeof(src->implementationUse));
}

void dumpLongAd(long_ad* la)
{
    int i;

    printf("Length = %d (0x%x)\n", la->length, la->length);
    dumpLBAddr(&la->extentLocation);

    printf("Implementation Use = ");
    for (i = 0; i < sizeof(la->implementationUse); i++) {
        printf("%02x ", la->implementationUse[i]);
    }
    printf("\n");
}

//4/21 page 91
typedef struct __attribute__((packed)) {
    DescriptorTag tag;

    uint16_t versionNumber;

    uint8_t characteristics;

    uint8_t fileIdentifierLength;

    long_ad icb;

    uint16_t implementationLength;

#if 0
    implementationUse;

    fileIdentifier;

    padding;
#else
    uint8_t rest[VOLUME_DESCRIPTOR_SIZE - 38];
    /*TODO: Break out the other stuff*/
#endif

} FileIdentifierDescriptor;

void copyFileIdentifierDescriptor(FileIdentifierDescriptor* dst, FileIdentifierDescriptor* src)
{

    copyTag(&dst->tag, &src->tag);

#if 0
    dst->versionNumber = ntohs(src->versionNumber);
#else
    printf("Byte order is Little Endian, handle byte order\n");
    dst->versionNumber = src->versionNumber;
#endif

    dst->characteristics      = src->characteristics;
    dst->fileIdentifierLength = src->fileIdentifierLength;
    copyLongAd(&dst->icb, &src->icb);

#if 0
    dst->implementationLength = ntohs(src->implementationLength);
#else
    printf("Byte order is Little Endian, handle byte order\n");
    dst->implementationLength = src->implementationLength;
#endif

    memcpy(dst->rest, src->rest, sizeof(src->rest));
}

/*
 * NOTES
 *
 * File Set Descriptor
 ** 
 0 16 Descriptor Tag tag (4/7.2)(Tag=256)
 16 12 Recording Date and Time timestamp (1/7.3)
 28 2 Interchange Level Uint16 (1/7.1.3)
 30 2 Maximum Interchange Level Uint16 (1/7.1.3)
 32 4 Character Set List Uint32 (1/7.1.5)
 36 4 Maximum Character Set List Uint32 (1/7.1.5)
 40 4 File Set Number Uint32 (1/7.1.5)
 44 4 File Set Descriptor Number Uint32 (1/7.1.5)
 48 64 Logical Volume Identifier Character Set charspec (1/7.2.1)
 112 128 Logical Volume Identifier dstring (1/7.2.12)
 240 64 File Set Character Set charspec (1/7.2.1)
 304 32 File Set Identifier dstring (1/7.2.12)
 336 32 Copyright File Identifier dstring (1/7.2.12)
 368 32 Abstract File Identifier dstring (1/7.2.12)
 400 16 Root Directory ICB long_ad (4/14.14.2)
 416 32 Domain Identifier regid (1/7.4)
 448 16 Next Extent long_ad (4/14.14.2)
 464 16 System Stream Directory ICB long_ad (4/14.14.2)
 480 32 Reserved #00 bytes
 *
 *
 *
 * File Identifier Descriptor
 * 0 16 Descriptor Tag tag (4/7.2)(Tag=257)
 16 2 File Version Number Uint16 (1/7.1.3)
 18 1 File Characteristics Uint8 (1/7.1.1)
 19 1 Length of File Identifier (=L_FI) Uint8 (1/7.1.1)
 20 16 ICB long_ad (4/14.14.2)
 36 2 Length of Implementation Use (=L_IU) Uint16 (1/7.1.3)
 38 L_IU Implementation Use bytes
 [L_IU+38] L_FI File Identifier d-characters (1/7.2)
 [L_FI+L_IU+38] * Padding bytes
 *
 *
 * File Entry
 0 16 Descriptor Tag tag (4/7.2)(Tag=261)
 16 20 ICB Tag icbtag (4/14.6)
 36 4 Uid Uint32 (1/7.1.5)
 40 4 Gid Uint32 (1/7.1.5)
 44 4 Permissions Uint32 (1/7.1.5)
 48 2 File Link Count Uint16 (1/7.1.3)
 50 1 Record Format Uint8 (1/7.1.1)
 51 1 Record Display Attributes Uint8 (1/7.1.1)
 52 4 Record Length Uint32 (1/7.1.5)
 56 8 Information Length Uint64 (1/7.1.7)
 64 8 Logical Blocks Recorded Uint64 (1/7.1.7)
 72 12 Access Date and Time timestamp (1/7.3)
 84 12 Modification Date and Time timestamp (1/7.3)
 96 12 Attribute Date and Time timestamp (1/7.3)
 108 4 Checkpoint Uint32 (1/7.1.5)
 112 16 Extended Attribute ICB long_ad (4/14.14.2)
 128 32 Implementation Identifier regid (1/7.4)
 160 8 Unique Id Uint64 (1/7.1.7)
 168 4 Length of Extended Attributes (=L_EA) Uint32 (1/7.1.5)
 172 4 Length of Allocation Descriptors (=L_AD) Uint32 (1/7.1.5)
 176 L_EA Extended Attribute s bytes
 [L_EA+176] L_AD Allocation descriptors bytes
 * */

/*
 *
 * DescriptorTag 16 bytes;
 *
 *
 */
/*Stream Directory has all file id descriptors.*/
void handlePrimaryVolumeDescriptor(const uint8_t* const data)
{

    /*
     * This is looking at the the File Identifier struct, I also need the File
     * Entry.
     */

    printf("%s::%d::TODO: Change the name of this function\n", __FUNCTION__, __LINE__);

    size_t i;
    printf("%s::%d::Entering\n", __FUNCTION__, __LINE__);

    FileIdentifierDescriptor fid;
    //copyTag(&(fid.tag), (FileIdentifierDescriptor *) data);
    copyFileIdentifierDescriptor(&fid, (FileIdentifierDescriptor*)data);

    if (257 != fid.tag.tagId) {
        printf("NOT the correct spot\n");
        goto done;
    }

#if 1
    for (i = 0; i < VOLUME_DESCRIPTOR_SIZE; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

    printf("sizeof (prim) = %lu\n", sizeof(PrimaryVolumeDescriptor));
#endif

    dumpTag(&fid.tag);
    printf("\n");
    printf("Version Number = %d (0x%x)\n", fid.versionNumber, fid.versionNumber);
    printf("Characteristics = %d (0x%x)\n", fid.characteristics, fid.characteristics);
    /*existence bit*/
    if (1 & fid.characteristics) {
        printf("\tFile DOES NOT need to be made known to the user\n");
    } else {
        printf("\tFile DOES need to be made known to the user\n");
    }

    /*directory bit*/
    if ((1 << 1) & fid.characteristics) {
        printf("\tFile IS a directory\n");
    } else {
        printf("\tFile IS NOT a directory\n");
    }

    /*deleted bit*/
    if ((1 << 2) & fid.characteristics) {
        printf("\tFile HAS been deleted\n");
    } else {
        printf("\tFile HAS NOT been deleted\n");
    }

    /*parent bit*/
    if ((1 << 3) & fid.characteristics) {
        printf("\tICB refers to the PARENT directory of the directory that this file is recorded in (Length of File Identifier below SHOULD be zero)\n");
    } else {
        printf("\tICB refers to the ICB of this file.\n");
    }

    /*metadata bit */
    if ((1 << 4) & fid.characteristics) {
        printf("\tFile contains implementation use data.\n");
    } else {
        printf("\tFile is either NOT in a stream directory, or in a stream directory that contains user data.\n");
    }

    printf("File Identifier Length = %d (0x%x)\n", fid.fileIdentifierLength, fid.fileIdentifierLength);

    dumpLongAd(&fid.icb); //Address of ICB.  Look at the address to actually understand the ICB.
    printf("\n");

    printf("Implementation Length = %d (0x%x)\n", fid.implementationLength, fid.implementationLength);

    for (i = 0; i < sizeof(fid.rest); i++) {
        printf("%02x ", fid.rest[i]);
    }
    printf("\n");

    printf("Padding is explained in 4/23 14.4.9\n");
    /*
     * L_FI = fileIdentifierLength
     * L_IU = implementationLength
     * 4 x ip((L_FI + L_IU + 38 + 3)/4) - (L_FI + L_IU + 38) 
     * ip = 'integer part'
     * */
    uint32_t temp = fid.fileIdentifierLength + fid.implementationLength + 38;
    printf("Padding should be '%d' bytes of zeros\n", (((temp + 3) / 4) * 4) - temp);
    //27

done:

    printf("%s::%d::Leaving\n", __FUNCTION__, __LINE__);
}

int parseVolumeDescriptor(const uint8_t* const data, size_t sectorNumber)
{
    size_t parseIdx = 0;
    size_t i        = 0;
    int ret         = -1;
    int allZeros    = 1;
    int terminator  = 0;
    char identifier[6];
    uint8_t volumeDescriptorType = data[parseIdx];

    /**/
    if (PRIMARY_VOLUME_DESCRIPTOR == volumeDescriptorType) {
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
    switch (volumeDescriptorType) {
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
            printf("Invalid Volume Descriptor '%d (0x%x)\n", volumeDescriptorType, volumeDescriptorType);
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
    for (i = 0; i < 5; i++) {
        identifier[i] = data[parseIdx++];
    }
    identifier[5] = 0;
    printf("Identifier '%s'\n", identifier);
    if (0 == strcmp("BEA01", identifier)) {
        handleBEA01(data);
    } else if (0 == strcmp("BOOT2", identifier)) {
        handleBOOT2(data);
    } else if (0 == strcmp("CD001", identifier)) {
        handleCD001(data);
    } else if (0 == strcmp("CDW02", identifier)) {
        handleCDW02(data);
    } else if (0 == strcmp("NSR02", identifier)) {
        handleNSR02(data);
    } else if (0 == strcmp("NSR03", identifier)) {
        handleNSR03(data);
    } else if (0 == strcmp("TEA01", identifier)) {
        handleTEA01(data);
    }

    printf("Version (Should always be one) '");
    printf("%d'\n", data[parseIdx]);
    parseIdx += 1;

    printf("Data '");
    allZeros = 1;
    for (i = 0; i < DATA_LEN; i++) {
        if (0 != data[parseIdx + i]) {
            allZeros = 0;
            break;
        }
    }
    if (allZeros) {
        printf("All zeros");
        parseIdx += DATA_LEN;
    } else {

        for (i = 0; i < DATA_LEN; i++) {
            printf("%02x ", data[parseIdx++]);
        }
    }
    printf("'\n");

    if (0 == terminator) {
        ret = 0;
    }
done:
    printf("\n");
    return ret;
}

/*
 * charsetType can be
    0 The CS0 coded character set (1/7.2.2).
    1 The CS1 coded character set (1/7.2.3).
    2 The CS2 coded character set (1/7.2.4).
    3 The CS3 coded character set (1/7.2.5).
    4 The CS4 coded character set (1/7.2.6).
    5 The CS5 coded character set (1/7.2.7).
    6 The CS6 coded character set (1/7.2.8).
    7 The CS7 coded character set (1/7.2.9).
    8 The CS8 coded character set (1/7.2.10).
    9-255 Reserved for future standardisation.
 *
 */
typedef struct {
    uint8_t charSetType;

    uint8_t charSetInfo[63];
} charspec;

typedef struct {
    uint16_t typeTimeZone; /*
                              0     Coordinated UTC
                              1     Local Time
                              2     Up to agreement between originator and recipient
                              3 - 15    Reserved
                            */
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t centiseconds;
    uint8_t hundredsMicroSeconds;
    uint8_t microseconds;
} timestamp;

typedef struct {
    uint8_t flags;
    /* 
     * 1/7.4
     * characteristics 
     * bit 0    dirty: If regid has been modified and might not be valid, set to * 1.  Otherwise, 0
     * bit 1    protected: If 1, this regid cannot be modified
     * bit 2-7  reserved
     */
    uint8_t identifier[23];
    /* 
     * If first byte is 0x2b, then this is covered by ECMA-168 (this spec)
     * If first byte is 0x2d, then this is not registered
     */
    uint8_t identifierSuffix[8];
} regid;



typedef struct {
    DescriptorTag tag;
    timestamp recordingDateTime;

    uint16_t interchangeLevel;

    uint16_t maxInterchangeLevel;
    uint32_t characterSetList;
    uint32_t maxCharacterSetList;

    uint32_t fileSetNumber;
    uint32_t fileSetDescriptorNumber;

    charspec logicalVolumeIdentifierCharSet;
    uint8_t logicalVolumeIdentifier[128];
    charspec fileSetCharSet;
    uint8_t fileSetIdentifier[32];

    uint8_t copyrightIdentifier[32];
    uint8_t abstractIdentifier[32];
    long_ad rootDirectoryICB;

    regid domainIdentifier;

    long_ad nextExtent;
    long_ad systemStreamDirectoryICB;
    uint8_t reserved[32];

} FileSetDescriptor;


int parseFileSetDescriptor(const uint8_t * const data, size_t sectorNumber){
    int ret = -1;

    FileSetDescriptor fsd;

    memcpy(&fsd, data, sizeof(fsd));

        printf("%s::%d::", "parseVolumeDescriptor2", __LINE__);
    //for (size_t i = 0; i < VOLUME_DESCRIPTOR_SIZE ; i++)
    for (size_t i = 0; i < 512 ; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");

    printf("%s::%d::'%s'\n", "parseVolumeDescriptor2", __LINE__, (char *) fsd.fileSetIdentifier);

    dumpTag(&(fsd.tag));

    





    ret = 0;
done:
    return ret;
}

typedef struct __attribute__((packed)) {

    DescriptorTag tag;

    uint16_t versionNumber;

    uint8_t characteristics;

    uint8_t fileIdentifierLength;

    long_ad icb;

    uint16_t implementationLength;

    uint8_t rest[1];

}FileIdentifierDescriptor2;

#define FILE_IDENTIFIER_DESCRIPTOR2_SIZE_KNOWN (sizeof(FileIdentifierDescriptor2) - 1)

/*Print characteristics for File Identifier Descriptor*/
static void printCharactersitics(FileIdentifierDescriptor2 * fid){

    /*existence bit*/
    if (1 & fid->characteristics) {
        printf("\tFile DOES NOT need to be made known to the user\n");
    } else {
        printf("\tFile DOES need to be made known to the user\n");
    }

    /*directory bit*/
    if ((1 << 1) & fid->characteristics) {
        printf("\tFile IS a directory\n");
    } else {
        printf("\tFile IS NOT a directory\n");
    }

    /*deleted bit*/
    if ((1 << 2) & fid->characteristics) {
        printf("\tFile HAS been deleted\n");
    } else {
        printf("\tFile HAS NOT been deleted\n");
    }

    /*parent bit*/
    if ((1 << 3) & fid->characteristics) {
        printf("\tICB refers to the PARENT directory of the directory that this file is recorded in (Length of File Identifier below SHOULD be zero)\n");
    } else {
        printf("\tICB refers to the ICB of this file.\n");
    }

    /*metadata bit */
    if ((1 << 4) & fid->characteristics) {
        printf("\tFile contains implementation use data.\n");
    } else {
        printf("\tFile is either NOT in a stream directory, or in a stream directory that contains user data.\n");
    }

}

/*Section 14.4.9 of https:... */
static uint32_t getPaddingLength(const FileIdentifierDescriptor2 * const fid){
    uint32_t ret = 0;
    uint32_t tmp = fid->implementationLength + fid->fileIdentifierLength + 38;
    ret = tmp + 3;
    ret = ret / 4;

    ret = ret * 4;
    ret = ret - tmp;

    return ret;
}

static void printDCharacters(const char * const func, const uint32_t line, const uint8_t * const start, const uint32_t len){
    /*According to section 1/7.2, the first byte is the character set type.
     * We are just printing the string to the screen, so we'll skip that
     * for now, and look at it later.
     */
    size_t i;

    printf("%s::%d::", func, line);
    for (i = 1; i < len; i++){
        if (start[i]){
        printf("%c", start[i]);
        }
    }
    printf("\n");
    
}

static int parseFileIdentifierDescriptor2(const uint8_t * const data, size_t sectorNumber){

    int ret = -1;

    size_t i;
    FileIdentifierDescriptor2 * fid = (FileIdentifierDescriptor2*) data;

    printf("%s::%d\n", __FUNCTION__, __LINE__);

    if (257 != fid->tag.tagId){
        printf("%s::%d::Shouldn't be here\n", __FUNCTION__, __LINE__);
        goto done;
    }

    printf("%s::%d\n", __FUNCTION__, __LINE__);
    while (257 == fid->tag.tagId){

        printf("\n%s::%d::Entering loop\n", __FUNCTION__, __LINE__);


        printCharactersitics(fid);
        printf("%s::%d::Length of file identifier = %d (0x%x)\n", __FUNCTION__, __LINE__, fid->fileIdentifierLength, fid->fileIdentifierLength);

        printf("%s::%d::Dumping ICB\n", __FUNCTION__, __LINE__);
    dumpLongAd(&fid->icb); //Address of ICB.  Look at the address to actually understand the ICB.
    printf("%s::%d::DONE Dumping ICB\n", __FUNCTION__, __LINE__);

    printf("%s::%d::Implementation Length = %d (0x%x)\n", __FUNCTION__, __LINE__, fid->implementationLength, fid->implementationLength);


    if (fid->fileIdentifierLength){
        printDCharacters(__FUNCTION__, __LINE__, &(fid->rest[fid->implementationLength]), fid->fileIdentifierLength);
    }







    uint32_t paddingLength = getPaddingLength(fid);

    printf("%s::%d::Padding length = '%d'\n", __FUNCTION__, __LINE__, paddingLength);




    fid = (FileIdentifierDescriptor2*) (((uint8_t*) fid) + FILE_IDENTIFIER_DESCRIPTOR2_SIZE_KNOWN + fid->implementationLength + fid->fileIdentifierLength + paddingLength);


#if 0
        fid += sizeof(FileIdentifierDescriptor2 );
#else

#endif






    }

        printf("%s::%d::Exiting\n", __FUNCTION__, __LINE__); exit(1);

    printf("%s::%d\n", __FUNCTION__, __LINE__);





    ret = 0;
done:
    return ret;
}

int parseVolumeDescriptor2(const uint8_t* const data, size_t sectorNumber)
{
    uint16_t tag;

    printf("%s::%d::ENTERING\n", __FUNCTION__, __LINE__);

    memcpy(&tag, data, 2);
    //tag = ntohs(tag);

    printf("%s::%d, tag = %d (0x%x)\n", __FUNCTION__, __LINE__, tag, tag);


    char* desc = NULL;
    switch (tag) {
        case 8: {
            desc = "Terminating Descriptor";
            break;
        }
        case 256: {
            desc = "File Set Descriptor";
            parseFileSetDescriptor(data, sectorNumber);
            break;
        }
        case 257: {
            desc = "File Identifier Descriptor";
            parseFileIdentifierDescriptor2(data, sectorNumber);
            //handlePrimaryVolumeDescriptor(data);
            break;
        }
        case 258: {
            desc = "Allocation Extent Descriptor";
            break;
        }
        case 259: {
            desc = "Indirect Entry";
            break;
        }
        case 260: {
            desc = "Terminal Entry";
            break;
        }
        case 261: {
            desc = "File Entry";
            break;
        }
        case 262: {
            desc = "Extended Attribute Header Descriptor";
            break;
        }
        case 263: {
            desc = "Unallocated Space Entry";
            break;
        }
        case 264: {
            desc = "Space Bitmap Descriptor";
            break;
        }
        case 265: {
            desc = "Partition Integrity Entry";
            break;
        }
        case 266: {
            desc = "Extended File Entry";
            break;
        }
        default:
            break;
    }

    if (desc){
        printf("%s::%d::%s\n", __FUNCTION__, __LINE__, desc);
    }

    if (257 == tag) {
        printf("%s::%d::", __FUNCTION__, __LINE__);
        for (size_t i = 0; i < VOLUME_DESCRIPTOR_SIZE; i++) {
            printf("%02x ", data[i]);
        }
        printf("\n");
    }
    printf("%s::%d::LEAVING\n", __FUNCTION__, __LINE__);

    return 0;
}

int parseFile(const char* const fileName)
{

    uint8_t* data  = NULL;
    size_t dataLen = 0;
    int ret        = -1;
    size_t i, parseIdx = EMPTY_LEN;

    if (readData(fileName, &data, &dataLen)) {
        printf("%s::%d::Unable to read file\n", __FUNCTION__, __LINE__);
        goto done;
    }
    if (dataLen < EMPTY_LEN) {
        printf("%s::%d::ERROR\n", __FUNCTION__, __LINE__);
        goto done;
    }

    printf("dataLen = %ld\n", dataLen);

#if 0
    int andy = 0;
#endif
    for (i = EMPTY_LEN; i < dataLen; i += VOLUME_DESCRIPTOR_SIZE) {
        if (parseVolumeDescriptor(&(data[i]), (i - EMPTY_LEN) / VOLUME_DESCRIPTOR_SIZE)) {
            goto done;
        }

        if (parseVolumeDescriptor2(&(data[i]), (i - EMPTY_LEN) / VOLUME_DESCRIPTOR_SIZE)) {
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
    if (data) {
        free(data);
    }
    return ret;
}

int main(int argc, char** argv)
{

    int i;
    for (i = 1; i < argc; i++) {
        parseFile(argv[i]);
    }

    return 0;
}
