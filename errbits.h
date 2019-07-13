extern void esetfd(); /* functions in this module default to FD 2 (stderr) for output, change with esetfd */
extern void eout();
extern void eout2();
extern void eout3();
#define eout4(s1,s2,s3,s4)		        	{ eout3(s1,s2,s3); eout(s4); }
#define eout5(s1,s2,s3,s4,s5)		        { eout3(s1,s2,s3); eout2(s4,s5); }
#define eout6(s1,s2,s3,s4,s5,s6)	    	{ eout3(s1,s2,s3); eout3(s4,s5,s6); }
#define eout7(s1,s2,s3,s4,s5,s6,s7) 		{ eout3(s1,s2,s3); eout4(s4,s5,s6,s7); }
#define eout8(s1,s2,s3,s4,s5,s6,s7,s8)		{ eout3(s1,s2,s3); eout5(s4,s5,s6,s7,s8); }
#define eout9(s1,s2,s3,s4,s5,s6,s7,s8,s9)	{ eout3(s1,s2,s3); eout6(s4,s5,s6,s7,s8,s9); }
extern void eoutsa();
extern void epid();
extern void eflush();
extern void eoutclean();
extern void eoutulong();
