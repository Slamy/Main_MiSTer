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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>
#include "menu.h"
#include "user_io.h"
#include "input.h"
#include "frame_timer.h"
#include "fpga_io.h"
#include "scheduler.h"
#include "osd.h"
#include "offload.h"
#include "cd.h"
#include <glob.h>
#include <string>
#include <assert.h>

/*
Keep in mind that (table->tracks[table->last].f.size is missing!
table->end will have wrong values compared to running on the MiSTer
*/

const char *version = "$VER:" VDATE;

toc_t table;
int cdi_load_cue(const char *filename, toc_t *table);
void prepare_toc_buffer(toc_t* toc);

int main(int argc, char *argv[])
{
	glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

	glob("cues/*.cue", GLOB_TILDE, NULL, &glob_result);

    for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
		auto filename = std::string(glob_result.gl_pathv[i]);
		assert(cdi_load_cue(filename.c_str(), &table)==1);
		prepare_toc_buffer(&table);
	}
}
