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
 * File:   udpClientDeviceInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on March 01, 2017
 */

#include <sys/stat.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <tcl.h>
#include "udpClientInt.h"
#include <netdissect-stdinc.h>
#define HAVE_PCAP_CREATE

static int pflag = 0;
static pcap_t *pd;
static int supports_monitor_mode;
static int packetsCaptured = 0;

// =================================== gmt2local =========================== 

/*
 * Returns the difference between gmt and local time in seconds.
 * Use gmtime() and localtime() to keep things simple.
 */
int32_t gmt2local(time_t t) {
  register int dt, dir;
  register struct tm *gmt, *loc;
  struct tm sgmt;

  if (t == 0)
          t = time(NULL);
  gmt = &sgmt;
  *gmt = *gmtime(&t);
  loc = localtime(&t);
  dt = (loc->tm_hour - gmt->tm_hour) * 60 * 60 +
      (loc->tm_min - gmt->tm_min) * 60;

  /*
   * If the year or julian day is different, we span 00:00 GMT
   * and must add or subtract a day. Check the year first to
   * avoid problems when the julian day wraps.
   */
  dir = loc->tm_year - gmt->tm_year;
  if (dir == 0)
          dir = loc->tm_yday - gmt->tm_yday;
  dt += dir * 24 * 60 * 60;

  return (dt);
}

// =================================== ndo_printf =========================== 

static int ndo_printf(netdissect_options *ndo, const char *fmt, ...) {
  va_list args;
  int ret;

  va_start(args, fmt);
  ret = vfprintf(stdout, fmt, args);
  va_end(args);

  if (ret < 0)
    fprintf(stderr, "Unable to write output: %s", pcap_strerror(errno));
  return (ret);
}

// =================================== etherIfHandler =========================== 

/*
 * This is the top level routine of the printer.  'p' points
 * to the ether header of the packet, 'h->len' is the length
 * of the packet off the wire, and 'h->caplen' is the number
 * of bytes actually captured.
 */
u_int etherIfHandler(netdissect_options *ndo, const struct pcap_pkthdr *h, const u_char *p) {
printf("\netherIfHandler\n");
  return (etherHandler(ndo, p, h->len, h->caplen, NULL, NULL));
}

// =================================== handlePacket =========================== 

static void handlePacket(u_char *data, const struct pcap_pkthdr *h, const u_char *sp) {
  netdissect_options *ndo;
  int hdrlen;

  ndo = (netdissect_options *)data;
printf("handlePacket: ndo_packet_number: %d\n", ndo->ndo_packet_number);
printf("handlePacket: %s!\n", sp+32);
                               
// FIXME that handles the time stamp
//  ts_print(ndo, &h->ts); 
                
  /*
   * Handlers must check that they're not walking off the end of
   * the packet.
   * Rather than pass it all the way down, we set this member
   * of the netdissect_options structure.
   */
  ndo->ndo_snapend = sp + h->caplen;

  hdrlen = (ndo->ndo_if_printer)(ndo, h, sp);
printf("handlePacket: hdrlen: %d\n", hdrlen);
}

// =================================== openInterface =========================== 

static pcap_t *openInterface(const char *device, netdissect_options *ndo, char *ebuf) {
  pcap_t *pc;
  int status;
  char *cp;

printf("openInterface: %s\n", device);
  pc = pcap_create(device, ebuf);
printf("pc: %p ebuf: %s\n", pc, ebuf);
  if (pc == NULL) {
    /*
     * If this failed with "No such device", that means
     * the interface doesn't exist; return NULL, so that
     * the caller can see whether the device name is
     * actually an interface index.
     */
    if (strstr(ebuf, "No such device") != NULL)
      return (NULL);
    error("%s", ebuf);
  }
  /*
   * Is this an interface that supports monitor mode?
   */
  if (pcap_can_set_rfmon(pc) == 1)
    supports_monitor_mode = 1;
  else
    supports_monitor_mode = 0;
printf("suports_monitor_mode: %d\n", supports_monitor_mode);
  status = pcap_set_snaplen(pc, ndo->ndo_snaplen);
  if (status != 0)
    error("%s: Can't set snapshot length: %s",
        device, pcap_statustostr(status));
  status = pcap_set_promisc(pc, !pflag);
  if (status != 0)
    error("%s: Can't set promiscuous mode: %s",
        device, pcap_statustostr(status));
  status = pcap_activate(pc);
  if (status < 0) {
    /*
     * pcap_activate() failed.
     */
    cp = pcap_geterr(pc);
    if (status == PCAP_ERROR)
      error("%s", cp);
    else if (status == PCAP_ERROR_NO_SUCH_DEVICE) {
      /*
       * Return an error for our caller to handle.
       */
      snprintf(ebuf, PCAP_ERRBUF_SIZE, "%s: %s\n(%s)",
          device, pcap_statustostr(status), cp);
      pcap_close(pc);
      return (NULL);
    } else if (status == PCAP_ERROR_PERM_DENIED && *cp != '\0')
      error("%s: %s\n(%s)", device,
          pcap_statustostr(status), cp);
    else
      error("%s: %s", device,
          pcap_statustostr(status));
  } else if (status > 0) {
    /*
     * pcap_activate() succeeded, but it's warning us
     * of a problem it had.
     */
    cp = pcap_geterr(pc);
    if (status == PCAP_WARNING)
      printf("Warning: %s", cp);
    else if (status == PCAP_WARNING_PROMISC_NOTSUP && *cp != '\0')
      printf("Warning: %s: %s\n(%s)", device, pcap_statustostr(status), cp);
    else
      printf("Warning: %s: %s", device, pcap_statustostr(status));
  }
  return (pc);
}

// =================================== udpClientOpenDevice =========================== 

int udpClientOpenDevice (udpClientPtr_t self, char *deviceName) {
  char ebuf[PCAP_ERRBUF_SIZE];
  netdissect_options Ndo;
  netdissect_options *ndo = &Ndo;
  bpf_u_int32 localnet = 0;
  bpf_u_int32 netmask = 0;
  u_char *pcap_userdata;
  struct bpf_program fcode;
  pcap_handler callback;
  char *cmdbuf;
  char *ret = NULL;
  char *programName = "udpClient";
  int timezone_offset = 0;
  int dataLinkType;
  int i;
  int cnt;
  int status;
  int result1;

  /*
   * Initialize the netdissect code.
   */
  if (nd_init(ebuf, sizeof ebuf) == -1) {
fprintf(stderr, "ERROR in nd_init: %s\n", ebuf);
    return UDP_CLIENT_ERR_ND_INIT;
  }

  memset(ndo, 0, sizeof(*ndo));
//  ndo_set_function_pointers(ndo);
  ndo->ndo_printf=ndo_printf;
  ndo->ndo_snaplen = DEFAULT_SNAPLEN;
printf("udpClientOpenDevice\n");
fflush(stdout);
  pd = openInterface(deviceName, ndo, ebuf);
  if (pd == NULL) {

    return TCL_ERROR;
  }
  if (setgid(getgid()) != 0 || setuid(getuid()) != 0) {
    fprintf(stderr, "Warning: setgid/setuid failed !\n");
  }
  result1 = setuid(0);
printf("set root uid: %d\n", result1);
  i = pcap_snapshot(pd);
printf("i: %d snaplen: %d\n", i, ndo->ndo_snaplen);
  if (ndo->ndo_snaplen < i) {
    printf("Warning: snaplen raised from %d to %d", ndo->ndo_snaplen, i);
    ndo->ndo_snaplen = i;
  }
  // FIXME need that as argv part!!
  cmdbuf = "udp port 1900";
  if (pcap_compile(pd, &fcode, cmdbuf, /* Oflag */ 0, netmask) < 0)
    error("%s", pcap_geterr(pd));
printf("call initPrint\n");
  timezone_offset = gmt2local(0);
//  init_print(ndo, localnet, netmask, timezone_offset);
  if (pcap_setfilter(pd, &fcode) < 0)
    error("%s", pcap_geterr(pd));
  dataLinkType = pcap_datalink(pd);
printf("dataLinkType: %d\n", dataLinkType);
  ndo->ndo_if_printer = etherIfHandler;
  callback = handlePacket;
  pcap_userdata = (u_char *)ndo;

printf("before pcap_loop\n");
  do {
    status = pcap_loop(pd, cnt, handlePacket, pcap_userdata);
    /*
     * We're printing packets.  Flush the printed output,
     * so it doesn't get intermingled with error output.
     */
printf("loop: status: %d\n", status);
    if (status == -2) {
      /*
       * We got interrupted, so perhaps we didn't
       * manage to finish a line we were printing.
       * Print an extra newline, just in case.
       */
      putchar('\n');
    }
    (void)fflush(stdout);
    if (status == -2) {
      /*
       * We got interrupted. If we are reading multiple
       * files (via -V) set these so that we stop.
       */
      ret = NULL;
    }
    if (status == -1) {
      /*
       * Error.  Report it.
       */
      (void)fprintf(stderr, "%s: pcap_loop: %s\n", programName, pcap_geterr(pd));
    }
    /*
     * We're doing a live capture.  Report the capture
     * statistics.
     */
//  info(1);
    pcap_close(pd);
  } while (ret != NULL);

//  free(cmdbuf);
  pcap_freecode(&fcode);
//  exit_tcpdump(status == -1 ? 1 : 0); 
  return TCL_OK;
}

// =================================== udpClientCloseDevice =========================== 

int udpClientCloseDevice (udpClientPtr_t self) {
printf("udpClientCloseDevice\n");
  return TCL_OK;
}

// =================================== udpClientDeviceInfoInit =========================== 

int udpClientDeviceInfoInit (udpClientPtr_t self) {
  return UDP_CLIENT_ERR_OK;
}
