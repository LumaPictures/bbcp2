/******************************************************************************/
/*                                                                            */
/*                             b b c p _ I O . C                              */
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "bbcp_Headers.h"
#include "bbcp_IO.h"
#include "bbcp_NetLogger.h"

/******************************************************************************/
/*                        G l o b a l   O b j e c t s                         */
/******************************************************************************/

extern bbcp_NetLogger bbcp_NetLog;

/******************************************************************************/
/*                     L o c a l   D e f i n i t i o n s                      */
/******************************************************************************/

#define LOGIT(a, l) \
        if (a) bbcp_NetLog.Emit(a, (char *)"", \
               "BBCP.FD=%d BBCP.SK=%lld BBCP.SZ=%d",iofd,xfrseek,l)

/******************************************************************************/
/*                                 C l o s e                                  */
/******************************************************************************/

int bbcp_IO::Close()
{
    if (iofd > 0)
    {
        int oldfd = iofd;
        iofd = -1;
        return (close(oldfd) ? -errno : 0);
    }
    return 0;
}

/******************************************************************************/
/*                                   L o g                                    */
/******************************************************************************/

void bbcp_IO::Log(const char* rk, const char* wk)
{
    char buff[80];

    if (RKeyA)
        free(RKeyA);
    if (RKeyZ)
        free(RKeyZ);

    if (rk)
    {
        if (strlen(rk) > 16)
        {
            sprintf(buff, "START_%16s_READ", rk);
            RKeyA = strdup(buff);
            sprintf(buff, "END_%16s_READ", rk);
            RKeyZ = strdup(buff);
        }
        else
        {
            sprintf(buff, "START_%s_READ", rk);
            RKeyA = strdup(buff);
            sprintf(buff, "END_%s_READ", rk);
            RKeyZ = strdup(buff);
        }
    }

    if (WKeyA)
        free(WKeyA);
    if (WKeyZ)
        free(WKeyZ);

    if (wk)
    {
        if (strlen(wk) > 16)
        {
            sprintf(buff, "START_%16s_WRITE", wk);
            WKeyA = strdup(buff);
            sprintf(buff, "END_%16s_WRITE", wk);
            WKeyZ = strdup(buff);
        }
        else
        {
            sprintf(buff, "START_%s_WRITE", wk);
            WKeyA = strdup(buff);
            sprintf(buff, "END_%s_WRITE", wk);
            WKeyZ = strdup(buff);
        }
    }
}

/******************************************************************************/
/*                                  R e a d                                   */
/******************************************************************************/

ssize_t bbcp_IO::Read(char* buff, size_t rdsz)
{
    ssize_t nbytes = 1, rlen = 0;
    int rc;

// Read data into the buffer
//
    xfrtime.Start();
    while (1)
    {
        LOGIT(RKeyA, rdsz);
        if ((nbytes = read(iofd, (void*)buff, rdsz)) < 0)
        {
            rc = errno;
            LOGIT(RKeyZ, 0);
            if (rc == EINTR)
                continue;
            break;
        }
        LOGIT(RKeyZ, nbytes);
        if (!nbytes)
            break;
        rdsz -= nbytes;
        rlen += nbytes;
        buff += nbytes;
    }
    xfrtime.Stop();

// All done
//
    if (rlen > 0)
    {
        xfrbytes += rlen;
        xfrseek += rlen;
    }
    if (nbytes < 0)
        return -rc;
    return rlen;
}

ssize_t bbcp_IO::Read(const struct iovec* iovp, int iovn)
{
    ssize_t nbytes;
    int rc;

// If logging, add up the bytes to be read
//
    if (RKeyA)
    {
        int i;
        nbytes = iovp[0].iov_len;
        for (i = 1; i < iovn; i++)
            nbytes += iovp[i].iov_len;
    }

// Read data into the buffer. Note that logging only occurs once even though
// we may execute the read multiple times simply because a readv() is an all
// or nothing operation and EINTR's rarely if ever happen on a readv().
//
    xfrtime.Start();
    LOGIT(RKeyA, nbytes);
    while ((nbytes = readv(iofd, (const struct iovec*)iovp, iovn)) < 0 &&
           errno == EINTR)
    {
    }
    if (nbytes < 0)
        rc = errno;
    LOGIT(RKeyZ, (nbytes < 0 ? 0 : nbytes));
    xfrtime.Stop();

// All done
//
    if (nbytes >= 0)
    {
        xfrbytes += nbytes;
        xfrseek += nbytes;
    }
    else
        return -rc;
    return nbytes;
}

/******************************************************************************/
/*                                  S e e k                                   */
/******************************************************************************/

int bbcp_IO::Seek(long long offv)
{
    if (lseek(iofd, (off_t)offv, SEEK_SET) < 0)
        return -errno;
    xfrseek = offv;
    return 0;
}

/******************************************************************************/
/*                                 W r i t e                                  */
/******************************************************************************/

ssize_t bbcp_IO::Write(char* buff, size_t wrsz)
{
    ssize_t nbytes = 0, wlen = 0;
    int rc;

// Write data from the buffer
//
    xfrtime.Start();
    while (wrsz > 0)
    {
        LOGIT(WKeyA, wrsz);
        if ((nbytes = write(iofd, (void*)buff, wrsz)) < 0)
        {
            rc = errno;
            LOGIT(WKeyZ, 0);
            if (rc == EINTR)
                continue;
            break;
        }
        LOGIT(WKeyZ, nbytes);
        wrsz -= nbytes;
        wlen += nbytes;
        buff += nbytes;
    }
    xfrtime.Stop();

// All done
//
    if (wlen > 0)
    {
        xfrbytes += wlen;
        xfrseek += wlen;
    }
    if (nbytes < 0)
        return -rc;
    return wlen;
}

ssize_t bbcp_IO::Write(char* buff, size_t wrsz, off_t offs)
{
    ssize_t nbytes = 0, wlen = 0;
    int rc;

// Write data from the buffer
//
    xfrseek = offs;
    xfrtime.Start();
    while (wrsz > 0)
    {
        LOGIT(WKeyA, wrsz);
        if ((nbytes = pwrite(iofd, (void*)buff, wrsz, offs)) < 0)
        {
            rc = errno;
            LOGIT(WKeyZ, 0);
            if (rc == EINTR)
                continue;
            break;
        }
        LOGIT(WKeyZ, nbytes);
        wrsz -= nbytes;
        wlen += nbytes;
        buff += nbytes;
        offs += nbytes;
    }
    xfrtime.Stop();

// All done
//
    if (wlen > 0)
    {
        xfrbytes += wlen;
        xfrseek += wlen;
    }
    if (nbytes < 0)
        return -rc;
    return wlen;
}

ssize_t bbcp_IO::Write(const struct iovec* iovp, int iovn)
{
    ssize_t nbytes;
    int rc;

// If logging, add up the bytes to be written
//
    if (WKeyA)
    {
        int i;
        nbytes = iovp[0].iov_len;
        for (i = 1; i < iovn; i++)
            nbytes += iovp[i].iov_len;
    }

// Write data from the buffer. Note that logging only occurs once even though
// we may execute the write multiple times simply because a writev() is an all
// or nothing operation and EINTR's rarely if ever happen on a writev().
//
    xfrtime.Start();
    LOGIT(WKeyA, nbytes);
    while ((nbytes = writev(iofd, (const struct iovec*)iovp, iovn)) < 0 &&
           errno == EINTR)
    {
    }
    if (nbytes < 0)
        rc = errno;
    LOGIT(WKeyZ, (nbytes < 0 ? 0 : nbytes));
    xfrtime.Stop();

// All done
//
    if (nbytes >= 0)
    {
        xfrbytes += nbytes;
        xfrseek += nbytes;
    }
    else
        return -rc;
    return nbytes;
}
