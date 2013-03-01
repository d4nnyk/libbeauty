/*
 *  Copyright (C) 2004-2009 The libbeauty Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * 11-9-2004 Initial work.
 *   Copyright (C) 2004 James Courtier-Dutton James@superbug.co.uk
 * 10-10-2009 Updates.
 *   Copyright (C) 2009 James Courtier-Dutton James@superbug.co.uk
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

#include <rev.h>
#include <bfl.h>

/* The symbol table.  */
//static asymbol **syms;

/* Number of symbols in `syms'.  */
//static long symcount = 0;

static void insert_section(struct bfd *b, asection *sect, void *obj)
{
	struct rev_eng *r = obj;
        debug_print(DEBUG_INPUT_BFD, 1, "Section entered\n");
	r->section[r->section_sz++] = sect;
}

static void print_sections(struct rev_eng* ret)
{
	char *comma;
	unsigned int       opb = bfd_octets_per_byte (ret->bfd);
	asection          *section;
	int n;

	printf("bfd:print_sections: 0x%"PRIx64" sections\n", ret->section_sz);
	for (n = 0; n < ret->section_sz; n++) {
		comma = "";
		section = ret->section[n];
		printf ("%3d %-13s %08lx  ", section->index,
		bfd_get_section_name (ret->bfd, section),
			(unsigned long) bfd_section_size (ret->bfd, section) / opb);
		bfd_printf_vma (ret->bfd, bfd_get_section_vma (ret->bfd, section));
		printf("  ");
		bfd_printf_vma (ret->bfd, section->lma);
		printf("  %08lx  2**%u", (unsigned long) section->filepos,
		bfd_get_section_alignment (ret->bfd, section));
		printf("\n                ");
		printf("  ");

#define PF(x, y) \
	if (section->flags & x) { printf ("%s%s", comma, y); comma = ", "; }

		PF (SEC_HAS_CONTENTS, "CONTENTS");
		PF (SEC_ALLOC, "ALLOC");
		PF (SEC_LOAD, "LOAD");
		PF (SEC_RELOC, "RELOC");
		PF (SEC_READONLY, "READONLY");
		PF (SEC_CODE, "CODE");
		PF (SEC_DATA, "DATA");
		PF (SEC_ROM, "ROM");
		PF (SEC_CONSTRUCTOR, "CONSTRUCTOR");
		PF (SEC_NEVER_LOAD, "NEVER_LOAD");
		PF (SEC_THREAD_LOCAL, "THREAD_LOCAL");
		PF (SEC_HAS_GOT_REF, "GOT_REF");
		PF (SEC_IS_COMMON, "IS_COMMON");
		PF (SEC_DEBUGGING, "DEBUGGING");
		PF (SEC_IN_MEMORY, "IN_MEMORY");
		PF (SEC_EXCLUDE, "EXCLUDE");
		PF (SEC_SORT_ENTRIES, "SORT_ENTRIES");
		PF (SEC_LINK_ONCE, "LINK_ONCE");
		PF (SEC_LINK_DUPLICATES, "LINK_DUPLICATES");
		PF (SEC_LINK_DUPLICATES_ONE_ONLY, "LINK_DUPLICATES_ONE_ONLY");
		PF (SEC_LINK_DUPLICATES_SAME_SIZE, "LINK_DUPLICATES_SAME_SIZE");
		PF (SEC_LINKER_CREATED, "LINKER_CREATED");
		PF (SEC_KEEP, "KEEP");
		PF (SEC_SMALL_DATA, "SMALL_DATA");
		PF (SEC_MERGE, "MERGE");
		PF (SEC_STRINGS, "STRINGS");
		PF (SEC_GROUP, "GROUP");
		PF (SEC_COFF_SHARED_LIBRARY, "COFF_SHARED_LIBRARY");
		PF (SEC_COFF_SHARED, "COFF_SHARED");
		PF (SEC_TIC54X_BLOCK, "TIC54X_BLOCK");
		PF (SEC_TIC54X_CLINK, "TIC54X_CLINK");

		/*      if (section->comdat != NULL)
		 *       printf (" (COMDAT %s %ld)", section->comdat->name,
		 *               section->comdat->symbol);
		 */

		comma = ", ";

		printf ("\n");
	}
	#undef PF
}

#if 0
static void print_code_section(struct rev_eng* ret)
{
  asection          *section = ret->section[0];
  int                n;
  bfd_byte          *data = NULL;
  bfd_size_type      datasize = 0;

  datasize = bfd_get_section_size(section);
  if (datasize == 0)
    return;
  data = malloc(datasize);
  bfd_get_section_contents(ret->bfd, section, data, 0, datasize);
  for(n=0;n<datasize;n++) {
    debug_print(DEBUG_INPUT_BFD, 1, "0x%x ",data[n]);
  }
  debug_print(DEBUG_INPUT_BFD, 1, "\n");
  free(data);
  data = NULL;
}
#endif

int bf_find_section(struct rev_eng* ret, char *name, int name_len, int *section_number)
{
	int n;
	int found = 0;
	*section_number = 0;

	for (n = 0; n < ret->section_sz; n++) {
		/* The + 1 is there to ensure both strings have zero terminators */
		if (!strncmp(ret->section[n]->name, name, name_len + 1)) {
			debug_print(DEBUG_INPUT_BFD, 1, "bf_find_section %s\n", ret->section[n]->name);
			found = 1;
			*section_number = n;
			break;
		}
	}
	return found;
}


int64_t bf_get_code_size(struct rev_eng* ret)
{
	asection          *section = ret->section[0];
	bfd_size_type      datasize = 0;
	int64_t            code_size = 0;
	int n;
	int tmp;

	tmp = bf_find_section(ret, ".text", 5, &n);
	
	if (tmp) {
		section = ret->section[n];
		datasize = bfd_get_section_size(section);
		code_size = datasize;
	}
	return code_size;
}

int64_t bf_get_data_size(struct rev_eng* ret)
{
	asection          *section = ret->section[1];
	bfd_size_type      datasize = 0;
	int64_t            code_size = 0;
	int n;
	int tmp;

	tmp = bf_find_section(ret, ".data", 5, &n);
	
	if (tmp) {
		section = ret->section[n];
		datasize = bfd_get_section_size(section);
		code_size = datasize;
	}
	return code_size;
}

int64_t bf_get_rodata_size(struct rev_eng* ret)
{
	asection          *section = ret->section[1];
	bfd_size_type      datasize = 0;
	int64_t            code_size = 0;
	int n;
	int tmp;

	tmp = bf_find_section(ret, ".rodata", 7, &n);
	
	if (tmp) {
		section = ret->section[n];
		datasize = bfd_get_section_size(section);
		code_size = datasize;
	}
	return code_size;
}

int bf_get_reloc_table_size_code_section(struct rev_eng* ret, uint64_t *size)
{
	asection          *section = ret->section[0];
	bfd_size_type      datasize = *size;

	datasize = bfd_get_reloc_upper_bound(ret->bfd, section);
	*size = datasize;
	return 1;
}

static void
dump_reloc_set (bfd *abfd, asection *sec, arelent **relpp, long relcount)
{
  arelent **p;
//  char *last_filename, *last_functionname;
//  unsigned int last_line;

  /* Get column headers lined up reasonably.  */
  {
    static int width;

    if (width == 0)
      {
	char buf[30];

	bfd_sprintf_vma (abfd, buf, (bfd_vma) -1);
	width = strlen (buf) - 7;
      }
    printf ("OFFSET %*s TYPE %*s VALUE \n", width, "", 12, "");
  }

//  last_filename = NULL;
//  last_functionname = NULL;
//  last_line = 0;

  for (p = relpp; relcount && *p != NULL; p++, relcount--)
    {
      arelent *q = *p;
      const char *sym_name;
      const char *section_name;

      if (q->sym_ptr_ptr && *q->sym_ptr_ptr)
	{
	  sym_name = (*(q->sym_ptr_ptr))->name;
	  section_name = (*(q->sym_ptr_ptr))->section->name;
	}
      else
	{
	  sym_name = NULL;
	  section_name = NULL;
	}

      bfd_printf_vma (abfd, q->address);
      if (q->howto == NULL)
	printf (" *unknown*         ");
      else if (q->howto->name)
	printf (" %-16s  ", q->howto->name);
      else
	printf (" %-16d  ", q->howto->type);
      if (sym_name)
	debug_print(DEBUG_INPUT_BFD, 1, "sym_name: %s\n", sym_name);
//	objdump_print_symname (abfd, NULL, *q->sym_ptr_ptr);
      else
	{
	  if (section_name == NULL)
	    section_name = "*unknown*";
	  printf ("[%s]", section_name);
	}

      if (q->addend)
	{
	  printf ("+0x");
	  bfd_printf_vma (abfd, q->addend);
	}

      printf ("\n");
    }
}

int bf_get_reloc_table_code_section(struct rev_eng* ret)
{
	/* FIXME: search for .text section instead of selecting 0 */
	asection	*section;
	asection	*sym_sec;
	bfd_size_type	datasize;
	arelent		**relpp;
	arelent		*rel;
	uint64_t relcount;
	int n;
	int tmp;
	const char *sym_name;
	uint64_t sym_val;

	tmp = bf_find_section(ret, ".text", 5, &n);
	//debug_print(DEBUG_INPUT_BFD, 1, "%s: section = 0x%x\n", __FUNCTION__, n);
	section = ret->section[n];

	datasize = bfd_get_reloc_upper_bound(ret->bfd, section);
	relpp = malloc (datasize);
	/* This function silently fails if ret->symtab is not set
	 * to an already loaded symbol table.
	 */
	relcount = bfd_canonicalize_reloc(ret->bfd, section, relpp, ret->symtab);
	//debug_print(DEBUG_INPUT_BFD, 1, "Relcount=0x%"PRIx64"\n", relcount);
	ret->reloc_table_code = calloc(relcount, sizeof(*ret->reloc_table_code));
	ret->reloc_table_code_sz = relcount;
	//debug_print(DEBUG_INPUT_BFD, 1, "reloc_size=0x%"PRIx64"\n", sizeof(*ret->reloc_table_code));
	//dump_reloc_set (ret->bfd, section, relpp, relcount);
	for (n=0; n < relcount; n++) {
		rel = relpp[n];
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:addr = 0x%"PRIx64"\n", rel->address);
		ret->reloc_table_code[n].address = rel->address;
		ret->reloc_table_code[n].size = (uint64_t) bfd_get_reloc_size (rel->howto);
		ret->reloc_table_code[n].value = rel->addend;
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:size = 0x%"PRIx64"\n", (uint64_t) bfd_get_reloc_size (rel->howto));
		//if (rel->howto == NULL)
		//	printf (" howto *unknown*\n");
		//else if (rel->howto->name)
		//	printf (" howto->name %-16s\n", rel->howto->name);
		//else
		//	printf (" howto->type %-16d\n", rel->howto->type);

		//debug_print(DEBUG_INPUT_BFD, 1, "p1 %p\n",&rel->sym_ptr_ptr);
		//debug_print(DEBUG_INPUT_BFD, 1, "p2 %p\n",rel->sym_ptr_ptr);
		if (rel->sym_ptr_ptr == NULL) {
			continue;
		}
		
		sym_name = bfd_asymbol_name(*rel->sym_ptr_ptr);
		sym_val = bfd_asymbol_value(*rel->sym_ptr_ptr);
		sym_sec = bfd_get_section(*rel->sym_ptr_ptr);
		ret->reloc_table_code[n].section_index = sym_sec->index;
		ret->reloc_table_code[n].section_name = sym_sec->name;
		ret->reloc_table_code[n].symbol_name = sym_name;
		
		//printf ("sym_name = %s, sym_val = 0x%"PRIx64"\n",sym_name, sym_val);

	}
	free(relpp);
	return 1;
}

int bf_get_reloc_table_data_section(struct rev_eng* ret)
{
	asection	*section;
	asection	*sym_sec;
	bfd_size_type	datasize;
	arelent		**relpp;
	arelent		*rel;
	uint64_t relcount;
	int n;
	int tmp;
	const char *sym_name;
	uint64_t sym_val;

	tmp = bf_find_section(ret, ".data", 5, &n);
	//debug_print(DEBUG_INPUT_BFD, 1, "%s: section = 0x%x\n", __FUNCTION__, n);
	section = ret->section[n];

	datasize = bfd_get_reloc_upper_bound(ret->bfd, section);
	relpp = malloc (datasize);
	/* This function silently fails if ret->symtab is not set
	 * to an already loaded symbol table.
	 */
	relcount = bfd_canonicalize_reloc(ret->bfd, section, relpp, ret->symtab);
	//debug_print(DEBUG_INPUT_BFD, 1, "relcount=0x%"PRIx64"\n", relcount);
	ret->reloc_table_data = calloc(relcount, sizeof(*ret->reloc_table_data));
	ret->reloc_table_data_sz = relcount;
	//debug_print(DEBUG_INPUT_BFD, 1, "reloc_size=%d\n", sizeof(*ret->reloc_table));
	//dump_reloc_set (ret->bfd, section, relpp, relcount);
	for (n=0; n < relcount; n++) {
		rel = relpp[n];
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:addr = 0x%"PRIx64"\n", rel->address);
		ret->reloc_table_data[n].address = rel->address;
		ret->reloc_table_data[n].size = (uint64_t) bfd_get_reloc_size (rel->howto);
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:size = 0x%"PRIx64"\n", (uint64_t) bfd_get_reloc_size (rel->howto));
		//if (rel->howto == NULL)
		//	printf (" *unknown*\n");
		//else if (rel->howto->name)
		//	printf (" %-16s\n", rel->howto->name);
		//else
		//	printf (" %-16d\n", rel->howto->type);

		//debug_print(DEBUG_INPUT_BFD, 1, "p1 %p\n",&rel->sym_ptr_ptr);
		//debug_print(DEBUG_INPUT_BFD, 1, "p2 %p\n",rel->sym_ptr_ptr);
		if (rel->sym_ptr_ptr == NULL) {
			continue;
		}
		
		//sym_name = bfd_asymbol_name(*rel->sym_ptr_ptr);
		sym_name = bfd_asymbol_name(*rel->sym_ptr_ptr);
		sym_val = bfd_asymbol_value(*rel->sym_ptr_ptr);
		sym_sec = bfd_get_section(*rel->sym_ptr_ptr);
		ret->reloc_table_data[n].section_index = sym_sec->index;
		ret->reloc_table_data[n].section_name = sym_sec->name;
		ret->reloc_table_data[n].symbol_name = sym_name;
		
		//printf (" %i, %s\n",sym_sec->index, sym_name);

	}
	free(relpp);
	return 1;
}

int bf_get_reloc_table_rodata_section(struct rev_eng* ret)
{
	asection	*section;
	asection	*sym_sec;
	bfd_size_type	datasize;
	arelent		**relpp;
	arelent		*rel;
	uint64_t relcount;
	int n;
	int tmp;
	const char *sym_name;
	uint64_t sym_val;

	tmp = bf_find_section(ret, ".rodata", 7, &n);
	//debug_print(DEBUG_INPUT_BFD, 1, "%s: section = 0x%x\n", __FUNCTION__, n);
	section = ret->section[n];

	datasize = bfd_get_reloc_upper_bound(ret->bfd, section);
	relpp = malloc (datasize);
	/* This function silently fails if ret->symtab is not set
	 * to an already loaded symbol table.
	 */
	relcount = bfd_canonicalize_reloc(ret->bfd, section, relpp, ret->symtab);
	//debug_print(DEBUG_INPUT_BFD, 1, "relcount=0x%"PRIx64"\n", relcount);
	ret->reloc_table_rodata = calloc(relcount, sizeof(*ret->reloc_table_rodata));
	ret->reloc_table_rodata_sz = relcount;
	//debug_print(DEBUG_INPUT_BFD, 1, "reloc_size=%d\n", sizeof(*ret->reloc_table));
	//dump_reloc_set (ret->bfd, section, relpp, relcount);
	for (n=0; n < relcount; n++) {
		rel = relpp[n];
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:addr = 0x%"PRIx64"\n", rel->address);
		ret->reloc_table_rodata[n].address = rel->address;
		ret->reloc_table_rodata[n].size = (uint64_t) bfd_get_reloc_size (rel->howto);
		ret->reloc_table_rodata[n].value = rel->addend;
		//debug_print(DEBUG_INPUT_BFD, 1, "rel:size = 0x%"PRIx64"\n", (uint64_t) bfd_get_reloc_size (rel->howto));
		//debug_print(DEBUG_INPUT_BFD, 1, "value 0x%"PRIx64"\n", rel->addend);
//		if (rel->howto == NULL)
//			printf ("howto *unknown*\n");
//		else if (rel->howto->name)
//			printf ("howto %-16s\n", rel->howto->name);
//		else
//			printf ("howto %-16d\n", rel->howto->type);

//		debug_print(DEBUG_INPUT_BFD, 1, "p1 %p\n",&rel->sym_ptr_ptr);
//		debug_print(DEBUG_INPUT_BFD, 1, "p2 %p\n",rel->sym_ptr_ptr);
		if (rel->sym_ptr_ptr == NULL) {
			continue;
		}
		
		sym_name = bfd_asymbol_name(*rel->sym_ptr_ptr);
		sym_val = bfd_asymbol_value(*rel->sym_ptr_ptr);
		sym_sec = bfd_get_section(*rel->sym_ptr_ptr);
		ret->reloc_table_rodata[n].section_index = sym_sec->index;
		ret->reloc_table_rodata[n].section_name = sym_sec->name;
		ret->reloc_table_rodata[n].symbol_name = sym_name;
		
		//printf (" %i, %s\n",sym_sec->index, sym_name);

	}
	free(relpp);
	return 1;
}

int bf_copy_code_section(struct rev_eng* ret, uint8_t *data, uint64_t data_size)
{
	asection	*section;
	bfd_size_type	datasize = data_size;
	int		n, tmp;
	int 		result = 0;

	if (!ret)
		return 0;

	tmp = bf_find_section(ret, ".text", 5, &n);
	
	if (tmp) {
		section = ret->section[n];
		bfd_get_section_contents(ret->bfd, section, data, 0, datasize);
		debug_print(DEBUG_INPUT_BFD, 1, "Text Data at %p\n",data);
		result = 1;
	}
	return result;
}

int bf_copy_data_section(struct rev_eng* ret, uint8_t *data, uint64_t data_size)
{
	asection	*section;
	bfd_size_type	datasize = data_size;
	int		n, tmp;
	int 		result = 0;

	if (!ret)
		return 0;

	tmp = bf_find_section(ret, ".data", 5, &n);
	
	if (tmp) {
		section = ret->section[n];
		bfd_get_section_contents(ret->bfd, section, data, 0, datasize);
		debug_print(DEBUG_INPUT_BFD, 1, "Data at %p\n",data);
		result = 1;
	}
	return result;
}

int bf_copy_rodata_section(struct rev_eng* ret, uint8_t *data, uint64_t data_size)
{
	asection	*section;
	bfd_size_type	datasize = data_size;
	int		n, tmp;
	int 		result = 0;

	if (!ret)
		return 0;

	tmp = bf_find_section(ret, ".rodata", 7, &n);
	
	if (tmp) {
		section = ret->section[n];
		bfd_get_section_contents(ret->bfd, section, data, 0, datasize);
		debug_print(DEBUG_INPUT_BFD, 1, "ROData at %p\n",data);
		result = 1;
	}
	return result;
}

const char *bfd_err(void)
{
	return bfd_errmsg(bfd_get_error());
}

int bf_get_arch_mach(struct rev_eng *handle, uint32_t *arch, uint64_t *mach)
{
	bfd *b;

	if (!handle) {
		return 1;
	}
	
	b = handle->bfd;
	debug_print(DEBUG_INPUT_BFD, 1, "format:%"PRIu32", %"PRIu64"\n",bfd_get_arch(b), bfd_get_mach(b));
	*arch = bfd_get_arch(b);
	*mach = bfd_get_mach(b);
	return 0;
}


struct rev_eng *bf_test_open_file(const char *fn)
{
	struct rev_eng *ret;
	int64_t tmp;
	bfd *b;
	char **matching;
	int result;
	int64_t storage_needed;
	int64_t number_of_symbols;
	//symbol_info sym_info;

        debug_print(DEBUG_INPUT_BFD, 1, "Open entered\n");
	/* Open the file with libbfd */
	b = bfd_openr(fn, NULL);
	if ( b == NULL ) {
		debug_print(DEBUG_INPUT_BFD, 1, "Error opening %s:\n%s",
				fn, bfd_err());
		return NULL;
	}
	result = bfd_check_format_matches (b, bfd_object, &matching);
	debug_print(DEBUG_INPUT_BFD, 1, "check format result=%d, file format=%s\n",result, b->xvec->name);
	debug_print(DEBUG_INPUT_BFD, 1, "format:%"PRIu32", %"PRIu64"\n",bfd_get_arch(b), bfd_get_mach(b));
	debug_print(DEBUG_INPUT_BFD, 1, "arch:%"PRIu32", mach64:%"PRIu32", mach32:%"PRIu32"\n",bfd_arch_i386, bfd_mach_x86_64, bfd_mach_i386_i386);

	if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	{
		debug_print(DEBUG_INPUT_BFD, 1, "Couldn't determine format of %s:%s\n",
				fn, bfd_err());
		bfd_close(b);
		return NULL;
	}
/*
		nonfatal (bfd_get_filename (abfd));
		list_matching_formats (matching);
		free (matching);
		return;
	}

  if (bfd_get_error () != bfd_error_file_not_recognized)
    {
      nonfatal (bfd_get_filename (abfd));
      return;
    }

  if (bfd_check_format_matches (abfd, bfd_core, &matching))
    {
      dump_bfd (abfd);
      return;
    }
*/

	/* Check it's an object file and not a core dump, or
	 * archive file or whatever else...
	 */
	if ( !bfd_check_format(b, bfd_object) ) {
		debug_print(DEBUG_INPUT_BFD, 1, "Couldn't determine format of %s:%s\n",
				fn, bfd_err());
		bfd_close(b);
		return NULL;
	}

	/* Create our structure */
	ret = calloc(1, sizeof(*ret));
	if ( ret == NULL ) {
		debug_print(DEBUG_INPUT_BFD, 1, "Couldn't calloc struct rev_eng\n");
		bfd_close(b);
		return NULL;
        }

	ret->bfd = b;

	tmp = bfd_count_sections(ret->bfd);
	if ( tmp <= 0 ) {
          debug_print(DEBUG_INPUT_BFD, 1, "Couldn't count sections\n");
          bfd_close(b);
          return NULL;
        }
	ret->section = calloc(tmp, sizeof(*ret->section));
	if ( ret->section == NULL ) {
          debug_print(DEBUG_INPUT_BFD, 1, "Couldn't calloc struct ret->section\n");
          bfd_close(b);
          return NULL;
        }
	bfd_map_over_sections(ret->bfd, insert_section, ret);
	print_sections(ret);
/*
	print_code_section(ret);
*/
	storage_needed  = bfd_get_symtab_upper_bound(ret->bfd);
	debug_print(DEBUG_INPUT_BFD, 1, "symtab_upper_bound = %"PRId64"\n", storage_needed);
	ret->symtab = calloc(1, storage_needed);
	debug_print(DEBUG_INPUT_BFD, 1, "symtab = %p\n", ret->symtab);
	number_of_symbols = bfd_canonicalize_symtab(ret->bfd, ret->symtab);
	ret->symtab_sz = number_of_symbols;
	debug_print(DEBUG_INPUT_BFD, 1, "symtab_canon = %"PRId64"\n", number_of_symbols);
#if 0
	for (l = 0; l < number_of_symbols; l++) {
		debug_print(DEBUG_INPUT_BFD, 1, "%"PRId64"\n", l);
		debug_print(DEBUG_INPUT_BFD, 1, "type:0x%02x\n", ret->symtab[l]->flags);
		debug_print(DEBUG_INPUT_BFD, 1, "name:%s\n", ret->symtab[l]->name);
		debug_print(DEBUG_INPUT_BFD, 1, "value=0x%02"PRIx64"\n", ret->symtab[l]->value);
		//debug_print(DEBUG_INPUT_BFD, 1, "value2=0x%02x\n",
		//	bfd_asymbol_flavour(ret->symtab[l]));
		//debug_print(DEBUG_INPUT_BFD, 1, "value3=0x%02x\n",
		//	bfd_asymbol_base(ret->symtab[l]));
#if 0
		debug_print(DEBUG_INPUT_BFD, 1, "%d:0x%02x:%s=%lld\n",
			n, sym_info.type, sym_info.name, sym_info.value);
#endif
		/* Print the "other" value for a symbol.  For common symbols,
		 * we've already printed the size; now print the alignment.
		 * For other symbols, we have no specified alignment, and
		 * we've printed the address; now print the size.  */
#if 0
		if (bfd_is_com_section(ret->symtab[n]->section))
			val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_value;
		else
			val = ((elf_symbol_type *) symbol)->internal_elf_sym.st_size;
		bfd_fprintf_vma(abfd, file, val);
#endif

	}
#endif
        debug_print(DEBUG_INPUT_BFD, 1, "Setup ok\n");

	return ret;
}

void bf_test_close_file(struct rev_eng *r)
{
	if (!r) return;
	if ( r->section )
		free(r->section);
	if ( r->symtab )
		free(r->symtab);
	if ( r->dynsymtab )
		free(r->dynsymtab);
	if ( r->dynreloc )
		free(r->dynreloc);
	bfd_close(r->bfd);
	free(r);
}

