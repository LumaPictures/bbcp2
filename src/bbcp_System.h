#ifndef __BBCP_SYSTEM_H__
#define __BBCP_SYSTEM_H__
/******************************************************************************/
/*                                                                            */
/*                         b b c p _ S y s t e m . h                          */
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

#include <sys/types.h>
#include "bbcp_Pthread.h"

class bbcp_System {
public:

// Spawn a new process and return the process id (0 if child)
//
    pid_t Fork();

// Convert a group name to a GID return -1 if failed.
//
    gid_t getGID(const char* group);

// Convert a user name to a UID return -1 if failed.
//
    uid_t getUID(const char* uid);

// Convert a GID to a group name return "nogroup" if failed.
//
    char* getGNM(gid_t gid);

// Convert a UID to a user name and return "nouser" if failed.
//
    char* getUNM(uid_t uid);

// Get the home directory
//
    char* getHomeDir();

// Get the process id of the grandparent process
//
    pid_t getGrandP();

// Get total cpu time spent in seconds
//
    int Usage(int& sys, int& usr);

// Get my username
//
    char* UserName();

// Wait for a process to end
//
    int Waitpid(pid_t thePid);

    int Waitpid(pid_t* pvec, int* ent = 0, int nomsg = 0);

    bbcp_System();

    ~bbcp_System()
    {
    }

    char** EnvP;        // Global environment pointer
    long long FreeRAM;     // Free ememory at startup
    int FreePag;     // Number of free pages at startup
    int TotPag;      // Maximum number of readv/writev elements
    int PageSize;    // Page size

private:

    bbcp_Mutex Glookup;
    bbcp_Mutex Plookup;
};

#endif
