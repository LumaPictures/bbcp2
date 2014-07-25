/******************************************************************************/
/*                                                                            */
/*                            b b c p _ S e t . C                             */
/*                                                                            */
/*(c) 2010-14 by the Board of Trustees of the Leland Stanford, Jr., University*//*      All Rights Reserved. See bbcp_Version.C for complete License Terms    *//*                            All Rights Reserved                             */
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

#include "bbcp_C32.h"
#include "bbcp_Set.h"

/******************************************************************************/
/*                         L o c a l   O b j e c t s                          */
/******************************************************************************/

namespace
{
bbcp_C32 kHash;
};
  
/******************************************************************************/
/*                           C o n s t r u c t o r                            */
/******************************************************************************/
  
bbcp_Set::bbcp_Set(int slots)
{
   int n = slots * sizeof(SetItem *);

// Allocate the table
//
   Slots = slots;
   SetTab = (SetItem **)malloc(n);
   memset(SetTab, 0, n);
}

/******************************************************************************/
/*                            D e s t r u c t o r                             */
/******************************************************************************/
  
bbcp_Set::~bbcp_Set()
{
   SetItem *sP, *dP;

   for (int i = 0; i < Slots; i++)
       {if ((sP = SetTab[i]))
           do {dP = sP; sP = sP->next; delete dP;} while(sP);
       }
   free(SetTab);
}

/******************************************************************************/
/*                                   A d d                                    */
/******************************************************************************/
  
bool bbcp_Set::Add(const char *key)
{
   SetItem *sP;
   unsigned int hVal, kEnt;

// Get the hash for the key
//
   hVal = *(unsigned int *)kHash.Calc(key, strlen(key));
   kEnt = hVal % Slots;

// Find the entry
//
   if ((sP = SetTab[kEnt]))
      do {if (!strcmp(key, sP->key)) return false;} while((sP = sP->next));

// Add the item
//
   sP = new SetItem(key, SetTab[kEnt]);
   SetTab[kEnt] = sP;
   return true;
}
