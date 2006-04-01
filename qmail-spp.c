/*
 * Copyright (C) 2004-2005 Pawel Foremski <pjf@asn.pl>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *** Note
 *
 * This is the core of qmail-spp patch for qmail
 *
 * Why I made it a separate file? Because I wanted qmail-spp to apply more
 * cleanly on heavily patched qmail sources and to make it bit simpler to
 * maintain, so don't treat it as a library.
 *
 * "..." comments marks places where code for other SMTP commands should be
 * added, if needed.
 *
 */

#include "readwrite.h"
#include "stralloc.h"
#include "substdio.h"
#include "control.h"
#include "str.h"
#include "byte.h"
#include "env.h"
#include "exit.h"
#include "wait.h"
#include "fork.h"
#include "fd.h"
#include "fmt.h"
#include "getln.h"

/* stuff needed from qmail-smtpd */
extern void flush();
extern void out();
extern void die_nomem();
extern stralloc addr;
/* *** */

stralloc sppheaders = {0};
static int spprun = 0;
static int sppfok = 0;
static int sppret;
static stralloc sppf = {0};
static stralloc plugins_dummy = {0}, plugins_connect = {0}, plugins_helo = {0}, plugins_mail = {0},
                plugins_rcpt = {0}, plugins_data = {0}; /* ... */
static stralloc error_mail = {0}, error_rcpt = {0}, error_data = {0}; /* ... */
static stralloc sppmsg = {0};
static char rcptcountstr[FMT_ULONG];
static unsigned long rcptcount;
static unsigned long rcptcountall;
static substdio ssdown;
static char downbuf[128];

static void err_spp(s1, s2) char *s1, *s2; { out("451 qmail-spp failure: "); out(s1); out(": "); out(s2); out(" (#4.3.0)\r\n"); }

int spp_init()
{
  int i, len = 0;
  stralloc *plugins_to;
  char *x, *conffile = "control/smtpplugins";

  if (!env_get("NOSPP")) {
    spprun = 1;
    plugins_to = &plugins_dummy;
    x = env_get("SPPCONFFILE");
    if (x && *x) conffile = x;
    sppfok = control_readfile(&sppf, conffile, 0);
    if (sppfok != 1) return -1;
    for (i = 0; i < sppf.len; i += len) {
      len = str_len(sppf.s + i) + 1;
      if (sppf.s[i] == '[')
        switch (sppf.s[i + 1]) {
          case 'c': plugins_to = &plugins_connect; break;
          case 'h': plugins_to = &plugins_helo; break;
          case 'm': plugins_to = &plugins_mail; break;
          case 'r': plugins_to = &plugins_rcpt; break;
          case 'd': plugins_to = &plugins_data; break;
          /* ... */
          default: plugins_to = &plugins_dummy;
        }
      else
        if (!stralloc_catb(plugins_to, sppf.s + i, len)) die_nomem();
    }
  }

  return 0;
}

void sppout() { if (sppmsg.len) out(sppmsg.s); out("\r\n"); }

int spp(plugins, addrenv) stralloc *plugins; char *addrenv;
{
  static int pipes[2];
  static int i, pid, wstat, match, last;
  static stralloc data = {0};
  static char *(args[4]);
  static stralloc *errors_to;

  if (!spprun) return 1;
  if (addrenv) if (!env_put2(addrenv, addr.s)) die_nomem();
  last = 0;

  for (i = 0; i < plugins->len; i += str_len(plugins->s + i) + 1) {
    if (plugins->s[i] == ':')
      { args[0] = "/bin/sh"; args[1] = "-c"; args[2] = plugins->s + i + 1; args[3] = 0; }
    else
      { args[0] = plugins->s + i; args[1] = 0; }

    if (pipe(pipes) == -1)
      { err_spp(plugins->s + i, "can't pipe()"); return 0; }

    switch (pid = vfork()) {
      case -1:
        err_spp(plugins->s + i, "vfork() failed");
        return 0;
      case 0:
        close(0); close(pipes[0]); fd_move(1, pipes[1]);
        execv(*args, args);
        _exit(120);
    }

    close(pipes[1]);
    substdio_fdbuf(&ssdown, read, pipes[0], downbuf, sizeof(downbuf));
    do {
      if (getln(&ssdown, &data, &match, '\n') == -1) die_nomem();
      if (data.len > 1) {
        data.s[data.len - 1] = 0;
        switch (data.s[0]) {
          case 'H':
            if (!stralloc_catb(&sppheaders, data.s + 1, data.len - 2)) die_nomem();
            if (!stralloc_append(&sppheaders, "\n")) die_nomem();
            break;
          case 'C':
            if (addrenv) {
              if (!stralloc_copyb(&addr, data.s + 1, data.len - 1)) die_nomem();
              if (!env_put2(addrenv, addr.s)) die_nomem();
            }
            break;
          case 'S': if (!env_put(data.s + 1)) die_nomem(); break;
          case 'U': if (!env_unset(data.s + 1)) die_nomem(); break;
          case 'A': spprun = 0;
          case 'O':
          case 'N':
          case 'D': last = 1; match = 0; break;
          case 'E':
          case 'R': last = 1; match = 0;
          case 'P': out(data.s + 1); out("\r\n"); break;
          case 'L':
            switch (data.s[1]) {
              case 'M': errors_to = &error_mail; break;
              case 'R': errors_to = &error_rcpt; break;
              case 'D': errors_to = &error_data; break;
              /* ... */
              default: errors_to = 0;
            }
            if (errors_to) {
              if (!stralloc_catb(errors_to, data.s + 2, data.len - 3)) die_nomem();
              if (!stralloc_catb(errors_to, "\r\n", 2)) die_nomem();
            }
            break;
        }
      }
    } while (match);

    close(pipes[0]);
    if (wait_pid(&wstat,pid) == -1) { err_spp(plugins->s + i, "wait_pid() failed"); return 0; }
    if (wait_crashed(wstat)) { err_spp(plugins->s + i, "child crashed"); return 0; }
    if (wait_exitcode(wstat) == 120) { err_spp(plugins->s + i, "can't execute"); return 0; }

    if (last)
      switch (*data.s) {
        case 'E': return 0;
        case 'A':
        case 'N': return 1;
        case 'O': return 2;
        case 'R':
        case 'D': flush(); _exit(0);
      }
  }

  return 1;
}

int spp_errors(errors) stralloc *errors;
{
  if (!errors->len) return 1;
  if (!stralloc_0(errors)) die_nomem();
  out(errors->s);
  return 0;
}

int spp_connect() { return spp(&plugins_connect, 0); }

int spp_helo(arg) char *arg;
{
  if (!env_put2("SMTPHELOHOST", arg)) die_nomem();
  return spp(&plugins_helo, 0);
}

void spp_rset()
{ 
  if (!stralloc_copys(&sppheaders, "")) die_nomem();
  if (!stralloc_copys(&error_mail, "")) die_nomem();
  if (!stralloc_copys(&error_rcpt, "")) die_nomem();
  if (!stralloc_copys(&error_data, "")) die_nomem();
  /* ... */
  rcptcount = rcptcountall = 0;
}

int spp_mail()
{
  if (!spp_errors(&error_mail)) return 0;
  rcptcount = rcptcountall = 0;
  return spp(&plugins_mail, "SMTPMAILFROM");
}

int spp_rcpt(allowed) int allowed;
{
  if (!spp_errors(&error_rcpt)) return 0;
  rcptcountstr[fmt_ulong(rcptcountstr, rcptcount)] = 0;
  if (!env_put2("SMTPRCPTCOUNT", rcptcountstr)) die_nomem();
  rcptcountstr[fmt_ulong(rcptcountstr, ++rcptcountall)] = 0;
  if (!env_put2("SMTPRCPTCOUNTALL", rcptcountstr)) die_nomem();
  if (!env_put2("SMTPRCPTHOSTSOK", allowed ? "1" : "0")) die_nomem();
  sppret = spp(&plugins_rcpt, "SMTPRCPTTO");
  return sppret;
}

void spp_rcpt_accepted() { rcptcount++; }

int spp_data()
{
  if (!spp_errors(&error_data)) return 0;
  return spp(&plugins_data, 0);
}

/* ... */
