/*
 * Copyright (c) 2003, 2004, 2005
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Steven K. Reinhardt
 *          Ali G. Saidi
 */

#include <string>

#include "gelf.h"

#include "base/loader/elf_object.hh"
#include "base/loader/symtab.hh"
#include "base/misc.hh"
#include "base/trace.hh"	// for DPRINTF
#include "sim/byteswap.hh"

using namespace std;

ObjectFile *
ElfObject::tryFile(const string &fname, int fd, size_t len, uint8_t *data)
{
    Elf *elf;
    GElf_Ehdr ehdr;
    Arch arch = UnknownArch;
    OpSys opSys = UnknownOpSys;

    // check that header matches library version
    if (elf_version(EV_CURRENT) == EV_NONE)
        panic("wrong elf version number!");

    // get a pointer to elf structure
    elf = elf_memory((char*)data,len);
    // will only fail if fd is invalid
    assert(elf != NULL);

    // Check that we actually have a elf file
    if (gelf_getehdr(elf, &ehdr) ==0) {
        DPRINTFR(Loader, "Not ELF\n");
        elf_end(elf);
        return NULL;
    } else {
        //Detect the architecture
        //Since we don't know how to check for alpha right now, we'll
        //just assume if it wasn't something else and it's 64 bit, that's
        //what it must be.
        if (ehdr.e_machine == EM_SPARC64 ||
                (ehdr.e_machine == EM_SPARC &&
                 ehdr.e_ident[EI_CLASS] == ELFCLASS64)||
                ehdr.e_machine == EM_SPARCV9) {
            arch = ObjectFile::SPARC64;
        } else if (ehdr.e_machine == EM_SPARC32PLUS ||
                        (ehdr.e_machine == EM_SPARC &&
                         ehdr.e_ident[EI_CLASS] == ELFCLASS32)) {
            arch = ObjectFile::SPARC32;
        } else if (ehdr.e_machine == EM_MIPS
                && ehdr.e_ident[EI_CLASS] == ELFCLASS32) {
            arch = ObjectFile::Mips;
        } else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
            arch = ObjectFile::Alpha;
        } else if (ehdr.e_machine == EM_ARM) {
            arch = ObjectFile::Arm;
        } else {
            warn("Unknown architecture: %d\n", ehdr.e_machine);
            arch = ObjectFile::UnknownArch;
        }

        //Detect the operating system
        switch (ehdr.e_ident[EI_OSABI])
        {

          case ELFOSABI_LINUX:
          case ELFOSABI_ARM:
            opSys = ObjectFile::Linux;
            break;
          case ELFOSABI_SOLARIS:
            opSys = ObjectFile::Solaris;
            break;
          case ELFOSABI_TRU64:
            opSys = ObjectFile::Tru64;
            break;
          default:
            opSys = ObjectFile::UnknownOpSys;
        }

        //take a look at the .note.ABI section
        //It can let us know what's what.
        if (opSys == ObjectFile::UnknownOpSys) {
            Elf_Scn *section;
            GElf_Shdr shdr;
            Elf_Data *data;
            uint32_t osAbi;;
            int secIdx = 1;

            // Get the first section
            section = elf_getscn(elf, secIdx);

            // While there are no more sections
            while (section != NULL && opSys == ObjectFile::UnknownOpSys) {
                gelf_getshdr(section, &shdr);
                if (shdr.sh_type == SHT_NOTE && !strcmp(".note.ABI-tag",
                            elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name))) {
                    // we have found a ABI note section
                    // Check the 5th 32bit word for OS  0 == linux, 1 == hurd,
                    // 2 == solaris, 3 == freebsd
                    data = elf_rawdata(section, NULL);
                    assert(data->d_buf);
                    if(ehdr.e_ident[EI_DATA] == ELFDATA2LSB)
                        osAbi = htole(((uint32_t*)data->d_buf)[4]);
                    else
                        osAbi = htobe(((uint32_t*)data->d_buf)[4]);

                    switch(osAbi) {
                      case 0:
                        opSys = ObjectFile::Linux;
                        break;
                      case 2:
                        opSys = ObjectFile::Solaris;
                        break;
                    }
                } // if section found
                if (!strcmp(".SUNW_version", elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name)))
                        opSys = ObjectFile::Solaris;
                if (!strcmp(".stab.index", elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name)))
                        opSys = ObjectFile::Solaris;

            section = elf_getscn(elf, ++secIdx);
            } // while sections
        }

        ElfObject * result = new ElfObject(fname, fd, len, data, arch, opSys);

        //The number of headers in the file
        result->_programHeaderCount = ehdr.e_phnum;
        //Record the size of each entry
        result->_programHeaderSize = ehdr.e_phentsize;
        if(result->_programHeaderCount) //If there is a program header table
        {
            //Figure out the virtual address of the header table in the
            //final memory image. We use the program headers themselves
            //to translate from a file offset to the address in the image.
            GElf_Phdr phdr;
            uint64_t e_phoff = ehdr.e_phoff;
            result->_programHeaderTable = 0;
            for(int hdrnum = 0; hdrnum < result->_programHeaderCount; hdrnum++)
            {
                gelf_getphdr(elf, hdrnum, &phdr);
                //Check if we've found the segment with the headers in it
                if(phdr.p_offset <= e_phoff &&
                        phdr.p_offset + phdr.p_filesz > e_phoff)
                {
                    result->_programHeaderTable = phdr.p_paddr + e_phoff;
                    break;
                }
            }
        }
        else
            result->_programHeaderTable = 0;


        elf_end(elf);
        return result;
    }
}


ElfObject::ElfObject(const string &_filename, int _fd,
                     size_t _len, uint8_t *_data,
                     Arch _arch, OpSys _opSys)
    : ObjectFile(_filename, _fd, _len, _data, _arch, _opSys)

{
    Elf *elf;
    GElf_Ehdr ehdr;

    // check that header matches library version
    if (elf_version(EV_CURRENT) == EV_NONE)
        panic("wrong elf version number!");

    // get a pointer to elf structure
    elf = elf_memory((char*)fileData,len);
    // will only fail if fd is invalid
    assert(elf != NULL);

    // Check that we actually have a elf file
    if (gelf_getehdr(elf, &ehdr) ==0) {
        panic("Not ELF, shouldn't be here");
    }

    entry = ehdr.e_entry;

    // initialize segment sizes to 0 in case they're not present
    text.size = data.size = bss.size = 0;

    int secIdx = 1;
    Elf_Scn *section;
    GElf_Shdr shdr;

    // The first address of some important sections.
    Addr textSecStart = 0;
    Addr dataSecStart = 0;
    Addr bssSecStart = 0;

    // Get the first section
    section = elf_getscn(elf, secIdx);

    // Find the beginning of the most interesting sections.
    while (section != NULL) {
        gelf_getshdr(section, &shdr);
        char * secName = elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name);

        if (!strcmp(".text", secName)) {
            textSecStart = shdr.sh_addr;
        } else if (!strcmp(".data", secName)) {
            dataSecStart = shdr.sh_addr;
        } else if (!strcmp(".bss", secName)) {
            bssSecStart = shdr.sh_addr;
        }

        section = elf_getscn(elf, ++secIdx);
    }

    // Go through all the segments in the program, record them, and scrape
    // out information about the text, data, and bss areas needed by other
    // code.
    for (int i = 0; i < ehdr.e_phnum; ++i) {
        GElf_Phdr phdr;
        if (gelf_getphdr(elf, i, &phdr) == 0) {
            panic("gelf_getphdr failed for segment %d.", i);
        }

        // for now we don't care about non-loadable segments
        if (!(phdr.p_type & PT_LOAD))
            continue;

        // Check to see if this segment contains the bss section.
        if (phdr.p_paddr <= bssSecStart &&
                phdr.p_paddr + phdr.p_memsz > bssSecStart &&
                phdr.p_memsz - phdr.p_filesz > 0) {
            bss.baseAddr = phdr.p_paddr + phdr.p_filesz;
            bss.size = phdr.p_memsz - phdr.p_filesz;
            bss.fileImage = NULL;
        }

        // Check to see if this is the text or data segment
        if (phdr.p_vaddr <= textSecStart &&
                phdr.p_vaddr + phdr.p_filesz > textSecStart) {
            text.baseAddr = phdr.p_paddr;
            text.size = phdr.p_filesz;
            text.fileImage = fileData + phdr.p_offset;
        } else if (phdr.p_vaddr <= dataSecStart &&
                phdr.p_vaddr + phdr.p_filesz > dataSecStart) {
            data.baseAddr = phdr.p_paddr;
            data.size = phdr.p_filesz;
            data.fileImage = fileData + phdr.p_offset;
        } else {
            Segment extra;
            extra.baseAddr = phdr.p_paddr;
            extra.size = phdr.p_filesz;
            extra.fileImage = fileData + phdr.p_offset;
            extraSegments.push_back(extra);
        }
    }

    // should have found at least one loadable segment
    assert(text.size != 0);

    DPRINTFR(Loader, "text: 0x%x %d\ndata: 0x%x %d\nbss: 0x%x %d\n",
             text.baseAddr, text.size, data.baseAddr, data.size,
             bss.baseAddr, bss.size);

    elf_end(elf);

    // We will actually read the sections when we need to load them
}


bool
ElfObject::loadSomeSymbols(SymbolTable *symtab, int binding)
{
    Elf *elf;
    int sec_idx = 1; // there is a 0 but it is nothing, go figure
    Elf_Scn *section;
    GElf_Shdr shdr;
    Elf_Data *data;
    int count, ii;
    bool found = false;
    GElf_Sym sym;

    if (!symtab)
        return false;

    // check that header matches library version
    if (elf_version(EV_CURRENT) == EV_NONE)
        panic("wrong elf version number!");

    // get a pointer to elf structure
    elf = elf_memory((char*)fileData,len);

    assert(elf != NULL);

    // Get the first section
    section = elf_getscn(elf, sec_idx);

    // While there are no more sections
    while (section != NULL) {
        gelf_getshdr(section, &shdr);

        if (shdr.sh_type == SHT_SYMTAB) {
            found = true;
            data = elf_getdata(section, NULL);
            count = shdr.sh_size / shdr.sh_entsize;
            DPRINTF(Loader, "Found Symbol Table, %d symbols present\n", count);

            // loop through all the symbols, only loading global ones
            for (ii = 0; ii < count; ++ii) {
                gelf_getsym(data, ii, &sym);
                if (GELF_ST_BIND(sym.st_info) == binding) {
                   symtab->insert(sym.st_value,
                                  elf_strptr(elf, shdr.sh_link, sym.st_name));
                }
            }
        }
        ++sec_idx;
        section = elf_getscn(elf, sec_idx);
    }

    elf_end(elf);

    return found;
}

bool
ElfObject::loadGlobalSymbols(SymbolTable *symtab, Addr addrMask)
{
    return loadSomeSymbols(symtab, STB_GLOBAL);
}

bool
ElfObject::loadLocalSymbols(SymbolTable *symtab, Addr addrMask)
{
    return loadSomeSymbols(symtab, STB_LOCAL);
}

bool
ElfObject::loadSections(Port *memPort, Addr addrMask)
{
    if (!ObjectFile::loadSections(memPort, addrMask))
        return false;

    vector<Segment>::iterator extraIt;
    for (extraIt = extraSegments.begin();
            extraIt != extraSegments.end(); extraIt++) {
        if (!loadSection(&(*extraIt), memPort, addrMask)) {
            return false;
        }
    }
    return true;
}

void
ElfObject::getSections()
{
    Elf *elf;
    int sec_idx = 1; // there is a 0 but it is nothing, go figure
    Elf_Scn *section;
    GElf_Shdr shdr;

    GElf_Ehdr ehdr;

    assert(!sectionNames.size());

    // check that header matches library version
    if (elf_version(EV_CURRENT) == EV_NONE)
        panic("wrong elf version number!");

    // get a pointer to elf structure
    elf = elf_memory((char*)fileData,len);
    assert(elf != NULL);

    // Check that we actually have a elf file
    if (gelf_getehdr(elf, &ehdr) ==0) {
        panic("Not ELF, shouldn't be here");
    }

    // Get the first section
    section = elf_getscn(elf, sec_idx);

    // While there are no more sections
    while (section != NULL) {
        gelf_getshdr(section, &shdr);
        sectionNames.insert(elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name));
        section = elf_getscn(elf, ++sec_idx);
    } // while sections
}

bool
ElfObject::sectionExists(string sec)
{
    if (!sectionNames.size())
        getSections();
    return sectionNames.find(sec) != sectionNames.end();
}

