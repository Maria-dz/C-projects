#include <stdio.h>
#include <stdlib.h>

int moveBytes(unsigned char *bytes, int inBufferCount) {
    bytes[0] = bytes[1];
    bytes[1] = bytes[2];
    inBufferCount--;
    return inBufferCount;
}

int check0(unsigned char b) {
    return ((b | 0x7F) == 0x7F);
}

int check110(unsigned char b) {
    return ((b & 0xE0) == 0xC0);
}

int check10(unsigned char b) {
    return ((b >> 6) == 0x2);
}

int check111(unsigned char b) {
    return ((b & 0xE0) == 0xE0);
}

void printing(int BOM, unsigned char A, unsigned char B, FILE *out) {
    unsigned short converted;
    if (BOM){
        converted = (B << 8) | A;
    }
    else converted = (A << 8) | B;
    fwrite(&converted, sizeof converted, 1, out);
}

void printError(int pos){
    fprintf(stderr, "Error with the %d symbol", pos);
}

int convertWhen0( unsigned char *bytes, int inBufferCount, int BOM, FILE *out, unsigned char byteA, unsigned char byteB) {
    byteA = 0;
    byteB = bytes[0];
    printing(BOM, byteA, byteB, out);
    inBufferCount = moveBytes(bytes, inBufferCount);
    return inBufferCount;
}
int convertWhen110( unsigned char *bytes, int inBufferCount, int BOM,  FILE *out, unsigned char byteA, unsigned char byteB) {
    byteA = (bytes[0] & 0x1C) >> 2;
    byteB = ((bytes[0] & 0x3) << 6) | (bytes[1] & 0x3F);
    printing(BOM, byteA, byteB, out);
    bytes[0] = bytes[2];
    inBufferCount -= 2;
    return inBufferCount;
}
void convertWhen111( unsigned char *bytes, int BOM, FILE *out, unsigned char byteA, unsigned char byteB) {
    byteA = ((bytes[0] & 0xF) << 4) | ((bytes[1] & 0x3C) >> 2);
    byteB = ((bytes[1] & 0x3) << 6) | (bytes[2]& 0x3F);
    printing(BOM, byteA, byteB, out);
}

int converting(unsigned char *bytes, int inBufferCount, int inFilePosition, int BOM, FILE *out) {
    int position = inFilePosition - inBufferCount + 3;

    unsigned char byteA, byteB;

    if (inBufferCount == 0) return 0;
    if (check0(bytes[0])) {   //  starts with 0
        inBufferCount = convertWhen0(bytes, inBufferCount, BOM, out, byteA, byteB);
        return converting(bytes, inBufferCount, position, BOM, out);
    }
    else if ( check110(bytes[0])) { //starts with 110
        if (inBufferCount >= 2) {
            if (check10(bytes[1])) {//starts with 10
                inBufferCount = convertWhen110(bytes, inBufferCount, BOM, out, byteA, byteB);
                return converting(bytes, inBufferCount, position, BOM, out);
            }
            else {
                printError(position - 2);
                inBufferCount = moveBytes(bytes, inBufferCount);
                return converting(bytes, inBufferCount, position, BOM, out);
            }
        }
        else return 1;
    }
    else if (check111(bytes[0])){
        if (inBufferCount == 3) {
            if (check10(bytes[1])) {
                if (check10(bytes[2])) {
                    convertWhen111(bytes, BOM, out, byteA, byteB);
                    return 0;
                } else {
                    printError(position - 2);
                    printError(position - 1);
                    bytes[0] = bytes[2];
                    inBufferCount -= 2;
                    return converting(bytes, inBufferCount, position, BOM, out);
                }
            } else {
                printError(position - 2);
                inBufferCount = moveBytes(bytes, inBufferCount);
                return converting(bytes, inBufferCount, position, BOM, out);
            }
        }
        else return inBufferCount;
    }
    else {
        printError(position - 2);
        inBufferCount = moveBytes(bytes, inBufferCount);
        return converting(bytes, inBufferCount, position, BOM, out);
    }
}


int main(int argc, char **argv) {
    FILE *inputf;
    FILE *outputf;
    unsigned char byte, inputBytes[3];
    int BOM = 0, pos = 0; //if BOM == 0 => lE, else be
    if ((inputf = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Error with the input file!\n");
        exit(1);
    }
    if ((outputf = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "Error with the output file!\n");
        fclose(inputf);
        exit(1);
    }

    int inside = fread(&inputBytes, sizeof byte, 3, inputf);
    if (inside == 3)
        BOM = (inputBytes[0] == 0xEF) && (inputBytes[1] == 0xBB) && (inputBytes[2] == 0xBF);
    pos += inside;
    if (!BOM){
        unsigned char fe = 0xFE;
        unsigned char ff = 0xFF;
        putc(ff,outputf);
        putc(fe, outputf);
    }
    inside = converting (inputBytes, inside, pos, BOM, outputf);


    while (!feof(inputf)) {
      if (inside != 3) {
          while ((!feof(inputf)) && (inside != 3)){
              byte = fgetc(inputf);
              pos++;
              inside++;
              inputBytes[inside - 1] = byte;
            }
        }
        inside = converting(inputBytes, inside, pos, BOM, outputf);
    }
    fclose(inputf);
    fclose(outputf);
    return 0;
}
