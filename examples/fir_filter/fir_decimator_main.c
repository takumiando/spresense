/****************************************************************************
 * examples/fir_filter/fir_decimation_main.c
 *
 *   Copyright 2021 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <digital_filter/fir_decimator.h>

#include "wav.h"

#define BLOCK_SIZE  (256)

static float si_data[BLOCK_SIZE];
static float so_data[BLOCK_SIZE];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void print_usage(const char *appname)
{
  printf("Usage: %s <input wav file> <Decimation Factor>"
      " <TransitionWidth or TapNum> <output wav file>\n", appname);
  printf("       \"TransitionWidth or TapNum\" : it means tap number if it is "
      "less than 1000.\n        And it means Tanssition band width(Hz)"
      " if it is over 1000.\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main()
 *
 * Description:
 *   main routine of this example.
 ****************************************************************************/

int main(int argc, char **argv)
{
  decimator_instancef_t *dec;
  wav_instance_t input_wav;
  wav_instance_t output_wav;
  int n;
  int dec_factor;
  int dec_count;
  int dec_total;
  int param;

  if (argc != 5)
    {
      print_usage(argv[0]);
      return -1;
    }

  /* Decimation factor */

  dec_factor = atoi(argv[2]);

  /* Transition Width (>=1000) or Tap number (<1000 and >0) or no filter (=0) */

  param = atoi(argv[3]);

  /* Open input wav file */

  if (open_input_wavfile(&input_wav, argv[1]))
    {
      printf("Error: no such file <%s>\n", argv[1]);
      print_usage(argv[0]);
      return -1;
    }

  /* Create Decimation filter */

  if (param >= 1000)
    {
      dec = create_decimatorf(input_wav.fs, dec_factor, param, BLOCK_SIZE);
    }
  else
    {
      dec = create_decimatorf_tap(input_wav.fs, dec_factor, param, BLOCK_SIZE);
    }

  if (dec == NULL)
    {
      printf("Could not create decimator.\n");
      close_wavfile(&input_wav);
      return -1;
    }

  printf("Decimation Factor = %d\n", dec_factor);
  printf("Filter Block size = %d\n", BLOCK_SIZE);
  printf("Filter tap number = %d\n", decimator_tapnumf(dec));

  output_wav.fs     = input_wav.fs / dec_factor;
  output_wav.bits   = input_wav.bits;
  output_wav.length = input_wav.length / dec_factor;

  printf("Sampling rate = %d(Hz)\n", input_wav.fs);
  printf("Sampling length = %d\n\n", input_wav.length);

  printf("Output Sampling rate = %d\n", output_wav.fs);
  printf("Output Sampling length = %d\n", output_wav.length);
  printf("Saving filename : %s\n", argv[4]);

  /* Open output wav file */

  if (open_output_wavfile(&output_wav, argv[4]))
    {
      printf("Error : could not open output file..\n");
      decimator_deletef(dec);
      close_wavfile(&input_wav);
      return -1;
    }

  dec_total = 0;
  for (n = 0; n < (input_wav.length - BLOCK_SIZE + 1); n += BLOCK_SIZE)
    {
      /* Load PCM data from input wav file */

      read_wavdata(&input_wav, si_data, BLOCK_SIZE);

      /* Execute Decimation */

      dec_count = decimator_executef(dec, si_data, BLOCK_SIZE,
          so_data, BLOCK_SIZE);

      /* Save decimated data into output wav file */

      write_wavdata(&output_wav, so_data, dec_count);

      dec_total += dec_count;
    }

  close_wavfile(&output_wav);
  close_wavfile(&input_wav);

  decimator_deletef(dec);

  printf("Actual decimated data length = %d\n", dec_total);
  printf("Done\n");

  return 0;
}
