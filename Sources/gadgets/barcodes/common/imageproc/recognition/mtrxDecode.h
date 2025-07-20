#ifndef MATRXDCD_INC
#define MATRXDCD_INC

#include "MxReader.h"

extern int logVal[];
extern int aLogVal[];

typedef enum {
   DmtxMaskBit1 = 0x01 << 7,
   DmtxMaskBit2 = 0x01 << 6,
   DmtxMaskBit3 = 0x01 << 5,
   DmtxMaskBit4 = 0x01 << 4,
   DmtxMaskBit5 = 0x01 << 3,
   DmtxMaskBit6 = 0x01 << 2,
   DmtxMaskBit7 = 0x01 << 1,
   DmtxMaskBit8 = 0x01 << 0
} DmtxBitMask;

typedef enum {
   DmtxSchemeDecodeAsciiStd,
   DmtxSchemeDecodeAsciiExt,
   DmtxSchemeDecodeC40,
   DmtxSchemeDecodeText,
   DmtxSchemeDecodeX12,
   DmtxSchemeDecodeEdifact,
   DmtxSchemeDecodeBase256
} DmtxSchemeDecode;

__forceinline int GfProduct(int a, int b)
{
   if(a == 0 || b == 0)
      return 0;
   else
      return aLogVal[(logVal[a] + logVal[b]) % 255];
}

const int dataRegionRows[] =   { 8, 10, 12, 14, 16, 18, 20, 22, 24,
                                 14, 16, 18, 20, 22, 24,
                                 14, 16, 18, 20, 22, 24,
                                 18, 20, 22,
                                 6,  6, 10, 10, 14, 14 };
const int dataRegionCols[] =   { 8, 10, 12, 14, 16, 18, 20, 22, 24,
                                 14, 16, 18, 20, 22, 24,
                                 14, 16, 18, 20, 22, 24,
                                 18, 20, 22,
                                 16, 14, 24, 16, 16, 22 };
const int horizDataRegions[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 2, 2, 2, 2, 2, 2,
                                 4, 4, 4, 4, 4, 4,
                                 6, 6, 6,
                                 1, 2, 1, 2, 2, 2 };
const int interleavedBlocks[] ={ 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 2,
                                 2, 4, 4, 4, 4, 6,
                                 6, 8, 8,
                                 1, 1, 1, 1, 1, 1 };
const int errorWordLength[] =  { 5, 7, 10, 12,  14,  18,  20,  24,  28,
                                 36,  42,  48,  56,  68,  84,
                                 112, 144, 192, 224, 272, 336,
                                 408, 496, 620,
                                 7,  11,  14,  18,  24,  28 };
const int dataWordLength[] =   { 3, 5, 8, 12,  18,  22,   30,   36,  44,
                                 62,  86, 114,  144,  174, 204,
                                 280, 368, 456,  576,  696, 816,
                                 1050, 1304, 1558,
                                 5,  10,  16,   22,   32,  49 };

#ifndef GfSum
#define GfSum(a,b) (a ^ b)
#endif

__forceinline void PlaceModule(unsigned char *modules, int mappingRows, int mappingCols, int row, int col, unsigned char *codeword, DmtxBitMask mask, int moduleOnColor)
{
   if(row < 0) {
      row += mappingRows;
      col += 4 - ((mappingRows+4)%8);
   }
   if(col < 0) {
      col += mappingCols;
      row += 4 - ((mappingCols+4)%8);
   }

   /* If module has already been assigned then we are decoding the pattern into codewords */
   if(modules[row*mappingCols+col] & DMTX_MODULE_ASSIGNED) {
      if(modules[row*mappingCols+col] & moduleOnColor)
         *codeword |= mask;
      else
         *codeword &= (0xff ^ mask);
   }
   /* Otherwise we are encoding the codewords into a pattern */
   else {
      if(*codeword & mask)
         modules[row*mappingCols+col] |= moduleOnColor;

      modules[row*mappingCols+col] |= DMTX_MODULE_ASSIGNED;
   }

   modules[row*mappingCols+col] |= DMTX_MODULE_VISITED;
}

__forceinline void PatternShapeStandard(unsigned char *modules, int mappingRows, int mappingCols, int row, int col, unsigned char *codeword, int moduleOnColor)
{
   PlaceModule(modules, mappingRows, mappingCols, row-2, col-2, codeword, DmtxMaskBit1, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row-2, col-1, codeword, DmtxMaskBit2, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row-1, col-2, codeword, DmtxMaskBit3, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row-1, col-1, codeword, DmtxMaskBit4, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row-1, col,   codeword, DmtxMaskBit5, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row,   col-2, codeword, DmtxMaskBit6, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row,   col-1, codeword, DmtxMaskBit7, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, row,   col,   codeword, DmtxMaskBit8, moduleOnColor);
}


__forceinline void PatternShapeSpecial1(unsigned char *modules, int mappingRows, int mappingCols, unsigned char *codeword, int moduleOnColor)
{
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 0, codeword, DmtxMaskBit1, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 1, codeword, DmtxMaskBit2, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 2, codeword, DmtxMaskBit3, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-2, codeword, DmtxMaskBit4, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-1, codeword, DmtxMaskBit5, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-1, codeword, DmtxMaskBit6, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 2, mappingCols-1, codeword, DmtxMaskBit7, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 3, mappingCols-1, codeword, DmtxMaskBit8, moduleOnColor);
}

__forceinline void PatternShapeSpecial2(unsigned char *modules, int mappingRows, int mappingCols, unsigned char *codeword, int moduleOnColor)
{
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-3, 0, codeword, DmtxMaskBit1, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-2, 0, codeword, DmtxMaskBit2, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 0, codeword, DmtxMaskBit3, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-4, codeword, DmtxMaskBit4, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-3, codeword, DmtxMaskBit5, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-2, codeword, DmtxMaskBit6, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-1, codeword, DmtxMaskBit7, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-1, codeword, DmtxMaskBit8, moduleOnColor);
}

__forceinline void PatternShapeSpecial3(unsigned char *modules, int mappingRows, int mappingCols, unsigned char *codeword, int moduleOnColor)
{
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-3, 0, codeword, DmtxMaskBit1, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-2, 0, codeword, DmtxMaskBit2, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 0, codeword, DmtxMaskBit3, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-2, codeword, DmtxMaskBit4, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-1, codeword, DmtxMaskBit5, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-1, codeword, DmtxMaskBit6, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 2, mappingCols-1, codeword, DmtxMaskBit7, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 3, mappingCols-1, codeword, DmtxMaskBit8, moduleOnColor);
}

__forceinline void PatternShapeSpecial4(unsigned char *modules, int mappingRows, int mappingCols, unsigned char *codeword, int moduleOnColor)
{
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, 0, codeword, DmtxMaskBit1, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, mappingRows-1, mappingCols-1, codeword, DmtxMaskBit2, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-3, codeword, DmtxMaskBit3, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-2, codeword, DmtxMaskBit4, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 0, mappingCols-1, codeword, DmtxMaskBit5, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-3, codeword, DmtxMaskBit6, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-2, codeword, DmtxMaskBit7, moduleOnColor);
   PlaceModule(modules, mappingRows, mappingCols, 1, mappingCols-1, codeword, DmtxMaskBit8, moduleOnColor);
}

__forceinline int DmtxSymAttribVertDataRegions(int sizeIdx)
{
    return (sizeIdx < DMTX_SYMBOL_SQUARE_COUNT) ? horizDataRegions[sizeIdx] : 1;
}

__forceinline int DmtxSymAttribMappingMatrixRows(int sizeIdx)
{
    return dataRegionRows[sizeIdx] * DmtxSymAttribVertDataRegions(sizeIdx);
}


__forceinline int DmtxSymAttribMappingMatrixCols(int sizeIdx)
{
    return dataRegionCols[sizeIdx] * horizDataRegions[sizeIdx];
}

__forceinline int DmtxSymAttribInterleavedBlocks(int sizeIdx)
{
    return interleavedBlocks[sizeIdx];
}

__forceinline int DmtxSymAttribErrorWordLength(int sizeIdx)
{
    return errorWordLength[sizeIdx];
}

__forceinline int DmtxSymAttribDataWordLength(int sizeIdx)
{
    return dataWordLength[sizeIdx];
}

__forceinline int ModulePlacementEcc200(unsigned char *modules, unsigned char *codewords, int sizeIdx, int moduleOnColor)
{
   int row, col, chr;
   int mappingRows, mappingCols;

   ASSERT(moduleOnColor & (DMTX_MODULE_ON_RED | DMTX_MODULE_ON_GREEN | DMTX_MODULE_ON_BLUE));

   mappingRows = DmtxSymAttribMappingMatrixRows(sizeIdx);
   mappingCols = DmtxSymAttribMappingMatrixCols(sizeIdx);

   /* Start in the nominal location for the 8th bit of the first character */
   chr = 0;
   row = 4;
   col = 0;

   do {
      /* Repeatedly first check for one of the special corner cases */
      if((row == mappingRows) && (col == 0))
         PatternShapeSpecial1(modules, mappingRows, mappingCols, &(codewords[chr++]), moduleOnColor);
      else if((row == mappingRows-2) && (col == 0) && (mappingCols%4))
         PatternShapeSpecial2(modules, mappingRows, mappingCols, &(codewords[chr++]), moduleOnColor);
      else if((row == mappingRows-2) && (col == 0) && (mappingCols%8 == 4))
         PatternShapeSpecial3(modules, mappingRows, mappingCols, &(codewords[chr++]), moduleOnColor);
      else if((row == mappingRows+4) && (col == 2) && (!(mappingCols%8)))
         PatternShapeSpecial4(modules, mappingRows, mappingCols, &(codewords[chr++]), moduleOnColor);

      /* Sweep upward diagonally, inserting successive characters */
      do {
         if((row < mappingRows) && (col >= 0) &&
               !(modules[row*mappingCols+col] & DMTX_MODULE_VISITED))
            PatternShapeStandard(modules, mappingRows, mappingCols, row, col, &(codewords[chr++]), moduleOnColor);
         row -= 2;
         col += 2;
      } while ((row >= 0) && (col < mappingCols));
      row += 1;
      col += 3;

      /* Sweep downward diagonally, inserting successive characters */
      do {
         if((row >= 0) && (col < mappingCols) &&
               !(modules[row*mappingCols+col] & DMTX_MODULE_VISITED))
            PatternShapeStandard(modules, mappingRows, mappingCols, row, col, &(codewords[chr++]), moduleOnColor);
         row += 2;
         col -= 2;
      } while ((row < mappingRows) && (col >= 0));
      row += 3;
      col += 1;
      /* ... until the entire modules array is scanned */
   } while ((row < mappingRows) || (col < mappingCols));

   /* If lower righthand corner is untouched then fill in the fixed pattern */
   if(!(modules[mappingRows * mappingCols - 1] &
         DMTX_MODULE_VISITED)) {

      modules[mappingRows * mappingCols - 1] |= moduleOnColor;
      modules[(mappingRows * mappingCols) - mappingCols - 2] |= moduleOnColor;
   } /* XXX should this fixed pattern also be used in reading somehow? */

   /* XXX compare that chr == region->dataSize here */
   return chr; /* XXX number of codewords read off */
}

__forceinline int DecodeCheckErrors(DmtxMatrixRegion *region)
{
   int i, j, step, errorWordLength;
   unsigned char reg, a;

   step = DmtxSymAttribInterleavedBlocks(region->sizeIdx);
   errorWordLength = DmtxSymAttribErrorWordLength(region->sizeIdx);

   for(i = 0; i < errorWordLength; i++) {
      a = aLogVal[i / step + 1];

      reg = 0;
      for(j = i % step; j < region->codeSize; j += step) {
         reg = GfSum(region->code[j], GfProduct(a, reg));
      }

      if(reg != 0)
         return DMTX_FAILURE;
   }

   return DMTX_SUCCESS;
}

__forceinline unsigned char * NextEncodationScheme(DmtxSchemeDecode *encScheme, unsigned char *ptr)
{
   switch(*ptr) {
      case 230:
         *encScheme = DmtxSchemeDecodeC40;
         break;
      case 231:
         *encScheme = DmtxSchemeDecodeBase256;
         break;
      case 235:
         *encScheme = DmtxSchemeDecodeAsciiExt;
         break;
      case 238:
         *encScheme = DmtxSchemeDecodeX12;
         break;
      case 239:
         *encScheme = DmtxSchemeDecodeText;
         break;
      case 240:
         *encScheme = DmtxSchemeDecodeEdifact;
         break;
      default:
         *encScheme = DmtxSchemeDecodeAsciiStd;
         return ptr;
   }

   return ptr + 1;
}

__forceinline unsigned char *DecodeSchemeAsciiStd(DmtxMatrixRegion *region, unsigned char *ptr, unsigned char *dataEnd)
{
   int digits;

   if(*ptr <= 128) {
      region->output[region->outputIdx++] = *ptr - 1;
   }
   else if(*ptr == 129) {
      return dataEnd;
   }
   else if(*ptr <= 229) {
      digits = *ptr - 130;
      region->output[region->outputIdx++] = digits/10 + '0';
      region->output[region->outputIdx++] = digits - (digits/10)*10 + '0';
   }

   return ptr + 1;
}

__forceinline unsigned char *DecodeSchemeAsciiExt(DmtxMatrixRegion *region, unsigned char *ptr, unsigned char *dataEnd)
{
   region->output[region->outputIdx++] = *ptr + 128;

   return ptr + 1;
}

__forceinline unsigned char *DecodeSchemeC40Text(DmtxMatrixRegion *region, unsigned char *ptr, unsigned char *dataEnd, DmtxSchemeDecode encScheme)
{
   int i;
   int packed;
   int shift = 0;
   unsigned char c40Values[3];

   ASSERT(encScheme == DmtxSchemeDecodeC40 || encScheme == DmtxSchemeDecodeText);

   while(ptr < dataEnd) {

      /* FIXME Also check that ptr+1 is safe to access */
      packed = (*ptr << 8) | *(ptr+1);
      c40Values[0] = ((packed - 1)/1600);
      c40Values[1] = ((packed - 1)/40) % 40;
      c40Values[2] =  (packed - 1) % 40;
      ptr += 2;

      for(i = 0; i < 3; i++) {
         if(shift == 0) { /* Basic set */
            if(c40Values[i] <= 2) {
               shift = c40Values[i] + 1;
            }
            else if(c40Values[i] == 3) {
               region->output[region->outputIdx++] = ' '; /* Space */
            }
            else if(c40Values[i] <= 13) {
               region->output[region->outputIdx++] = c40Values[i] - 13 + '9'; /* 0-9 */
            }
            else if(c40Values[i] <= 39) {
               if(encScheme == DmtxSchemeDecodeC40) {
                  region->output[region->outputIdx++] = c40Values[i] - 39 + 'Z'; /* A-Z */
               }
               else if(encScheme == DmtxSchemeDecodeText) {
                  region->output[region->outputIdx++] = c40Values[i] - 39 + 'z'; /* a-z */
               }
            }
         }
         else if(shift == 1) { /* Shift 1 set */
            region->output[region->outputIdx++] = c40Values[i]; /* ASCII 0 - 31 */

            shift = 0;
         }
         else if(shift == 2) { /* Shift 2 set */
            if(c40Values[i] <= 14)
               region->output[region->outputIdx++] = c40Values[i] + 33; /* ASCII 33 - 47 */
            else if(c40Values[i] <= 21)
               region->output[region->outputIdx++] = c40Values[i] + 43; /* ASCII 58 - 64 */
            else if(c40Values[i] <= 26)
               region->output[region->outputIdx++] = c40Values[i] + 69; /* ASCII 91 - 95 */
            else if(c40Values[i] == 27)
               fprintf(stdout, "FNC1 (?)"); /* FNC1 (eh?) */
            else if(c40Values[i] == 30)
               fprintf(stdout, "Upper Shift (?)"); /* Upper Shift (eh?) */

            shift = 0;
         }
         else if(shift == 3) { /* Shift 3 set */
            if(encScheme == DmtxSchemeDecodeC40) {
               region->output[region->outputIdx++] = c40Values[i] + 96;
            }
            else if(encScheme == DmtxSchemeDecodeText) {
               if(c40Values[i] == 0)
                  region->output[region->outputIdx++] = c40Values[i] + 96;
               else if(c40Values[i] <= 26)
                  region->output[region->outputIdx++] = c40Values[i] - 26 + 'Z'; /* A-Z */
               else
                  region->output[region->outputIdx++] = c40Values[i] - 31 + 127; /* { | } ~ DEL */
            }

            shift = 0;
         }
      }

      /* Unlatch if codeword 254 follows 2 codewords in C40/Text encodation */
      if(*ptr == 254)
         return ptr + 1;

      /* Unlatch is implied if only one codeword remains */
      if(dataEnd - ptr == 1)
         return ptr;
   }

   return ptr;
}

__forceinline unsigned char *DecodeSchemeX12(DmtxMatrixRegion *region, unsigned char *ptr, unsigned char *dataEnd)
{
   int i;
   int packed;
   unsigned char x12Values[3];

   while(ptr < dataEnd) {

      /* FIXME Also check that ptr+1 is safe to access */
      packed = (*ptr << 8) | *(ptr+1);
      x12Values[0] = ((packed - 1)/1600);
      x12Values[1] = ((packed - 1)/40) % 40;
      x12Values[2] =  (packed - 1) % 40;
      ptr += 2;

      for(i = 0; i < 3; i++) {
         if(x12Values[i] == 0)
            region->output[region->outputIdx++] = 13;
         else if(x12Values[i] == 1)
            region->output[region->outputIdx++] = 42;
         else if(x12Values[i] == 2)
            region->output[region->outputIdx++] = 62;
         else if(x12Values[i] == 3)
            region->output[region->outputIdx++] = 32;
         else if(x12Values[i] <= 13)
            region->output[region->outputIdx++] = x12Values[i] + 44;
         else if(x12Values[i] <= 90)
            region->output[region->outputIdx++] = x12Values[i] + 51;
      }

      /* Unlatch if codeword 254 follows 2 codewords in C40/Text encodation */
      if(*ptr == 254)
         return ptr + 1;

      /* Unlatch is implied if only one codeword remains */
      if(dataEnd - ptr == 1)
         return ptr;
   }

   return ptr;
}

__forceinline unsigned char *DecodeSchemeEdifact(DmtxMatrixRegion *region, unsigned char *ptr, unsigned char *dataEnd)
{
   int i;
   unsigned char unpacked[4];

   while(ptr < dataEnd) {

      /* FIXME Also check that ptr+2 is safe to access -- shouldn't be a
         problem because I'm guessing you can guarantee there will always
         be at least 3 error codewords */
      unpacked[0] = (*ptr & 0xfc) >> 2;
      unpacked[1] = (*ptr & 0x03) << 4 | (*(ptr+1) & 0xf0) >> 4;
      unpacked[2] = (*(ptr+1) & 0x0f) << 2 | (*(ptr+2) & 0xc0) >> 6;
      unpacked[3] = *(ptr+2) & 0x3f;

      for(i = 0; i < 4; i++) {

         /* Advance input ptr (4th value comes from already-read 3rd byte) */
         if(i < 3)
            ptr++;

         /* Test for unlatch condition */
         if(unpacked[i] == 0x1f) {
            ASSERT(region->output[region->outputIdx] == 0); /* XXX dirty why? */
            return ptr;
         }

         region->output[region->outputIdx++] = unpacked[i] ^ (((unpacked[i] & 0x20) ^ 0x20) << 1);
      }

      /* Unlatch is implied if fewer than 3 codewords remain */
      if(dataEnd - ptr < 3) {
         return ptr;
      }
   }

   return ptr;
}

__forceinline unsigned char UnRandomize255State(unsigned char value, int idx)
{
   int pseudoRandom;
   int tmp;

   pseudoRandom = ((149 * idx) % 255) + 1;
   tmp = value - pseudoRandom;

   return (tmp >= 0) ? tmp : tmp + 256;
}

__forceinline unsigned char *DecodeSchemeBase256(DmtxMatrixRegion *region, 
   unsigned char *ptr, unsigned char *dataEnd)
{
   int d0, d1;
   int i;
   unsigned char *ptrEnd;

   /* XXX i is the positional index used for unrandomizing */
   i = (int)(ptr - region->code) + 1;

   d0 = UnRandomize255State(*(ptr++), i++);
   if(d0 == 0) {
      ptrEnd = dataEnd;
   }
   else if(d0 <= 249) {
      ptrEnd = ptr + d0; /* XXX not verifed */
   }
   else {
      d1 = UnRandomize255State(*(ptr++), i++);
      ptrEnd = ptr + (d0 - 249) * 250 + d1; /* XXX not verified */
   }

   if(ptrEnd > dataEnd) {
      exit(40); /* XXX needs cleaner error handling */
   }

   while(ptr < ptrEnd) {
      region->output[region->outputIdx++] = UnRandomize255State(*(ptr++), i++);
   }

   return ptr;
}

__forceinline void DecodeDataStream(DmtxMatrixRegion *region)
{
   DmtxSchemeDecode encScheme;
   unsigned char *ptr, *dataEnd;

   ptr = region->code;
   dataEnd = ptr + DmtxSymAttribDataWordLength(region->sizeIdx);

   while(ptr < dataEnd) {

      ptr = NextEncodationScheme(&encScheme, ptr);

      switch(encScheme) {
         case DmtxSchemeDecodeAsciiStd:
            ptr = DecodeSchemeAsciiStd(region, ptr, dataEnd);
            break;

         case DmtxSchemeDecodeAsciiExt:
            ptr = DecodeSchemeAsciiExt(region, ptr, dataEnd);
            break;

         case DmtxSchemeDecodeC40:
         case DmtxSchemeDecodeText:
            ptr = DecodeSchemeC40Text(region, ptr, dataEnd, encScheme);
            break;

         case DmtxSchemeDecodeX12:
            ptr = DecodeSchemeX12(region, ptr, dataEnd);
            break;

         case DmtxSchemeDecodeEdifact:
            ptr = DecodeSchemeEdifact(region, ptr, dataEnd);
            break;

         case DmtxSchemeDecodeBase256:
            ptr = DecodeSchemeBase256(region, ptr, dataEnd);
            break;
      }
   }
}


__forceinline int DecodeRegion(DmtxMatrixRegion *region)
{
   int success;

   ModulePlacementEcc200(region->array, region->code,
         region->sizeIdx, DMTX_MODULE_ON_RED | DMTX_MODULE_ON_GREEN | DMTX_MODULE_ON_BLUE);

   success = DecodeCheckErrors(region);
   if(!success) {
      fprintf(stderr, "Rejected due to ECC validation\n");
      return DMTX_FAILURE;
   }

   DecodeDataStream(region);

   return DMTX_SUCCESS;
}

#endif

