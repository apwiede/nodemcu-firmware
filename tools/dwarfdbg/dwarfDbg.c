/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   dwarfdbg.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#include "dwarfDbgInt.h"

/*
 * = = == === ===== ======== ============= =====================
 */

// =================================== dwarfDbgNew =========================== 

dwarfDbgPtr_t dwarfDbgNew () {
  int result;

  _dwarfDbg_t *dbg2 = (_dwarfDbg_t *) ckalloc(sizeof(_dwarfDbg_t));
  _dwarfDbg_t *dbg = ALLOC (_dwarfDbg_t);
  // dwarfdbgEsb module
  dbg->dwarfdbgEsb = ALLOC(dwarfdbgEsb_t);
  result = dwarfdbgEsbInit(dbg);
  // dwarfdbgUtil module
  dbg->dwarfdbgUtil = ALLOC(dwarfdbgUtil_t);
//  result = dwarfdbgEsbInit(dbg);
  // dwarfdbgOpenElf module
  dbg->dwarfDbgOpenElf = ALLOC(dwarfdbgOpenElf_t);
  result = dwarfdbgOpenElfInit(dbg);
// add all other init parts for modules here !!
  return dbg;
}

// =================================== dwarfDbgDel =========================== 

void
dwarfDbgDel (dwarfDbgPtr_t dbg) {
  ckfree ((char*) dbg);
}

// =================================== dwarfdbg_clientdata_set =========================== 

void
dwarfdbg_clientdata_set (dwarfDbgPtr_t dbg, void* clientdata) {
}

// =================================== dwarfdbg_clientdata_get =========================== 

void* dwarfdbg_clientdata_get (dwarfDbgPtr_t dbg) {
return NULL;
}

// =================================== dwarfDbg_fcn1 =========================== 

void dwarfDbg_fcn1 (dwarfDbgPtr_t dbg) {
printf("dwarfdbg_fcn1\n");
fflush(stdout);

}

// =================================== dwarfDbgGetErrorStr =========================== 

char * dwarfDbgGetErrorStr (dwarfDbgPtr_t dbg) {
printf("dwarfdbg_errorStr\n");
fflush(stdout);
  return dbg->errorStr;
}

/*
 * = = == === ===== ======== ============= =====================
 */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
