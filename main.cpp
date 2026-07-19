/*
Copyright 2005, 2006, 2007 Dennis van Weeren
Copyright 2008, 2009 Jakub Bednarski
Copyright 2012 Till Harbaum

This file is part of Minimig

Minimig is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Minimig is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cd.h"
#include "crc32.h"
#include "fpga_io.h"
#include "frame_timer.h"
#include "input.h"
#include "menu.h"
#include "offload.h"
#include "osd.h"
#include "scheduler.h"
#include "user_io.h"
#include <assert.h>
#include <ctype.h>
#include <glob.h>
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

/*
Keep in mind that (table->tracks[table->last].f.size is missing!
table->end will have wrong values compared to running on the MiSTer
*/

const char *version = "$VER:" VDATE;

int cdi_load_cue(const char *filename, toc_t *table);
void prepare_toc_buffer(toc_t *toc);
int cdi_load_chd(const char *filename, toc_t *table);
void cdi_read_cd(uint8_t *buffer, int lba, int cnt);
toc_t *cdi_toc();

#define CDI_SECTOR_LEN 2352
#define CDI_SUBCHANNEL_LEN ((12 + 96) * 2)
#define CDI_CDIC_BUFFER_SIZE (CDI_SECTOR_LEN + CDI_SUBCHANNEL_LEN)

uint32_t table[256];

int main(int argc, char *argv[]) {
  crc32::generate_table(table);

#if 0
	glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

	glob("cues/*.cue", GLOB_TILDE, NULL, &glob_result);

    for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
		auto filename = std::string(glob_result.gl_pathv[i]);
		assert(cdi_load_cue(filename.c_str(), &table)==1);
		prepare_toc_buffer(&table);
	}
#endif

#if 1
  assert(cdi_load_chd("/home/andre/Downloads/inxs/INXS - Listen Like Thieves "
                      "(USA)/INXS - Listen Like Thieves (USA).chd",
                      cdi_toc()));
  FILE *f = fopen("chdout.bin", "wb");
#endif

#if 0
  assert(cdi_load_cue("/home/andre/Downloads/inxs/INXS - Listen Like Thieves "
                      "(USA)/barf.cue",
                      cdi_toc()) == 1);
  FILE *f = fopen("cuesingle.bin", "wb");
#endif

#if 0
  assert(cdi_load_cue("/home/andre/Downloads/inxs/INXS - Listen Like Thieves "
                      "(USA)/INXS - Listen Like Thieves (USA).cue",
                      cdi_toc()) == 1);
  FILE *f = fopen("cuemulti.bin", "wb");
#endif

  uint8_t buffer[CDI_CDIC_BUFFER_SIZE * 6];

  // return 0;

#if 0
  int lba_start = 16372;
  int lba_end = 16372+5; //167845;
#else
  int lba_start = 0;
  int lba_end = 167845;
#endif

  int sectors_per_read = 1;
  int expected_bytes_per_request = CDI_CDIC_BUFFER_SIZE * sectors_per_read;
  int file_offset = 0;

  for (int lba = lba_start; lba < lba_end; lba += sectors_per_read) {
    cdi_read_cd(buffer, lba, sectors_per_read);
    uint32_t crc = crc32::update(table, 0, buffer, expected_bytes_per_request);
    printf("Block %d at %x with CRC %x\n", lba, file_offset, crc);

#if 0
    int bytes = fwrite(buffer, 1, expected_bytes_per_request, f);
    assert(bytes == expected_bytes_per_request);
#endif
    file_offset += expected_bytes_per_request;
  }
  fclose(f);
}
