/*
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "backtrace.h"

#include <QtGlobal>
#include <iostream>
#include <cstdlib>
#include <ostream>
#include <sstream>
#ifndef Q_OS_WIN
#include <execinfo.h>
#include <unistd.h>
#include <cxxabi.h>
#include <dlfcn.h>
// #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <execinfo.h>
// #include <bfd.h>
// #include <dlfcn.h>
// #include <link.h>
#else
#include <io.h>
#include <process.h>
#endif

// class FileLineDesc
// {
// public:
//    FileLineDesc( asymbol** syms, bfd_vma pc ) : mPc( pc ), mFound( false ), mSyms( syms ) {}

//    void findAddressInSection( bfd* abfd, asection* section );

//    bfd_vma      mPc;
//    char*        mFilename;
//    char*        mFunctionname;
//    unsigned int mLine;
//    int          mFound;
//    asymbol**    mSyms;
// };

// void FileLineDesc::findAddressInSection( bfd* abfd, asection* section )
// {
//    if ( mFound )
//       return;

//    if (( bfd_get_section_flags( abfd, section ) & SEC_ALLOC ) == 0 )
//       return;

//    bfd_vma vma = bfd_get_section_vma( abfd, section );
//    if ( mPc < vma )
//       return;

//    bfd_size_type size = bfd_section_size( abfd, section );
//    if ( mPc >= ( vma + size ))
//       return;

//    mFound = bfd_find_nearest_line( abfd, section, mSyms, ( mPc - vma ),
//                                    (const char**)&mFilename, (const char**)&mFunctionname, &mLine );
// }

// static void findAddressInSection( bfd* abfd, asection* section, void* data )
// {
//    FileLineDesc* desc = (FileLineDesc*)data;
//    assert( desc );
//    return desc->findAddressInSection( abfd, section );
// }

// static char** translateAddressesBuf( bfd* abfd, bfd_vma* addr, int numAddr, asymbol** syms )
// {
//    char** ret_buf = NULL;
//    s32    total   = 0;

//    char   b;
//    char*  buf     = &b;
//    s32    len     = 0;

//    for ( u32 state = 0; state < 2; state++ )
//    {
//       if ( state == 1 )
//       {
//          ret_buf = (char**)malloc( total + ( sizeof(char*) * numAddr ));
//          buf = (char*)(ret_buf + numAddr);
//          len = total;
//       }

//       for ( s32 i = 0; i < numAddr; i++ )
//       {
//          FileLineDesc desc( syms, addr[i] );

//          if ( state == 1 )
//             ret_buf[i] = buf;

//          bfd_map_over_sections( abfd, FindAddressInSection, (void*)&desc );

//          if ( !desc.mFound )
//          {
//             total += snprintf( buf, len, "[0x%llx] \?\? \?\?:0", (long long unsigned int) addr[i] ) + 1;

//          } else {

//             const char* name = desc.mFunctionname;
//             if ( name == NULL || *name == '\0' )
//                name = "??";
//             if ( desc.mFilename != NULL )
//             {
//                char* h = strrchr( desc.mFilename, '/' );
//                if ( h != NULL )
//                   desc.mFilename = h + 1;
//             }
//             total += snprintf( buf, len, "%s:%u %s", desc.mFilename ? desc.mFilename : "??", desc.mLine, name ) + 1;
//             // elog << "\"" << buf << "\"\n";
//          }
//       }

//       if ( state == 1 )
//       {
//          buf = buf + total + 1;
//       }
//    }

//    return ret_buf;
// }


// static asymbol** kstSlurpSymtab( bfd* abfd, const char* fileName )
// {
//    if ( !( bfd_get_file_flags( abfd ) & HAS_SYMS ))
//    {
//       printf( "Error bfd file \"%s\" flagged as having no symbols.\n", fileName );
//       return NULL;
//    }

//    asymbol** syms;
//    unsigned int size;

//    long symcount = bfd_read_minisymbols( abfd, false, (void**)&syms, &size );
//    if ( symcount == 0 )
//         symcount = bfd_read_minisymbols( abfd, true,  (void**)&syms, &size );

//    if ( symcount < 0 )
//    {
//       printf( "Error bfd file \"%s\", found no symbols.\n", fileName );
//       return NULL;
//    }

//    return syms;
// }

// static char** processFile( const char* fileName, bfd_vma* addr, int naddr )
// {
//    bfd* abfd = bfd_openr( fileName, NULL );
//    if ( !abfd )
//    {
//       printf( "Error opening bfd file \"%s\"\n", fileName );
//       return NULL;
//    }

//    if ( bfd_check_format( abfd, bfd_archive ) )
//    {
//       printf( "Cannot get addresses from archive \"%s\"\n", fileName );
//       bfd_close( abfd );
//       return NULL;
//    }

//    char** matching;
//    if ( !bfd_check_format_matches( abfd, bfd_object, &matching ))
//    {
//       printf( "Format does not match for archive \"%s\"\n", fileName );
//       bfd_close( abfd );
//       return NULL;
//    }

//    asymbol** syms = kstSlurpSymtab( abfd, fileName );
//    if ( !syms )
//    {
//       printf( "Failed to read symbol table for archive \"%s\"\n", fileName );
//       bfd_close( abfd );
//       return NULL;
//    }

//    char** retBuf = translateAddressesBuf( abfd, addr, naddr, syms );

//    free( syms );

//    bfd_close( abfd );
//    return retBuf;
// }

// class FileMatch
// {
// public:
//    FileMatch( void* addr ) : mAddress( addr ), mFile( NULL ), mBase( NULL ) {}

//    void*       mAddress;
//    const char* mFile;
//    void*       mBase;
// };

// static int findMatchingFile( struct dl_phdr_info* info, size_t size, void* data )
// {
//    FileMatch* match = (FileMatch*)data;

//    for ( u32 i = 0; i < info->dlpi_phnum; i++ )
//    {
//       const ElfW(Phdr)& phdr = info->dlpi_phdr[i];

//       if ( phdr.p_type == PT_LOAD )
//       {
//          ElfW(Addr) vaddr = phdr.p_vaddr + info->dlpi_addr;
//          ElfW(Addr) maddr = ElfW(Addr)(match->mAddress);
//          if (( maddr >= vaddr ) &&
//              ( maddr < vaddr + phdr.p_memsz ))
//          {
//             match->mFile =        info->dlpi_name;
//             match->mBase = (void*)info->dlpi_addr;
//             return 1;
//          }
//       }
//    }
//    return 0;
// }

// char** backtraceSymbols( void* const* addrList, int numAddr )
// {
//    char*** locations = (char***) alloca( sizeof( char** ) * numAddr );

//    // initialize the bfd library
//    bfd_init();

//    int total = 0;
//    u32 idx = numAddr;
//    for ( s32 i = 0; i < numAddr; i++ )
//    {
//       // find which executable, or library the symbol is from
//       FileMatch match( addrList[--idx] );
//       dl_iterate_phdr( findMatchingFile, &match );

//       // adjust the address in the global space of your binary to an
//       // offset in the relevant library
//       bfd_vma addr  = (bfd_vma)( addrList[idx] );
//               addr -= (bfd_vma)( match.mBase );

//       // lookup the symbol
//       if ( match.mFile && strlen( match.mFile ))
//          locations[idx] = processFile( match.mFile,      &addr, 1 );
//       else
//          locations[idx] = processFile( "/proc/self/exe", &addr, 1 );

//       total += strlen( locations[idx][0] ) + 1;
//    }

//    // return all the file and line information for each address
//    char** final = (char**)malloc( total + ( numAddr * sizeof( char* )));
//    char* f_strings = (char*)( final + numAddr );

//    for ( s32 i = 0; i < numAddr; i++ )
//    {
//       strcpy( f_strings, locations[i][0] );
//       free( locations[i] );
//       final[i] = f_strings;
//       f_strings += strlen( f_strings ) + 1;
//    }

//    return final;
// }


void printStacktrace()
{
#ifndef Q_OS_WIN
    int skip = 1;
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    std::ostringstream trace_buf;
    for (int i = skip; i < nFrames; i++) {
        // printf("%s\n", symbols[i]);
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;
            if (info.dli_sname[0] == '_') {
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            }
            snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i],
                    status == 0 ? demangled :
                    info.dli_sname == 0 ? symbols[i] : info.dli_sname,
                    (char *)callstack[i] - (char *)info.dli_saddr);
            free(demangled);
        } else {
            snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
        }
        trace_buf << buf;
    }
    free(symbols);
    if (nFrames == nMaxFrames) {
        trace_buf << "[truncated]\n";
    }
    std::cerr << trace_buf.str();
#endif
}
