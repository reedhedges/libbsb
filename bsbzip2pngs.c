

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "bsb.h"

#include <zip.h>
#include <png.h>

int bsb_to_png(BSBImage *bsb, const char *filename) 
{
  assert(bsb);
  assert(filename);

  png_structp pngptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  assert(pngptr);
  
  png_infop infoptr = png_create_info_struct(pngptr);
  assert(infoptr);

  size_t len = strlen(filename);
  assert(len >= 4);
  char *pngfilename = (char*)malloc(len+1);
  //strncpy(pngfilename, filename, len);
  strcpy(pngfilename, filename);
  //strncpy(pngfilename + (len-4), ".png", 4);
  strcpy(pngfilename + (len-4), ".png");

  FILE *outfd = fopen(pngfilename, "wb");
  if(!outfd)
  {
    fprintf(stderr, "Error opening %s for writing\n", pngfilename);
    png_destroy_write_struct(&pngptr, &infoptr);
    return 6;
  }
  
  printf("Converting %s to %s...\n", filename, pngfilename);

  png_init_io(pngptr, outfd);

  png_set_IHDR(pngptr, infoptr, bsb->width, bsb->height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  static const size_t ntext = 2;
  png_text text[ntext];
  text[0].key = "Title";
  text[0].text = bsb->name;
  text[0].compression = PNG_TEXT_COMPRESSION_NONE;
  text[1].key = "Generator";
  text[1].text = "bsbzip2pngs";
  text[1].compression = PNG_TEXT_COMPRESSION_NONE;
  // todo add geographic metadata?
  png_set_text(pngptr, infoptr, text, ntext);
  png_write_info(pngptr, infoptr);


  int width = bsb->width;
  int height = bsb->height;
  uint8_t *bsb_row_buf = (uint8_t*)malloc(width);
  uint8_t *png_row_buf = (uint8_t*)malloc(width * bsb->depth);
  for(int row = 0; row < height; row++)
  {
    printf("%.0f%%...  \r", ((float)row/(float)height) * 100.0 );
    bsb_read_row_at(bsb, row, bsb_row_buf);
    for(int bp = 0, pp = 0; bp < width; bp++)
    {
      uint8_t i = bsb_row_buf[bp];
      png_row_buf[pp++] = bsb->red[i];
      png_row_buf[pp++] = bsb->green[i];
      png_row_buf[pp++] = bsb->blue[i];
    }
    png_write_row(pngptr, png_row_buf);
  }
  printf("\n");
  free(bsb_row_buf);
  free(png_row_buf);

  png_write_end(pngptr, NULL);
  fclose(outfd);
  printf("Wrote %s\n", pngfilename);
  free(pngfilename);
  png_destroy_write_struct(&pngptr, &infoptr);
  return 0;
}


int bsb_to_png_from_mem(void *buf, size_t size, const char *filename)
{
  FILE *fp = fmemopen(buf, size, "r");
  if(!fp)
  {
    perror("fmemopen");
    return 5;
  }

  BSBImage bsb;
  int r = bsb_open_fp(fp, &bsb);
  if(!r)  
    return 6;

  r = bsb_to_png(&bsb, filename);

  fclose(fp);
  return 0;
}

int convert_file_from_zip(zip_t *zip, zip_uint64_t index, const char *filename)
{
  zip_file_t *zfp = zip_fopen_index(zip, index, 0);
  if(!zfp)
  {
    fprintf(stderr, "Error opening file \"%s\" inside zip archive: %s\n", filename, zip_strerror(zip));
    return 3;
  }

  struct zip_stat stat;
  int r = zip_stat_index(zip, index, 0, &stat);
  assert(r == 0);
  assert(stat.valid & ZIP_STAT_SIZE);
  
  printf("Uncompressed file \"%s\" will need %.4f MB\n", filename, (stat.size / 1024.0 / 1024.0) );

  if(stat.size == 0)
  {
    fprintf(stderr, "Warning: File \"%s\" in zip has zero size. skipping.\n", filename);
    zip_fclose(zfp);
    return 0;
  }
  
  void *buf = malloc(stat.size);
  if(!buf)
  {
    perror("malloc");
    zip_fclose(zfp);
    return 4;
  }


  zip_int64_t n;
  void *p = buf;
  while( (n = zip_fread(zfp, p, 1024)) > 0 )
  {
    p += n;
  }
  assert( (p - buf) >= 0 && (zip_uint64_t)(p - buf) == stat.size);

  r = bsb_to_png_from_mem(buf, (size_t)stat.size, filename);

  free(buf);
  zip_fclose(zfp);
  return r;
}

int main(int argc, char **argv)
{
  if(argc != 2) 
  {
    fputs("Usage:\n\tbsbzip2pngs input.zip", stderr);
    return 1;
  }

  int err = 0;
  zip_t *zip = zip_open(argv[1], ZIP_RDONLY, &err);
  if(!zip)
  {
    fprintf(stderr, "Error opening zip file %s: %d\n", argv[1], err); //zip_error_strerror(&err));
    return 2;
  }

  zip_int64_t ne = zip_get_num_entries(zip, 0);
  assert(ne >= 0);
  zip_uint64_t n = (zip_uint64_t)ne;
  for(zip_uint64_t i = 0; i < n; ++i)
  {
    const char *path = zip_get_name(zip, i, ZIP_FL_ENC_GUESS);
    assert(path);
    printf("Found \"%s\"\n", path);
    size_t len = strlen(path);
    const char *ext = path + (len - 4);

    if(len > 4 && strncasecmp(ext, ".bsb", 4) == 0)
    {
      printf("Found BSB file \"%s\"\n", path);
      /* TODO read metadata from BSB file */
      continue;
    }

    if(len > 4 && strncasecmp(ext, ".kap", 4) == 0)
    {
      printf("Found KAP file \"%s\"\n", path);

      const char *filename = rindex(path, '/'); 
      if(filename == NULL) filename = path;
      else ++filename;
      int r = convert_file_from_zip(zip, i, filename);
      if(r) break;
      continue;
    }

  }
  
  zip_close(zip);
  
  return 0;
}
