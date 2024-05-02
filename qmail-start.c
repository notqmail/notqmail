#include <sys/stat.h>
#include <unistd.h>
#include "fd.h"
#include "prot.h"
#include "exit.h"
#include "fork.h"
#include "noreturn.h"
#include "uidgid.h"
#include "auto_uids.h"
#include "auto_users.h"

char *(qsargs[]) = { "qmail-send", 0 };
char *(qcargs[]) = { "qmail-clean", 0 };
char *(qlargs[]) = { "qmail-lspawn", "./Mailbox", 0 };
char *(qrargs[]) = { "qmail-rspawn", 0 };
char *(qtargs[]) = { "qmail-todo", 0};

void _noreturn_ die() { _exit(111); }

int pi0[2];
int pi1[2];
int pi2[2];
int pi3[2];
int pi4[2];
int pi5[2];
int pi6[2];
int pi7[2];
int pi8[2];
int pi9[2];
int pi10[2];

uid_t auto_uidl;
uid_t auto_uidq;
uid_t auto_uidr;
uid_t auto_uids;

gid_t auto_gidn;
gid_t auto_gidq;

void close23456() {
  close(2); close(3); close(4); close(5); close(6);
  close(7); close(8);
}

void closepipes() {
  close(pi1[0]); close(pi1[1]); close(pi2[0]); close(pi2[1]);
  close(pi3[0]); close(pi3[1]); close(pi4[0]); close(pi4[1]);
  close(pi5[0]); close(pi5[1]); close(pi6[0]); close(pi6[1]);
  close(pi7[0]); close(pi7[1]); close(pi8[0]); close(pi8[1]);
  close(pi9[0]); close(pi9[1]); close(pi10[0]); close(pi10[1]);
}

int main(int argc, char **argv)
{
  if (chdir("/") == -1) die();
  umask(077);

  auto_uidl = inituid(auto_userl);
  auto_uidq = inituid(auto_userq);
  auto_uidr = inituid(auto_userr);
  auto_uids = inituid(auto_users);

  auto_gidn = initgid(auto_groupn);
  auto_gidq = initgid(auto_groupq);

  if (prot_gid(auto_gidq) == -1) die();

  if (fd_copy(2,0) == -1) die();
  if (fd_copy(3,0) == -1) die();
  if (fd_copy(4,0) == -1) die();
  if (fd_copy(5,0) == -1) die();
  if (fd_copy(6,0) == -1) die();
  if (fd_copy(7,0) == -1) die();
  if (fd_copy(8,0) == -1) die();

  if (argv[1]) {
    qlargs[1] = argv[1];
    ++argv;
  }

  if (argv[1]) {
    if (pipe(pi0) == -1) die();
    switch(fork()) {
      case -1:
	die();
      case 0:
        if (prot_gid(auto_gidn) == -1) die();
        if (prot_uid(auto_uidl) == -1) die();
        close(pi0[1]);
        if (fd_move(0,pi0[0]) == -1) die();
        close23456();
        execvp(argv[1],argv + 1);
	die();
    }
    close(pi0[0]);
    if (fd_move(1,pi0[1]) == -1) die();
  }
 
  if (pipe(pi1) == -1) die();
  if (pipe(pi2) == -1) die();
  if (pipe(pi3) == -1) die();
  if (pipe(pi4) == -1) die();
  if (pipe(pi5) == -1) die();
  if (pipe(pi6) == -1) die();
  if (pipe(pi7) == -1) die();
  if (pipe(pi8) == -1) die();
  if (pipe(pi9) == -1) die();
  if (pipe(pi10) == -1) die();
 
  switch(fork()) {
    case -1: die();
    case 0:
      if (fd_copy(0,pi1[0]) == -1) die();
      if (fd_copy(1,pi2[1]) == -1) die();
      close23456();
      closepipes();
      execvp(*qlargs,qlargs);
      die();
  }
 
  switch(fork()) {
    case -1: die();
    case 0:
      if (prot_gids(auto_userr, auto_gidq) == -1) die();
      if (prot_uid(auto_uidr) == -1) die();
      if (fd_copy(0,pi3[0]) == -1) die();
      if (fd_copy(1,pi4[1]) == -1) die();
      close23456();
      closepipes();
      execvp(*qrargs,qrargs);
      die();
  }
 
  switch(fork()) {
    case -1: die();
    case 0:
      if (prot_uid(auto_uidq) == -1) die();
      if (fd_copy(0,pi5[0]) == -1) die();
      if (fd_copy(1,pi6[1]) == -1) die();
      close23456();
      closepipes();
      execvp(*qcargs,qcargs);
      die();
  }

  switch(fork()) {
    case -1: die();
    case 0:
      if (prot_uid(auto_uids) == -1) die();
      if (fd_copy(0,pi7[0]) == -1) die();
      if (fd_copy(1,pi8[1]) == -1) die();
      close23456();
      if (fd_copy(2,pi9[1]) == -1) die();
      if (fd_copy(3,pi10[0]) == -1) die();
      closepipes();
      execvp(*qtargs,qtargs);
      die();
  }

  switch(fork()) {
    case -1: die();
    case 0:
      if (prot_uid(auto_uidq) == -1) die();
      if (fd_copy(0,pi9[0]) == -1) die();
      if (fd_copy(1,pi10[1]) == -1) die();
      close23456();
      closepipes();
      execvp(*qcargs,qcargs);
      die();
  }
 
  if (prot_uid(auto_uids) == -1) die();
  if (fd_copy(0,1) == -1) die();
  if (fd_copy(1,pi1[1]) == -1) die();
  if (fd_copy(2,pi2[0]) == -1) die();
  if (fd_copy(3,pi3[1]) == -1) die();
  if (fd_copy(4,pi4[0]) == -1) die();
  if (fd_copy(5,pi5[1]) == -1) die();
  if (fd_copy(6,pi6[0]) == -1) die();
  if (fd_copy(7,pi7[1]) == -1) die();
  if (fd_copy(8,pi8[0]) == -1) die();
  closepipes();
  execvp(*qsargs,qsargs);
  die();
}
