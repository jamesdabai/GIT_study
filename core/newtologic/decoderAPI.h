#ifndef __DECODERAPI_H__
#define __DECODERAPI_H__

int initDecoder(int iwidth, int iheight);
void unInitDecoder();
int getResultNum();
char *getResultString();
int getResultLength();
int enableSymbology(int id, int value);
char getResultCodeID();
int DecodeImage( unsigned char *pImg,  int iwidth, int iheight);
void setDecodeScore(int score);
void setDecoderSearchTimeMax(int time);
void setDecoderAttemptTimeMax(int time);

enum SYM_ID
{
	AZTEC = 0,
	CODABAR = 1,
	CODE11 = 2,
	CODE128 = 3,
	CODE39 = 4,
	CODE49 = 5,         
	CODE93 = 6,
    COMPOSITE = 7,
    DATAMATRIX = 8,
    EAN8 = 9,
    EAN13 = 10,
    INT25 = 11,
    MAXICODE = 12,
    MICROPDF = 13,
    OCR = 14,
    PDF417 = 15,
    POSTNET = 16,
    QR = 17,
    RSS = 18,
    UPCA = 19,
    UPCE0 = 20,
    UPCE1 = 21,
    ISBT = 22,
    BPO = 23,
    CANPOST =24, 
    AUSPOST = 25,
    IATA25 = 26,
    CODABLOCK = 27,
    JAPOST = 28,
    PLANET = 29,
    DUTCHPOST = 30,
    MSI = 31,
    TLCODE39 = 32, 
    TRIOPTIC = 33,
    CODE32 = 34,
    STRT25 = 35,
    MATRIX25 = 36,
    PLESSEY = 37,   
    CHINAPOST = 38,
    KOREAPOST = 39,
    TELEPEN = 40,
    CODE16K = 41,     
    POSICODE = 42,       
    COUPONCODE = 43,
    USPS4CB = 44,
    IDTAG = 45,
    LABEL = 46,         
    GS1_128 = 47,
    HANXIN = 48,
    GRIDMATRIX = 49,    
    POSTALS = 50,		
    POSTALS1 = 51,   
    BOLOGIES = 52,
    ALL = 100,
};

#endif
