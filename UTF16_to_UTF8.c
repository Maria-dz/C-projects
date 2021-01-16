#include <stdio.h>
#include <stdlib.h>

void creatingUTF8 (unsigned short ch, FILE *outf){
    char nch1, nch2, nch3;
    if (ch <= 127){
        nch1 = ch & 0xFF;
        putc (nch1, outf);
    }
    else if (ch <= 2047){
        nch1 = 0xC0 | ((ch >> 6) & 0x1F);
        nch2 = 0x80 | (ch & 0x3F);
        putc (nch1, outf);
        putc (nch2, outf);
    }
    else{
        nch1 = 0xE0 | ((ch >> 12) & 0x0F);
        nch2 = 0x80 | ((ch >> 6) & 0x3F);
        nch3 = 0x80 | (ch & 0x3F);
        putc(nch1, outf);
        putc(nch2, outf);
        putc(nch3, outf);
    }
}

int main (int argc, char **argv){
    unsigned short utf16ch;
    unsigned char bytes[2];
    FILE* inputf;
    FILE* outputf;
    int checkmark = 0; //checkmark = 1 if 0xFFFE, else checkmark = 0

    if ((inputf = fopen(argv[1], "rb")) == NULL){
        fprintf (stderr, "Error with the input file!\n");
        exit(1);
    }
    if ((outputf = fopen(argv[2], "wb")) == NULL){
        fprintf (stderr, "Error with the output file!\n");
        fclose(inputf);
        exit(1);
    }
    if (fread(&utf16ch, sizeof utf16ch, 1, inputf)){
        if (utf16ch == 0xFFFE){
            checkmark = 1;
        }
        else{
            if (utf16ch != 0xFEFF){
                creatingUTF8(utf16ch, outputf);
            }
        }
        while (!feof(inputf)){
            int count = fread(&bytes, 1, 2, inputf);
            if (count == 2){
                  utf16ch = bytes[0] << 8;
                  utf16ch = utf16ch | bytes[1];
                  if (!checkmark) utf16ch = (utf16ch << 8) | (utf16ch >> 8);
                  creatingUTF8(utf16ch, outputf);
            }
            else{ if (count == 1) {
                    fprintf(stderr, "Odd number of bytes!\n");
                    break;
                }
            }

            }
        }
    else{
        if (feof(inputf)) fprintf(stderr, "Unexpected end of the file");
        if (ferror(inputf)) fprintf(stderr, "Error with an input file");
    }
    fclose(inputf);
    fclose(outputf);
    return 0;
}
