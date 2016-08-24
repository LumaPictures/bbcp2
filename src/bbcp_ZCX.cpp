/******************************************************************************/
/*                                                                            */
/*                            b b c p _ Z C X . C                             */
/*                                                                            */
/*(c) 2002-14 by the Board of Trustees of the Leland Stanford, Jr., University*//*      All Rights Reserved. See bbcp_Version.C for complete License Terms    *//*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC02-76-SFO0515 with the Department of Energy              */
/*                                                                            */
/* bbcp is free software: you can redistribute it and/or modify it under      */
/* the terms of the GNU Lesser General Public License as published by the     */
/* Free Software Foundation, either version 3 of the License, or (at your     */
/* option) any later version.                                                 */
/*                                                                            */
/* bbcp is distributed in the hope that it will be useful, but WITHOUT        */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public       */
/* License for more details.                                                  */
/*                                                                            */
/* You should have received a copy of the GNU Lesser General Public License   */
/* along with bbcp in a file called COPYING.LESSER (LGPL license) and file    */
/* COPYING (GPL license).  If not, see <http://www.gnu.org/licenses/>.        */
/*                                                                            */
/* The copyright holder's institutional names and contributor's names may not */
/* be used to endorse or promote products derived from this software without  */
/* specific prior written permission of the institution or contributor.       */
/******************************************************************************/

extern "C"
{
#include <zlib.h>
}

#include "bbcp_BuffPool.h"
#include "bbcp_Emsg.h"
#include "bbcp_Headers.h"
#include "bbcp_NetLogger.h"
#include "bbcp_Platform.h"
#include "bbcp_ZCX.h"

#include <blosc.h>
#include <cstring>
#include <vector>

/******************************************************************************/
/*                        G l o b a l   O b j e c t s                         */
/******************************************************************************/

extern bbcp_NetLogger bbcp_NetLog;

/******************************************************************************/
/*                     L o c a l   D e f i n i t i o n s                      */
/******************************************************************************/

#define LOGSTART \
        if (LogIDbeg) {xfrseek = (Clvl ? inbytes : outbytes); LOGIT(LogIDbeg);}

#define LOGEND \
        if (LogIDend) LOGIT(LogIDend)

#define LOGIT(a) \
        bbcp_NetLog.Emit(a, (char *)"", "BBCP.FD=%d BBCP.SK=%lld",iofd,xfrseek)

/******************************************************************************/
/*                           C o n s t r u c t o r                            */
/******************************************************************************/

bbcp_ZCX::bbcp_ZCX(bbcp_BuffPool* ib, bbcp_BuffPool* rb, bbcp_BuffPool* ob,
                   int clvl, int xfd, int logit)
{
    Ibuff = ib;
    Rbuff = rb;
    Obuff = ob;
    Clvl = clvl;
    iofd = xfd;
    cbytes = 0;
    TID = 0;

    if (!logit)
        LogIDbeg = LogIDend = 0;
    else
    {
        LogIDbeg = (clvl ? (char*)"STARTCMP" : (char*)"STARTEXP");
        LogIDend = (clvl ? (char*)"ENDCMP" : (char*)"ENDEXP");
    }
}

/******************************************************************************/
/*                               P r o c e s s                                */
/******************************************************************************/

int bbcp_ZCX::Process()
{
    std::vector<char> temp_buffer(Obuff->DataSize() * 2);

    auto memdupvec = [](std::vector<char> in, const size_t in_size) -> char* {
        char* ret = reinterpret_cast<char*>(malloc(in_size));
        memcpy(ret, in.data(), in_size);
        return ret;
    };

    long long outbytes = 0;
    long long inbytes = 0;

    cbytes = 0;

    bbcp_Buffer* ibp = Ibuff->getFullBuff();

    size_t number_of_fragments = 0;
    while (ibp && ibp->blen)
    {
        ++number_of_fragments;
        bbcp_Buffer* obp = Obuff->getEmptyBuff();

        if (obp == nullptr)
            return ENOBUFS;

        const size_t in_size = ibp->blen;

        if (Clvl) // compressing
        {
            const int compressed_size = blosc_compress(Clvl, 0, sizeof(char), in_size, ibp->data, temp_buffer.data(), temp_buffer.size());
            if (compressed_size < 0)
                return Z_STREAM_ERROR;

            obp->blen = compressed_size;
            obp->data = memdupvec(temp_buffer, compressed_size);
        }
        else
        {
            const int decompressed_size = blosc_decompress(ibp->data, temp_buffer.data(), temp_buffer.size());
            if (decompressed_size < 0)
                return Z_STREAM_ERROR;

            obp->blen = decompressed_size;
            obp->data = memdupvec(temp_buffer, decompressed_size);
        }

        obp->boff = outbytes;

        inbytes += ibp->blen;
        outbytes += obp->blen;

        cbytes = Clvl ? outbytes : inbytes;

        Obuff->putFullBuff(obp);
        Rbuff->putEmptyBuff(ibp);
        ibp = Ibuff->getFullBuff();
    }

    cbytes = Clvl ? outbytes : inbytes;

    Rbuff->putEmptyBuff(ibp);
    
    bbcp_Buffer* obp = Obuff->getEmptyBuff();
    obp->blen = 0;
    obp->boff = outbytes;
    Obuff->putFullBuff(obp);
    return 0;
}

/******************************************************************************/
/*                              Z f a i l u r e                               */
/******************************************************************************/

int bbcp_ZCX::Zfailure(int zerr, const char* oper, char* Zmsg)
{
    char* txt2 = (Clvl ? (char*)"compression" : (char*)"expansion");
    switch (zerr)
    {
        case Z_ERRNO:
            zerr = errno;
            break;
        case Z_STREAM_ERROR:
            zerr = ENOSR;
            break;
        case Z_DATA_ERROR:
            zerr = EOVERFLOW;
            break;
        case Z_MEM_ERROR:
            zerr = ENOMEM;
            break;
        case Z_BUF_ERROR:
            zerr = EBADSLT;
            break;
        case Z_VERSION_ERROR:
            zerr = EPROTO;
            break;
        default:
            zerr = (zerr < 0 ? 10000 - zerr : 20000 + zerr);
    }

    Ibuff->Abort();
    if (Ibuff != Rbuff)
        Rbuff->Abort();
    Obuff->Abort();

    if (Zmsg)
        return bbcp_Fmsg("Zlib", Zmsg, oper, txt2);
    return bbcp_Emsg("Zlib", zerr, oper, txt2, (Zmsg ? Zmsg : ""));
}

void bbcp_ZCX::init_compressor()
{
    blosc_init();
    blosc_set_nthreads(32);
    const char* compressors[] = {"blosclz", "lz4", "lz4hc", "snappy", "zlib"};
    blosc_set_compressor(compressors[4]);
}

void bbcp_ZCX::destroy_compressor()
{
    blosc_destroy();
}
