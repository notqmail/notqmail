#ifndef SIG_H
#define SIG_H

extern void sig_catch();
extern void sig_block();
extern void sig_unblock();
extern void sig_blocknone();
extern void sig_pause();

extern void sig_dfl();

extern void sig_miscignore();
extern void sig_bugcatch();

extern void sig_pipeignore();
extern void sig_pipedefault();

extern void sig_contblock();
extern void sig_contunblock();
extern void sig_contcatch();
extern void sig_contdefault();

extern void sig_termblock();
extern void sig_termunblock();
extern void sig_termcatch();
extern void sig_termdefault();

extern void sig_alarmblock();
extern void sig_alarmunblock();
extern void sig_alarmcatch();
extern void sig_alarmdefault();

extern void sig_childblock();
extern void sig_childunblock();
extern void sig_childcatch();
extern void sig_childdefault();

extern void sig_hangupblock();
extern void sig_hangupunblock();
extern void sig_hangupcatch();
extern void sig_hangupdefault();

#endif
