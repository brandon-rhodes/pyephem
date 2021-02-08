/* explode a raw gif file already in any array of char.
 * return 0 if ok, else -1.
 * code from fit2gif.tar.gz, author unknown but thanks!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	RDSZ	4096			/* bytes per read attempt */
#define	True	1
#define	False	0

typedef unsigned char byte;

typedef struct { byte *pic;                  /* image data */
		 int   w, h;                 /* size */
		 byte  r[256],g[256],b[256]; /* colormap */
	       } PICINFO;

typedef int boolean;

#define EXTENSION     0x21
#define IMAGESEP      0x2c
#define TRAILER       0x3b
#define INTERLACEMASK 0x40
#define COLORMAPMASK  0x80

  

static int BitOffset = 0,	/* Bit Offset of next code */
    XC = 0, YC = 0,		/* Output X and Y coords of current pixel */
    Pass = 0,			/* Used by output routine if interlaced pic */
    OutCount = 0,		/* Decompressor output 'stack count' */
    RWidth, RHeight,		/* screen dimensions */
    Width, Height,		/* image dimensions */
    LeftOfs, TopOfs,		/* image offset */
    BitsPerPixel,		/* Bits per pixel, read from GIF header */
    ColorMapSize,		/* number of colors */
    Background,			/* background color */
    CodeSize,			/* Code size, read from GIF header */
    InitCodeSize,		/* Starting code size, used during Clear */
    Code,			/* Value returned by ReadCode */
    MaxCode,			/* limiting value for current code size */
    ClearCode,			/* GIF clear code */
    EOFCode,			/* GIF end-of-information code */
    CurCode, OldCode, InCode,	/* Decompressor variables */
    FirstFree,			/* First free code, generated per GIF spec */
    FreeCode,			/* Decompressor,next free slot in hash table */
    FinChar,			/* Decompressor variable */
    BitMask,			/* AND mask for data size */
    ReadMask,			/* Code AND mask for current code size */
    Misc;                       /* miscellaneous bits (interlace, local cmap)*/


static boolean Interlace, HasColormap;

static byte *RawGIF;			/* The heap array to hold it, raw */
static byte *Raster;			/* The raster data stream, unblocked */
static byte *pic8;

    /* The hash table used by the decompressor */
static int Prefix[4096];
static int Suffix[4096];

    /* An output array used by the decompressor */
static int OutCode[4097];

static int   gif89 = 0;
static char *id87 = "GIF87a";
static char *id89 = "GIF89a";

static int EGApalette[16][3] = {
  {0,0,0},       {0,0,128},     {0,128,0},     {0,128,128}, 
  {128,0,0},     {128,0,128},   {128,128,0},   {200,200,200},
  {100,100,100}, {100,100,255}, {100,255,100}, {100,255,255},
  {255,100,100}, {255,100,255}, {255,255,100}, {255,255,255} };
  

static int   readImage();
static int   ReadCode();
static void  DoInterlace();
static int   gifError();

static int   filesize;
static int numcols;
static float normaspect;
static byte dummy;
static byte *dataptr;
#define NEXTBYTE (dummy = *dataptr++)

static char *g_errmsg;

static int LoadGIF();

int
explodeGIF (raw, nraw, wp, hp, pixap, ra, ga, ba, errmsg)
unsigned char *raw;		/* raw gif file */
int nraw;			/* bytes ingif file */
int *wp, *hp;			/* RETURN: image width/height */
unsigned char *pixap[];		/* RETURN: malloced w*h pixels, rgba indices */
unsigned char ra[], ga[], ba[];	/* RETURN: color map, 256 each */
char errmsg[];			/* excuse, if return -1 */
{
	PICINFO pi;

	dataptr = raw;
	filesize = nraw;
	g_errmsg = errmsg;
	if (LoadGIF (&pi) != 1)
	    return (-1);
	*wp = pi.w;
	*hp = pi.h;
	*pixap = pi.pic;
	memcpy (ra, pi.r, 256);
	memcpy (ga, pi.g, 256);
	memcpy (ba, pi.b, 256);
	return (0);
}


/*****************************/
static int LoadGIF(pinfo)
     PICINFO *pinfo;
/*****************************/
{
  /* returns '1' if successful */

  register byte  ch, *origptr;
  register int   i, block;
  int            aspect, gotimage;

  /* initialize variables */
  BitOffset = XC = YC = Pass = OutCount = gotimage = 0;
  Raster = pic8 = NULL;
  gif89 = 0;
  pinfo->pic     = (byte *) NULL;
  RawGIF = dataptr;
  
  /* the +256's are so we can read truncated GIF files without fear of 
     segmentation violation */
  if (!(Raster = (byte *) calloc(filesize+256,1))) 
    return( gifError(pinfo, "not enough memory to read gif file") );
  
  origptr = dataptr;

  if      (strncmp((char *) dataptr, id87, 6)==0) gif89 = 0;
  else if (strncmp((char *) dataptr, id89, 6)==0) gif89 = 1;
  else    return( gifError(pinfo, "not a GIF file"));
  
  dataptr += 6;
  
  /* Get variables from the GIF screen descriptor */
  
  ch = NEXTBYTE;
  RWidth = ch + 0x100 * NEXTBYTE;	/* screen dimensions... not used. */
  ch = NEXTBYTE;
  RHeight = ch + 0x100 * NEXTBYTE;
  
  ch = NEXTBYTE;
  HasColormap = ((ch & COLORMAPMASK) ? True : False);
  
  BitsPerPixel = (ch & 7) + 1;
  numcols = ColorMapSize = 1 << BitsPerPixel;
  BitMask = ColorMapSize - 1;
  
  Background = NEXTBYTE;		/* background color... not used. */
  
  aspect = NEXTBYTE;
  if (aspect) {
    if (!gif89) return(gifError(pinfo,"corrupt GIF file (screen descriptor)"));
    else normaspect = (float)((aspect + 15) / 64.0);   /* gif89 aspect ratio */
  }
  
  
  /* Read in global colormap. */
  
  if (HasColormap) {
    for (i=0; i<ColorMapSize; i++) {
      pinfo->r[i] = NEXTBYTE;
      pinfo->g[i] = NEXTBYTE;
      pinfo->b[i] = NEXTBYTE;
    }

    /* fill out with last color to avoid garbage clobbering xephem colormap.
     * from Jean-Etienne.LAMIAUD@tcc.thomson-csf.com
     */
    for( ; i < 256; i++ ) {
	pinfo->r[i] = pinfo->r[ColorMapSize-1];
	pinfo->g[i] = pinfo->g[ColorMapSize-1];
	pinfo->b[i] = pinfo->b[ColorMapSize-1];
    }
  } else {  /* no colormap in GIF file */
    /* put std EGA palette (repeated 16 times) into colormap, for lack of
       anything better to do */

    for (i=0; i<256; i++) {
      pinfo->r[i] = EGApalette[i&15][0];
      pinfo->g[i] = EGApalette[i&15][1];
      pinfo->b[i] = EGApalette[i&15][2];
    }
  }

  /* possible things at this point are:
   *   an application extension block
   *   a comment extension block
   *   an (optional) graphic control extension block
   *       followed by either an image
   *	   or a plaintext extension
   */

  while (1) {
    block = NEXTBYTE;

    if (block == EXTENSION) {  /* parse extension blocks */
      int i, fn, blocksize, aspnum, aspden;

      /* read extension block */
      fn = NEXTBYTE;

      if (fn == 'R') {                  /* GIF87 aspect extension */
	int sbsize;

	blocksize = NEXTBYTE;
	if (blocksize == 2) {
	  aspnum = NEXTBYTE;
	  aspden = NEXTBYTE;
	  if (aspden>0 && aspnum>0) 
	    normaspect = (float) aspnum / (float) aspden;
	  else { normaspect = 1.0;  aspnum = aspden = 1; }
	}
	else {
	  for (i=0; i<blocksize; i++) NEXTBYTE;
	}

	while ((sbsize=NEXTBYTE)>0) {  /* eat any following data subblocks */
	  for (i=0; i<sbsize; i++) NEXTBYTE;
	}
      }


      else if (fn == 0xFE) {  /* Comment Extension */
	int   ch, j, sbsize, cmtlen;
	byte *ptr1;

	cmtlen = 0;
	ptr1 = dataptr;      /* remember start of comments */

	/* figure out length of comment */
	do {
	  sbsize = NEXTBYTE;
	  cmtlen += sbsize;
	  for (j=0; j<sbsize; j++) ch = NEXTBYTE;
	} while (sbsize);


	if (cmtlen>0) {
	    do {
	      sbsize = (*ptr1++);
	      for (j=0; j<sbsize; j++, ptr1++) continue;
	    } while (sbsize);
	}  /* if cmtlen>0 */
      }


      else if (fn == 0x01) {  /* PlainText Extension */
	int j,sbsize,ch;
	int tgLeft, tgTop, tgWidth, tgHeight, cWidth, cHeight, fg, bg;
      
	sbsize   = NEXTBYTE;
	tgLeft   = NEXTBYTE;  tgLeft   += (NEXTBYTE)<<8;
	tgTop    = NEXTBYTE;  tgTop    += (NEXTBYTE)<<8;
	tgWidth  = NEXTBYTE;  tgWidth  += (NEXTBYTE)<<8;
	tgHeight = NEXTBYTE;  tgHeight += (NEXTBYTE)<<8;
	cWidth   = NEXTBYTE;
	cHeight  = NEXTBYTE;
	fg       = NEXTBYTE;
	bg       = NEXTBYTE;
	i=12;
	for ( ; i<sbsize; i++) NEXTBYTE;   /* read rest of first subblock*/
      
	/* read (and ignore) data sub-blocks */
	do {
	  j = 0;
	  sbsize = NEXTBYTE;
	  while (j<sbsize) {
	    ch = NEXTBYTE;  j++;
	  }
	} while (sbsize);
      }


      else if (fn == 0xF9) {  /* Graphic Control Extension */
	int j, sbsize;

	/* read (and ignore) data sub-blocks */
	do {
	  j = 0; sbsize = NEXTBYTE;
	  while (j<sbsize) { NEXTBYTE;  j++; }
	} while (sbsize);
      }
      

      else if (fn == 0xFF) {  /* Application Extension */
	int j, sbsize;

	/* read (and ignore) data sub-blocks */
	do {
	  j = 0; sbsize = NEXTBYTE;
	  while (j<sbsize) { NEXTBYTE;  j++; }
	} while (sbsize);
      }
      

      else { /* unknown extension */
	int j, sbsize;

	/* read (and ignore) data sub-blocks */
	do {
	  j = 0; sbsize = NEXTBYTE;
	  while (j<sbsize) { NEXTBYTE;  j++; }
	} while (sbsize);
      }
    }


    else if (block == IMAGESEP) {
      if (gotimage) {   /* just skip over remaining images */
	int i,misc,ch,ch1;

	/* skip image header */
	NEXTBYTE;  NEXTBYTE;  /* left position */
	NEXTBYTE;  NEXTBYTE;  /* top position */
	NEXTBYTE;  NEXTBYTE;  /* width */
	NEXTBYTE;  NEXTBYTE;  /* height */
	misc = NEXTBYTE;      /* misc. bits */

	if (misc & 0x80) {    /* image has local colormap.  skip it */
	  for (i=0; i< 1 << ((misc&7)+1);  i++) {
	    NEXTBYTE;  NEXTBYTE;  NEXTBYTE;
	  }
	}

	NEXTBYTE;       /* minimum code size */

	/* skip image data sub-blocks */
	do {
	  ch = ch1 = NEXTBYTE;
	  while (ch--) NEXTBYTE;
	  if ((dataptr - RawGIF) > filesize) break;      /* EOF */
	} while(ch1);
      }

      else if (readImage(pinfo)) gotimage = 1;
    }


    else if (block == TRAILER) {      /* stop reading blocks */
      break;
    }

    else {      /* unknown block type */
      char str[128];

      /* don't mention bad block if file was trunc'd, as it's all bogus */
      if ((dataptr - origptr) < filesize) {
	sprintf(str, "Unknown block type (0x%02x) at offset 0x%x",
		block, (unsigned int)(dataptr - origptr) - 1);

	if (!gotimage) return gifError(pinfo, str);
	else return( gifError(pinfo, str));
      }

      break;
    }
  }

  free(Raster);  Raster = NULL;

  if (!gotimage) 
     return( gifError(pinfo, "no image data found in GIF file") );

  return 1;
}


/********************************************/
static int readImage(pinfo)
     PICINFO *pinfo;
{
  register byte ch, ch1, *ptr1, *picptr;
  int           i, npixels, maxpixels;

  npixels = maxpixels = 0;

  /* read in values from the image descriptor */
  
  ch = NEXTBYTE;
  LeftOfs = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  TopOfs  = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Width   = ch + 0x100 * NEXTBYTE;
  ch = NEXTBYTE;
  Height  = ch + 0x100 * NEXTBYTE;

  Misc = NEXTBYTE;
  Interlace = ((Misc & INTERLACEMASK) ? True : False);

  if (Misc & 0x80) {
    for (i=0; i< 1 << ((Misc&7)+1); i++) {
      pinfo->r[i] = NEXTBYTE;
      pinfo->g[i] = NEXTBYTE;
      pinfo->b[i] = NEXTBYTE;
    }
  }


  if (!HasColormap && !(Misc&0x80)) {
    /* no global or local colormap */
    return (gifError (pinfo, "No colormap in this GIF file."));
  }
    

  
  /* Start reading the raster data. First we get the intial code size
   * and compute decompressor constant values, based on this code size.
   */
  
  CodeSize = NEXTBYTE;

  ClearCode = (1 << CodeSize);
  EOFCode = ClearCode + 1;
  FreeCode = FirstFree = ClearCode + 2;
  
  /* The GIF spec has it that the code size is the code size used to
   * compute the above values is the code size given in the file, but the
   * code size used in compression/decompression is the code size given in
   * the file plus one. (thus the ++).
   */
  
  CodeSize++;
  InitCodeSize = CodeSize;
  MaxCode = (1 << CodeSize);
  ReadMask = MaxCode - 1;
  


  /* UNBLOCK:
   * Read the raster data.  Here we just transpose it from the GIF array
   * to the Raster array, turning it from a series of blocks into one long
   * data stream, which makes life much easier for ReadCode().
   */
  
  ptr1 = Raster;
  do {
    ch = ch1 = NEXTBYTE;
    while (ch--) { *ptr1 = NEXTBYTE; ptr1++; }
    if ((dataptr - RawGIF) > filesize) {
      break;
    }
  } while(ch1);


  /* Allocate the 'pic' */
  maxpixels = Width*Height;
  picptr = pic8 = (byte *) malloc(maxpixels);
  if (!pic8) return( gifError(pinfo, "couldn't malloc 'pic8'") );

  
  /* Decompress the file, continuing until you see the GIF EOF code.
   * One obvious enhancement is to add checking for corrupt files here.
   */
  
  Code = ReadCode();
  while (Code != EOFCode) {
    /* Clear code sets everything back to its initial value, then reads the
     * immediately subsequent code as uncompressed data.
     */

    if (Code == ClearCode) {
      CodeSize = InitCodeSize;
      MaxCode = (1 << CodeSize);
      ReadMask = MaxCode - 1;
      FreeCode = FirstFree;
      Code = ReadCode();
      CurCode = OldCode = Code;
      FinChar = CurCode & BitMask;
      if (!Interlace) *picptr++ = FinChar;
         else DoInterlace(FinChar);
      npixels++;
    }
    else {
      /* If not a clear code, must be data: save same as CurCode and InCode */

      /* if we're at maxcode and didn't get a clear, stop loading */
      if (FreeCode>=4096) { /* printf("freecode blew up\n"); */
			    break; }

      CurCode = InCode = Code;
      
      /* If greater or equal to FreeCode, not in the hash table yet;
       * repeat the last character decoded
       */
      
      if (CurCode >= FreeCode) {
	CurCode = OldCode;
	if (OutCount > 4096) {  /* printf("outcount1 blew up\n"); */ break; }
	OutCode[OutCount++] = FinChar;
      }
      
      /* Unless this code is raw data, pursue the chain pointed to by CurCode
       * through the hash table to its end; each code in the chain puts its
       * associated output code on the output queue.
       */
      
      while (CurCode > BitMask) {
	if (OutCount > 4096) break;   /* corrupt file */
	OutCode[OutCount++] = Suffix[CurCode];
	CurCode = Prefix[CurCode];
      }
      
      if (OutCount > 4096) { /* printf("outcount blew up\n"); */ break; }
      
      /* The last code in the chain is treated as raw data. */
      
      FinChar = CurCode & BitMask;
      OutCode[OutCount++] = FinChar;
      
      /* Now we put the data out to the Output routine.
       * It's been stacked LIFO, so deal with it that way...
       */

      /* safety thing:  prevent exceeding range of 'pic8' */
      if (npixels + OutCount > maxpixels) OutCount = maxpixels-npixels;
	
      npixels += OutCount;
      if (!Interlace) for (i=OutCount-1; i>=0; i--) *picptr++ = OutCode[i];
                else  for (i=OutCount-1; i>=0; i--) DoInterlace(OutCode[i]);
      OutCount = 0;

      /* Build the hash table on-the-fly. No table is stored in the file. */
      
      Prefix[FreeCode] = OldCode;
      Suffix[FreeCode] = FinChar;
      OldCode = InCode;
      
      /* Point to the next slot in the table.  If we exceed the current
       * MaxCode value, increment the code size unless it's already 12.  If it
       * is, do nothing: the next code decompressed better be CLEAR
       */
      
      FreeCode++;
      if (FreeCode >= MaxCode) {
	if (CodeSize < 12) {
	  CodeSize++;
	  MaxCode *= 2;
	  ReadMask = (1 << CodeSize) - 1;
	}
      }
    }
    Code = ReadCode();
    if (npixels >= maxpixels) break;
  }
  
  if (npixels != maxpixels) {
    if (!Interlace)
      memset(pic8+npixels, 0, maxpixels-npixels);  /* clear to EOBuffer */
  }

  /* fill in the PICINFO structure */

  pinfo->pic     = pic8;
  pinfo->w       = Width;           
  pinfo->h       = Height;

  return 1;
}



/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */

static int ReadCode()
{
  int RawCode, ByteOffset;
  
  ByteOffset = BitOffset / 8;
  RawCode = Raster[ByteOffset] + (Raster[ByteOffset + 1] << 8);
  if (CodeSize >= 8)
    RawCode += ( ((int) Raster[ByteOffset + 2]) << 16);
  RawCode >>= (BitOffset % 8);
  BitOffset += CodeSize;

  return(RawCode & ReadMask);
}


/***************************/
static void DoInterlace(Index)
     byte Index;
{
  static byte *ptr = NULL;
  static int   oldYC = -1;
  
  if (oldYC != YC) {  ptr = pic8 + YC * Width;  oldYC = YC; }
  
  if (YC<Height)
    *ptr++ = Index;
  
  /* Update the X-coordinate, and if it overflows, update the Y-coordinate */
  
  if (++XC == Width) {
    
    /* deal with the interlace as described in the GIF
     * spec.  Put the decoded scan line out to the screen if we haven't gone
     * past the bottom of it
     */
    
    XC = 0;
    
    switch (Pass) {
    case 0:
      YC += 8;
      if (YC >= Height) { Pass++; YC = 4; }
      break;
      
    case 1:
      YC += 8;
      if (YC >= Height) { Pass++; YC = 2; }
      break;
      
    case 2:
      YC += 4;
      if (YC >= Height) { Pass++; YC = 1; }
      break;
      
    case 3:
      YC += 2;  break;
      
    default:
      break;
    }
  }
}


      
/*****************************/
static int gifError(pinfo, st)
     PICINFO *pinfo;
     char    *st;
{
  strcpy (g_errmsg, st);

  if (Raster != NULL) free(Raster);

  if (pinfo->pic) free(pinfo->pic);

  if (pic8 && pic8 != pinfo->pic) free(pic8);

  pinfo->pic = (byte *) NULL;

  return 0;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: explodegif.c,v $ $Date: 2009/01/05 20:55:16 $ $Revision: 1.3 $ $Name:  $"};
