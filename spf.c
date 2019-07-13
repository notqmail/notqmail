#include "stralloc.h"
#include "strsalloc.h"
#include "alloc.h"
#include "ip.h"
#include "ipalloc.h"
#include "ipme.h"
#include "str.h"
#include "fmt.h"
#include "scan.h"
#include "byte.h"
#include "now.h"
#include "dns.h"
#include "case.h"
#include "spf.h"

#define SPF_EXT    -1
#define SPF_SYNTAX -2

#define WSPACE(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n')
#define NXTOK(b, p, a) do { (b) = (p); \
          while((p) < (a)->len && !WSPACE((a)->s[(p)])) ++(p); \
          while((p) < (a)->len && WSPACE((a)->s[(p)])) (a)->s[(p)++] = 0; \
        } while(0)

/* this table and macro came from wget more or less */
/* and was in turn stolen by me from libspf as is :) */
const static unsigned char urlchr_table[256] =
{
  1,  1,  1,  1,   1,  1,  1,  1,   /* NUL SOH STX ETX  EOT ENQ ACK BEL */
  1,  1,  1,  1,   1,  1,  1,  1,   /* BS  HT  LF  VT   FF  CR  SO  SI  */
  1,  1,  1,  1,   1,  1,  1,  1,   /* DLE DC1 DC2 DC3  DC4 NAK SYN ETB */
  1,  1,  1,  1,   1,  1,  1,  1,   /* CAN EM  SUB ESC  FS  GS  RS  US  */
  1,  0,  1,  1,   0,  1,  1,  0,   /* SP  !   "   #    $   %   &   '   */
  0,  0,  0,  1,   0,  0,  0,  1,   /* (   )   *   +    ,   -   .   /   */
  0,  0,  0,  0,   0,  0,  0,  0,   /* 0   1   2   3    4   5   6   7   */
  0,  0,  1,  1,   1,  1,  1,  1,   /* 8   9   :   ;    <   =   >   ?   */
  1,  0,  0,  0,   0,  0,  0,  0,   /* @   A   B   C    D   E   F   G   */
  0,  0,  0,  0,   0,  0,  0,  0,   /* H   I   J   K    L   M   N   O   */
  0,  0,  0,  0,   0,  0,  0,  0,   /* P   Q   R   S    T   U   V   W   */
  0,  0,  0,  1,   1,  1,  1,  0,   /* X   Y   Z   [    \   ]   ^   _   */
  1,  0,  0,  0,   0,  0,  0,  0,   /* `   a   b   c    d   e   f   g   */
  0,  0,  0,  0,   0,  0,  0,  0,   /* h   i   j   k    l   m   n   o   */
  0,  0,  0,  0,   0,  0,  0,  0,   /* p   q   r   s    t   u   v   w   */
  0,  0,  0,  1,   1,  1,  1,  1,   /* x   y   z   {    |   }   ~   DEL */

  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,

  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
  1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,   1,  1,  1,  1,
};


extern stralloc addr;
extern stralloc helohost;
extern char *remoteip;
extern char *local;

extern stralloc spflocal;
extern stralloc spfguess;
extern stralloc spfexp;

static stralloc sender_fqdn = {0};
static stralloc explanation = {0};
static stralloc expdomain = {0};
static stralloc errormsg = {0};
static char *received;

static int recursion;
static struct ip_address ip;

static void hdr_pass() { received = "pass (%{xr}: %{xs} designates %{i} as permitted sender)"; };
static void hdr_softfail() { received = "softfail (%{xr}: transitioning %{xs} does not designate %{i} as permitted sender)"; };
static void hdr_fail() { received = "fail (%{xr}: %{xs} does not designate %{i} as permitted sender)"; };
static void hdr_unknown() { received = "unknown (%{xr}: domain at %{d} does not designate permitted sender hosts)"; };
static void hdr_neutral() { received = "neutral (%{xr}: %{i} is neither permitted nor denied by %{xs})"; };
static void hdr_none() { received = "none (%{xr}: domain at %{d} does not designate permitted sender hosts)"; };
static void hdr_unknown_msg(e) char *e; { stralloc_copys(&errormsg, e); received = "unknown (%{xr}: %{xe})"; };
static void hdr_ext(e) char *e; { stralloc_copys(&errormsg, e); received = "unknown %{xe} (%{xr}: %{xs} uses mechanism not recognized by this client)"; };
static void hdr_syntax() { received = "unknown (%{xr}: parse error in %{xs})"; };
static void hdr_error(e) char *e; { stralloc_copys(&errormsg, e); received = "error (%{xr}: error in processing during lookup of %{d}: %{xe})"; };
static void hdr_dns() { hdr_error("DNS problem"); }


static int matchip(struct ip_address *net, int mask, struct ip_address *ip)
{
	int j;
	int bytemask;

	for (j = 0; j < 4 && mask > 0; ++j) {
		if (mask > 8) bytemask = 8; else bytemask = mask;
		mask -= bytemask;

		if ((net->d[j] ^ ip->d[j]) & (0x100 - (1 << (8 - bytemask))))
			return 0;
	}
	return 1;
}

static int getipmask(char *mask, int ipv6) {
	unsigned long r;
	int pos;

	if (!mask) return 32;

	pos = scan_ulong(mask, &r);
	if (!pos || (mask[pos] && !(mask[pos] == '/' && ipv6))) return -1;
	if (r > 32) return -1;

	return r;
}

int spfget(stralloc *spf, stralloc *domain)
{
	strsalloc ssa = {0};
	int j;
	int begin, pos, i;
	int r = SPF_NONE;

	spf->len = 0;

	switch(dns_txt(&ssa, domain)) {
		case DNS_MEM: return SPF_NOMEM;
		case DNS_SOFT: hdr_dns(); return SPF_ERROR;
		case DNS_HARD: return SPF_NONE;
	}

	for (j = 0;j < ssa.len;++j) {
		pos = 0;

		NXTOK(begin, pos, &ssa.sa[j]);
		if (str_len(ssa.sa[j].s + begin) < 6) continue;
		if (!byte_equal(ssa.sa[j].s + begin,6,"v=spf1")) continue;
		if (ssa.sa[j].s[begin + 6]) {
			/* check for subversion */
			if (ssa.sa[j].s[begin + 6] != '.') continue;
			for(i = begin + 7;;++i)
				if (!(ssa.sa[j].s[i] >= '0' && ssa.sa[j].s[i] <= '9')) break;
			if (i == (begin + 7)) continue;
			if (ssa.sa[j].s[i]) continue;
		}

		if (spf->len > 0) {
			spf->len = 0;
			hdr_unknown_msg("Multiple SPF records returned");
			r = SPF_UNKNOWN;
			break;
		}
		if (!stralloc_0(&ssa.sa[j])) return SPF_NOMEM;
		if (!stralloc_copys(spf,ssa.sa[j].s + pos)) return SPF_NOMEM;
		r = SPF_OK;
	}

	for (j = 0;j < ssa.len;++j)
		alloc_free(ssa.sa[j].s);
	alloc_free(ssa.sa);
	return r;
}

static int spf_ptr(char *spec, char *mask);

int spfsubst(stralloc *expand, char *spec, char *domain)
{
	static char hexdigits[] = "0123456789abcdef";
	stralloc sa = {0};
	char ch;
	int digits = -1;
	int urlencode = 0;
	int reverse = 0;
	int start = expand->len;
	int i, pos;
	char *split = ".";

	if (!stralloc_readyplus(&sa,0)) return 0;

	if (*spec == 'x') { i = 1; ++spec; } else i = 0;
	ch = *spec++;
	if (!ch) { alloc_free(sa.s); return 1; }
	if (ch >= 'A' && ch <= 'Z') { ch += 32; urlencode = 1; }
	if (i) ch -= 32;
	while(*spec >= '0' && *spec <= '9') {
		if (digits < 0) digits = 0;
		if (digits >= 1000000) { digits = 10000000; continue; }
		digits = (digits * 10) + (*spec - '0');
		spec++;
	}

	while((*spec >= 'a' && *spec <= 'z') || (*spec >= 'A' && *spec <= 'Z')) {
		if (*spec == 'r') reverse = 1;
		spec++;
	}

	if (*spec) split = spec;

	switch(ch) {
		case 'l':
			pos = byte_rchr(addr.s, addr.len, '@');
			if (pos < addr.len) {
				if (!stralloc_copyb(&sa, addr.s, pos)) return 0;
			} else
				if (!stralloc_copys(&sa, "postmaster")) return 0;
			break;
		case 's':
			if (!stralloc_copys(&sa, addr.s)) return 0;
			break;
		case 'o':
			pos = byte_rchr(addr.s, addr.len, '@') + 1;
			if (pos > addr.len) break;
			if (!stralloc_copys(&sa, addr.s + pos)) return 0;
			break;
		case 'd':
			if (!stralloc_copys(&sa, domain)) return 0;
			break;
		case 'i':
			if (!stralloc_ready(&sa, IPFMT)) return 0;
			sa.len = ip_fmt(sa.s, &ip);
			break;
		case 't':
			if (!stralloc_ready(&sa, FMT_ULONG)) return 0;
			sa.len = fmt_ulong(sa.s, (unsigned long)now());
			break;
		case 'p':
			if (!sender_fqdn.len)
				spf_ptr(domain, 0);
			if (sender_fqdn.len) {
				if (!stralloc_copy(&sa, &sender_fqdn)) return 0;
			} else
				if (!stralloc_copys(&sa, "unknown")) return 0;
			break;
		case 'v': 
			if (!stralloc_copys(&sa, "in-addr")) return 0;
			break;
		case 'h':
			if (!stralloc_copys(&sa, helohost.s)) return 0; /* FIXME: FQDN? */
			break;
		case 'E':
			if (errormsg.len && !stralloc_copy(&sa, &errormsg)) return 0;
			break;
		case 'R':
			if (!stralloc_copys(&sa, local)) return 0;
			break;
		case 'S':
			if (expdomain.len > 0) {
				if (!stralloc_copys(&sa, "SPF record at ")) return 0;
				if (!stralloc_cats(&sa, expdomain.s)) return 0;
			} else {
				if (!stralloc_copys(&sa, "local policy")) return 0;
			}
			break;
	}

	if (reverse) {
		for(pos = 0; digits; ++pos) {
			pos += byte_cspn(sa.s + pos, sa.len - pos, split);
			if (pos >= sa.len) break;
			if (!--digits) break;
		}

		for(; pos > 0; pos = i - 1) {
			i = byte_rcspn(sa.s, pos, split) + 1;
			if (i > pos) i = 0;
			if (!stralloc_catb(expand, sa.s + i, pos - i)) return 0;
			if (i > 0 && !stralloc_append(expand, ".")) return 0;
		}
	} else {
		for(pos = sa.len; digits; --pos) {
			i = byte_rcspn(sa.s, pos, split) + 1;
			if (i > pos) { pos = 0; break; }
			pos = i;
			if (!--digits) break;
		}

		if (!stralloc_catb(expand, sa.s + pos, sa.len - pos)) return 0;
		if (split[0] != '.' || split[1])
			for(pos = 0; pos < expand->len; pos++) {
				pos += byte_cspn(expand->s + pos, expand->len - pos, split);
				if (pos < expand->len)
					expand->s[pos] = '.';
			}
	}

	if (urlencode) {
		stralloc_copyb(&sa, expand->s + start, expand->len - start);
		expand->len = start;

		for(pos = 0; pos < sa.len; ++pos) {
			ch = sa.s[pos];
			if (urlchr_table[(unsigned char)ch]) {
				if (!stralloc_readyplus(expand, 3)) return 0;
				expand->s[expand->len++] = '%';
				expand->s[expand->len++] = hexdigits[(unsigned char)ch >> 4];
				expand->s[expand->len++] = hexdigits[(unsigned char)ch & 15];
			} else
				if (!stralloc_append(expand, &ch)) return 0;
		}
	}

	alloc_free(sa.s);
	return 1;
}

int spfexpand(stralloc *sa, char *spec, char *domain)
{
	char *p;
	char append;
	int pos;

	if (!stralloc_readyplus(sa, 0)) return 0;
	sa->len = 0;

	for(p = spec; *p; p++) {
		append = *p;
		if (*p == '%') {
			p++;
			switch(*p) {
				case '%': break;
				case '_': append = ' '; break;
				case '-': if (!stralloc_cats(sa, "%20")) return 0; continue;
				case '{':
					pos = str_chr(p, '}');
					if (p[pos] != '}') { p--; break; }
					p[pos] = 0;
					if (!spfsubst(sa, p + 1, domain)) return 0;
					p += pos;
					continue;
				default: p--;
			}
		}
		if (!stralloc_append(sa, &append)) return 0;
	}

	return 1;
}

static int spflookup(stralloc *domain);

static int spf_include(char *spec, char *mask)
{
	stralloc sa = {0};
	int r;

	if (!stralloc_copys(&sa, spec)) return SPF_NOMEM;
	r = spflookup(&sa);
	alloc_free(sa.s);

	switch(r) {
		case SPF_NONE:
			hdr_unknown();
			r = SPF_UNKNOWN;
			break;
		case SPF_SYNTAX:
			r = SPF_UNKNOWN;
			break;
		case SPF_NEUTRAL:
		case SPF_SOFTFAIL:
		case SPF_FAIL:
			r = SPF_NONE;
			break;
	}

	return r;
}

static int spf_a(char *spec, char *mask)
{
	stralloc sa = {0};
	ipalloc ia = {0};
	int ipmask = getipmask(mask, 1);
	int r;
	int j;

	if (ipmask < 0) return SPF_SYNTAX;

	if (!stralloc_copys(&sa, spec)) return SPF_NOMEM;
	if (!stralloc_readyplus(&ia, 0)) return SPF_NOMEM;

	switch(dns_ip(&ia, &sa)) {
		case DNS_MEM: return SPF_NOMEM;
		case DNS_SOFT: hdr_dns(); r = SPF_ERROR; break;
		case DNS_HARD: r = SPF_NONE; break;
		default:
			r = SPF_NONE;
			for(j = 0; j < ia.len; ++j)
				if (matchip(&ia.ix[j].ip, ipmask, &ip)) {
					r = SPF_OK;
					break;
				}
	}

	alloc_free(sa.s);
	alloc_free(ia.ix);
	return r;
}

static int spf_mx(char *spec, char *mask)
{
	stralloc sa = {0};
	ipalloc ia = {0};
	int ipmask = getipmask(mask, 1);
	int random = now() + (getpid() << 16);
	int r;
	int j;

	if (ipmask < 0) return SPF_SYNTAX;

	if (!stralloc_copys(&sa, spec)) return SPF_NOMEM;
	if (!stralloc_readyplus(&ia, 0)) return SPF_NOMEM;

	switch(dns_mxip(&ia, &sa, random)) {
		case DNS_MEM: return SPF_NOMEM;
		case DNS_SOFT: hdr_dns(); r = SPF_ERROR; break;
		case DNS_HARD: r = SPF_NONE; break;
		default:
			r = SPF_NONE;
			for(j = 0; j < ia.len; ++j)
				if (matchip(&ia.ix[j].ip, ipmask, &ip)) {
					r = SPF_OK;
					break;
				}
	}

	alloc_free(sa.s);
	alloc_free(ia.ix);
	return r;
}

static int spf_ptr(char *spec, char *mask)
{
	strsalloc ssa = {0};
	ipalloc ia = {0};
	int len = str_len(spec);
	int r;
	int j, k;
	int pos;

	/* we didn't find host with the matching ip before */
	if (sender_fqdn.len == 7 && str_equal(sender_fqdn.s, "unknown"))
		return SPF_NONE;

	/* the hostname found will probably be the same as before */
	while (sender_fqdn.len) {
		pos = sender_fqdn.len - len;
		if (pos < 0) break;
		if (pos > 0 && sender_fqdn.s[pos - 1] != '.') break;
		if (case_diffb(sender_fqdn.s + pos, len, spec)) break;

		return SPF_OK;
	}

	/* ok, either it's the first test or it's a very weird setup */

	if (!stralloc_readyplus(&ssa, 0)) return SPF_NOMEM;
	if (!stralloc_readyplus(&ia, 0)) return SPF_NOMEM;

	switch(dns_ptr(&ssa, &ip)) {
		case DNS_MEM: return SPF_NOMEM;
		case DNS_SOFT: hdr_dns(); r = SPF_ERROR; break;
		case DNS_HARD: r = SPF_NONE; break;
		default:
			r = SPF_NONE;
			for(j = 0; j < ssa.len; ++j) {
				switch(dns_ip(&ia, &ssa.sa[j])) {
					case DNS_MEM: return SPF_NOMEM;
					case DNS_SOFT: hdr_dns(); r = SPF_ERROR; break;
					case DNS_HARD: break;
					default:
						for(k = 0; k < ia.len; ++k)
							if (matchip(&ia.ix[k].ip, 32, &ip)) {
								if (!sender_fqdn.len)
									if (!stralloc_copy(&sender_fqdn, &ssa.sa[j])) return SPF_NOMEM;

								pos = ssa.sa[j].len - len;
								if (pos < 0) continue;
								if (pos > 0 && ssa.sa[j].s[pos - 1] != '.') continue;
								if (case_diffb(ssa.sa[j].s + pos, len, spec)) continue;

								stralloc_copy(&sender_fqdn, &ssa.sa[j]);
								r = SPF_OK;
								break;
							}
				}

				if (r == SPF_ERROR) break;
			}
	}

	for(j = 0;j < ssa.len;++j)
		alloc_free(ssa.sa[j].s);

	alloc_free(ssa.sa);
	alloc_free(ia.ix);

	if (!sender_fqdn.len)
		if (!stralloc_copys(&sender_fqdn, "unknown")) return SPF_NOMEM;

	return r;
}

static int spf_ip(char *spec, char *mask)
{
	struct ip_address net;
	int ipmask = getipmask(mask, 0);

	if (ipmask < 0) return SPF_SYNTAX;
	if (!ip_scan(spec, &net)) return SPF_SYNTAX;

	if (matchip(&net, ipmask, &ip)) return SPF_OK;

	return SPF_NONE;
}

static int spf_exists(char *spec, char *mask)
{
	stralloc sa = {0};
	ipalloc ia = {0};
	int r;

	if (!stralloc_copys(&sa, spec)) return SPF_NOMEM;
	if (!stralloc_readyplus(&ia, 0)) return SPF_NOMEM;

	switch(dns_ip(&ia, &sa)) {
		case DNS_MEM: return SPF_NOMEM;
		case DNS_SOFT: hdr_dns(); r = SPF_ERROR; break;
		case DNS_HARD: r = SPF_NONE; break;
		default: r = SPF_OK;
	}

	alloc_free(sa.s);
	alloc_free(ia.ix);
	return r;
}

static struct mechanisms {
  char *mechanism;
  int (*func)(char *spec, char *mask);
  unsigned int takes_spec  : 1;
  unsigned int takes_mask  : 1;
  unsigned int expands     : 1;
  unsigned int filldomain  : 1;
  int defresult            : 4;
} mechanisms[] = {
  { "all",      0,          0,0,0,0,SPF_OK   }
, { "include",  spf_include,1,0,1,0,0        }
, { "a",        spf_a,      1,1,1,1,0        }
, { "mx",       spf_mx,     1,1,1,1,0        }
, { "ptr",      spf_ptr,    1,0,1,1,0        }
, { "ip4",      spf_ip,     1,1,0,0,0        }
, { "ip6",      0,          1,1,0,0,SPF_NONE }
, { "exists",   spf_exists, 1,0,1,0,0        }
, { "extension",0,          1,1,0,0,SPF_EXT  }
, { 0,          0,          1,1,0,0,SPF_EXT  }
};

static int spfmech(char *mechanism, char *spec, char *mask, char *domain)
{
	struct mechanisms *mech;
	stralloc sa = {0};
	int r;
	int pos;

	for(mech = mechanisms; mech->mechanism; mech++)
		if (str_equal(mech->mechanism, mechanism)) break;

	if (mech->takes_spec && !spec && mech->filldomain) spec = domain;
	if (!mech->takes_spec != !spec) return SPF_SYNTAX;
	if (!mech->takes_mask && mask) return SPF_SYNTAX;
	if (!mech->func) return mech->defresult;

	if (!stralloc_readyplus(&sa, 0)) return SPF_NOMEM;
	if (mech->expands && spec != domain) {
		if (!spfexpand(&sa, spec, domain)) return SPF_NOMEM;
		for (pos = 0; (sa.len - pos) > 255;) {
			pos += byte_chr(sa.s + pos, sa.len - pos, '.');
			if (pos < sa.len) pos++;
		}
		sa.len -= pos;
		if (pos > 0) byte_copy(sa.s, sa.len, sa.s + pos);
		stralloc_0(&sa);
		spec = sa.s;
	}

	r = mech->func(spec, mask);

	alloc_free(sa.s);
	return r;
}

static struct default_aliases {
  char *alias;
  int defret;
} default_aliases[] = {
  { "allow",   SPF_OK }
, { "pass",    SPF_OK }
, { "deny",    SPF_FAIL }
, { "softdeny",SPF_SOFTFAIL }
, { "fail",    SPF_FAIL }
, { "softfail",SPF_SOFTFAIL }
, { "unknown", SPF_NEUTRAL }
, { 0,         SPF_UNKNOWN }
};

static int spflookup(stralloc *domain)
{
	stralloc spf = {0};
	stralloc sa = {0};
	struct default_aliases *da;
	int main = !recursion;
	int local_pos = -1;
	int r, q;
	int begin, pos;
	int i;
	int prefix;
	int done;
	int guessing = 0;
	char *p;

	if (!stralloc_readyplus(&spf, 0)) return SPF_NOMEM;
	if (!stralloc_readyplus(&sa, 0)) return SPF_NOMEM;

	/* fallthrough result */
	if (main) hdr_none();

redirect:
	if (++recursion > 20) {
		alloc_free(spf.s);
		alloc_free(sa.s);
		hdr_unknown_msg("Maximum nesting level exceeded, possible loop");
		return SPF_SYNTAX;
	}

	if (!stralloc_0(domain)) return SPF_NOMEM;
	if (!stralloc_copy(&expdomain, domain)) return SPF_NOMEM;

	r = spfget(&spf, domain);
	if (r == SPF_NONE) {
		if (!main) { alloc_free(spf.s); return r; }

		if (spfguess.len) {
			/* try to guess */
			guessing = 1;
			if (!stralloc_copys(&spf, spfguess.s)) return SPF_NOMEM;
			if (!stralloc_append(&spf, " ")) return SPF_NOMEM;
		} else
			spf.len = 0;

		/* append local rulest */
		if (spflocal.len) {
			local_pos = spf.len;
			if (!stralloc_cats(&spf, spflocal.s)) return SPF_NOMEM;
		}
		if (!stralloc_0(&spf)) return SPF_NOMEM;

		expdomain.len = 0;
	} else if (r == SPF_OK) {
		if (!stralloc_0(&spf)) return SPF_NOMEM;
		if (main) hdr_neutral();
		r = SPF_NEUTRAL;

		/* try to add local rules before fail all mechs */
		if (main && spflocal.len) {
			pos = 0;
			p = (char *) 0;
			while(pos < spf.len) {
				NXTOK(begin, pos, &spf);
				if (!spf.s[begin]) continue;

				if (p && spf.s[begin] != *p) p = (char *) 0;
				if (!p && (spf.s[begin] == '-' || spf.s[begin] == '~' ||
				           spf.s[begin] == '?')) p = &spf.s[begin];

				if (p && p > spf.s && str_equal(spf.s + begin + 1, "all")) {
					/* ok, we can insert the local rules at p */
					local_pos = p - spf.s;

					stralloc_readyplus(&spf, spflocal.len);
					p = spf.s + local_pos;
					byte_copyr(p + spflocal.len, spf.len - local_pos, p);
					byte_copy(p, spflocal.len, spflocal.s);
					spf.len += spflocal.len;

					pos += spflocal.len;
					break;
				}
			}

			if (pos >= spf.len) pos = spf.len - 1;
			for(i = 0; i < pos; i++)
				if (!spf.s[i]) spf.s[i] = ' ';
		}
	} else {
		alloc_free(spf.s);
		return r;
	}

	pos = 0;
	done = 0;
	while(pos < spf.len) {
		NXTOK(begin, pos, &spf);
		if (!spf.s[begin]) continue;

		/* in local ruleset? */
		if (!done && local_pos >= 0 && begin >= local_pos) {
			if (begin < (local_pos + spflocal.len))
				expdomain.len = 0;
			else
				if (!stralloc_copy(&expdomain, domain))
					return SPF_NOMEM;
		}

		for (p = spf.s + begin;*p;++p)
			if (*p == ':' || *p == '/' || *p == '=') break;

		if (*p == '=') {
			*p++ = 0;

			/* modifiers are simply handled here */
			if (str_equal(spf.s + begin, "redirect")) {
				if (done) continue;

				if (!spfexpand(&sa, p, domain->s)) return SPF_NOMEM;
				stralloc_copy(domain, &sa);

				hdr_unknown();
				r = SPF_UNKNOWN;

				goto redirect;
			} else if (str_equal(spf.s + begin, "default")) {
				if (done) continue;

				for(da = default_aliases; da->alias; ++da)
					if (str_equal(da->alias, p)) break;

				r = da->defret;
			} else if (str_equal(spf.s + begin, "exp")) {
				strsalloc ssa = {0};

				if (!main) continue;

				if (!stralloc_copys(&sa, p)) return SPF_NOMEM;
				switch(dns_txt(&ssa, &sa)) {
					case DNS_MEM: return SPF_NOMEM;
					case DNS_SOFT: continue; /* FIXME... */
					case DNS_HARD: continue;
				}

				explanation.len = 0;
				for(i = 0; i < ssa.len; i++) {
					if (!stralloc_cat(&explanation, &ssa.sa[i])) return SPF_NOMEM;
					if (i < (ssa.len - 1))
						if (!stralloc_append(&explanation, "\n")) return SPF_NOMEM;

					alloc_free(ssa.sa[i].s);
				}
				if (!stralloc_0(&explanation)) return SPF_NOMEM;
			} /* and unknown modifiers are ignored */
		} else if (!done) {
			if (!stralloc_copys(&sa, spf.s + begin)) return SPF_NOMEM;
			if (!stralloc_0(&sa)) return SPF_NOMEM;

			switch(spf.s[begin]) {
				case '-': begin++; prefix = SPF_FAIL; break;
				case '~': begin++; prefix = SPF_SOFTFAIL; break;
				case '+': begin++; prefix = SPF_OK; break;
				case '?': begin++; prefix = SPF_NEUTRAL; break;
				default: prefix = SPF_OK;
			}

			if (*p == '/') {
				*p++ = 0;
				q = spfmech(spf.s + begin, 0, p, domain->s);
			} else {
				if (*p) *p++ = 0;
				i = str_chr(p, '/');
				if (p[i] == '/') {
					p[i++] = 0;
					q = spfmech(spf.s + begin, p, p + i, domain->s);
				} else if (i > 0)
					q = spfmech(spf.s + begin, p, 0, domain->s);
				else
					q = spfmech(spf.s + begin, 0, 0, domain->s);
			}

			if (q == SPF_OK) q = prefix;

			switch(q) {
				case SPF_OK: hdr_pass(); break;
				case SPF_NEUTRAL: hdr_neutral(); break;
				case SPF_SYNTAX: hdr_syntax(); break;
				case SPF_SOFTFAIL: hdr_softfail(); break;
				case SPF_FAIL: hdr_fail(); break;
				case SPF_EXT: hdr_ext(sa.s); break;
				case SPF_ERROR:
					if (!guessing)
						break;
					if (local_pos >= 0 && begin >= local_pos)
						break;
					hdr_none();
					q = SPF_NONE;
					break;
				case SPF_NONE: continue;
			}

			r = q;
			done = 1; /* we're done, no more mechanisms */
		}
	}

	/* we fell through, no local rule applied */
	if (!done && !stralloc_copy(&expdomain, domain)) return SPF_NOMEM;

	alloc_free(spf.s);
	alloc_free(sa.s);
	return r;
}

int spfcheck()
{
	stralloc domain = {0};
	int pos;
	int r;

	pos = byte_rchr(addr.s, addr.len, '@') + 1;
	if (pos < addr.len) {
		if (!stralloc_copys(&domain, addr.s + pos)) return SPF_NOMEM;
	} else {
		pos = str_rchr(helohost.s, '@');
		if (helohost.s[pos]) {
			if (!stralloc_copys(&domain, helohost.s + pos + 1)) return SPF_NOMEM;
		} else
			if (!stralloc_copys(&domain, helohost.s)) return SPF_NOMEM;
	}
	if (!stralloc_copys(&explanation, spfexp.s)) return SPF_NOMEM;
	if (!stralloc_0(&explanation)) return SPF_NOMEM;
	recursion = 0;

	if (!remoteip || !ip_scan(remoteip, &ip)) {
		hdr_unknown_msg("No IP address in conversation");
		return SPF_UNKNOWN;
	}

	if (!stralloc_readyplus(&expdomain, 0)) return SPF_NOMEM;
	if (!stralloc_readyplus(&errormsg, 0)) return SPF_NOMEM;
	expdomain.len = 0;
	errormsg.len = 0;
	sender_fqdn.len = 0;
	received = (char *) 0;

	if ((ip.d[0] == 127 && ip.d[1] == 0 && ip.d[2] == 0 && ip.d[3] == 1) || ipme_is(&ip))
		{ hdr_pass(); r = SPF_OK; }
	else
		r = spflookup(&domain);

	if (r < 0) r = SPF_UNKNOWN;

	alloc_free(domain.s);
	return r;
}

int spfexplanation(sa)
stralloc *sa;
{
	return spfexpand(sa, explanation.s, expdomain.s);
}

int spfinfo(sa)
stralloc *sa;
{
	stralloc tmp = {0};
	if (!stralloc_copys(&tmp, received)) return 0;
	if (!stralloc_0(&tmp)) return 0;
	if (!spfexpand(sa, tmp.s, expdomain.s)) return 0;
	alloc_free(tmp.s);
	return 1;
}
