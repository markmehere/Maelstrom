
#include <stdio.h>
#include <string.h>

#include "SDL_endian.h"
#include "Maelstrom_Globals.h"

#include "load.h"
#include "myerror.h"
#include "bitesex.h"
#include "colortable.h"
#include "png.h"


/* For data loading */
char *LibPath::exepath = NULL;

SDL_Surface *Load_Icon(char **xpm)
{
	SDL_Surface *icon;
	int width, height, num_colors, chars_per_pixel;
	int index, i;
	char *buf;
	int b, p;
	Uint8 rgb[3];

	/* Figure out the size of the picture */
	index = 0;
	if ( SDL_sscanf(xpm[index++], "%d %d %d %d", &width, &height, &num_colors, 
						&chars_per_pixel) != 4 ) {
		SDL_SetError("Can't read XPM format");
		return(NULL);
	}

	/* We only support 8-bit images, we punt here */
	if ( chars_per_pixel != 1 ) {
		SDL_SetError("Can't read XPM colors");
		return(NULL);
	}

	/* Allocate a surface of the appropriate type */
	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0,0,0,0);
	if ( icon == NULL ) {
		return(NULL);
	}

	/* Fill in the palette */
	for ( i=0; i<num_colors; ++i ) {
		buf = xpm[index++];
		p = *buf;
		memset(rgb, 0, 3);
		buf += 5;
		for ( b=0; b<6; ++b ) {
			rgb[b/2] *= 16;
			if ( (*buf >= 'a') && (*buf <='f') ) {
				rgb[b/2] += 10+*buf-'a';
			} else
			if ( (*buf >= 'A') && (*buf <='F') ) {
				rgb[b/2] += 10+*buf-'A';
			} else
			if ( (*buf >= '0') && (*buf <='9') ) {
				rgb[b/2] += *buf-'0';
			}
			++buf;
		}
		icon->format->palette->colors[p].r = rgb[0];
		icon->format->palette->colors[p].g = rgb[1];
		icon->format->palette->colors[p].b = rgb[2];
	}

	/* Fill in the pixels */
	buf = (char *)icon->pixels;
	for ( i=0; i<height; ++i ) {
		memcpy(buf, xpm[index++], width);
		buf += icon->pitch;
	}
	return(icon);
}

SDL_Surface *Load_Title(FrameBuf *screen, int title_id)
{
	char file[256];
	LibPath path;
	SDL_Surface *bmp, *title;
	
	/* Open the title file -- we know its colormap is our global one */
	SDL_snprintf(file, sizeof(file), "Images" DIR_SEP "Maelstrom_Titles#%d.bmp", title_id);
	bmp = SDL_LoadBMP(path.Path(file));
	if ( bmp == NULL ) {
		return(NULL);
	}

	/* Create an image from the BMP */
	title = screen->LoadImage(bmp->w, bmp->h, (Uint8 *)bmp->pixels, NULL);
	SDL_FreeSurface(bmp);
	return(title);
}


Uint8 **rows, **ccrows = NULL, ind = 0;
png_structp png_ptr;
png_infop info_ptr;


void SaveCIcons()
{
	char pngfile[256];
	SDL_snprintf(pngfile, sizeof(pngfile), "/tmp/Maelstrom_Icons.png");

	// create file
	FILE *fp = fopen(pngfile, "wb");
	if (!fp)
		error("[write_png_file] File %s could not be opened for writing", pngfile);

	// initialize stuff
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		error("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		error("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		error("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);

	// write header
	if (setjmp(png_jmpbuf(png_ptr)))
		error("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, 8 * 10, 8, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// write bytes
	if (setjmp(png_jmpbuf(png_ptr)))
		error("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, ccrows);

	// end write
	if (setjmp(png_jmpbuf(png_ptr)))
		error("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	fclose(fp);

	for (int r = 0; r < 32; r++) {
		delete[] ccrows[r];
	}
	delete ccrows;
}

SDL_Surface *GetCIcon(FrameBuf *screen, short cicn_id)
{
	char file[256];
	LibPath path;
	SDL_Surface *cicn;
	SDL_RWops *cicn_src;
	Uint8 *pixels, *mask;
	Uint16 w, h;

	if (!ccrows) {
		ccrows = new Uint8 *[8 * 6];
		for (int r = 0; r < 8; r++) {
			ccrows[r] = new Uint8[8 * 10 * 4]();
		}
	}
	
	/* Open the cicn sprite file.. */
	SDL_snprintf(file, sizeof(file), "Images" DIR_SEP "Maelstrom_Icon#%hd.cicn", cicn_id);
	if ( (cicn_src=SDL_RWFromFile(path.Path(file), "r")) == NULL ) {
		error("GetCIcon(%hd): Can't open CICN %s: ",
					cicn_id, path.Path(file));
		return(NULL);
	}

	w = SDL_ReadBE16(cicn_src);
	h = SDL_ReadBE16(cicn_src);
	pixels = new Uint8[w*h];
	if ( SDL_RWread(cicn_src, pixels, 1, w*h) != (w*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
	mask = new Uint8[(w/8)*h];
	if ( SDL_RWread(cicn_src, mask, 1, (w/8)*h) != ((w/8)*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		delete[] mask;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
	
	if (w == 8) {
		rows = new Uint8 *[8];
		for (int r = 0; r < h; r++) {
			rows[r] = &pixels[r * w];
			for (int p = 0; p < w; p++) {
				ccrows[r + (ind / 10) * 8][(p + (ind % 10) * 8) * 4] = colors[gGammaCorrect][rows[r][p]].r;
				ccrows[r + (ind / 10) * 8][(p + (ind % 10) * 8) * 4 + 1] = colors[gGammaCorrect][rows[r][p]].g;
				ccrows[r + (ind / 10) * 8][(p + (ind % 10) * 8) * 4 + 2] = colors[gGammaCorrect][rows[r][p]].b;
				ccrows[r + (ind / 10) * 8][(p + (ind % 10) * 8) * 4 + 3] = 0xFF;
			}
		}
		ind += 1;
		delete[] rows;
	}
	SDL_RWclose(cicn_src);

	cicn = screen->LoadImage(w, h, pixels, mask);
	delete[] pixels;
	delete[] mask;
	if ( cicn == NULL ) {
		error("GetCIcon(%hd): Couldn't convert CICN!\n", cicn_id);
	}
	return(cicn);
}
