#include "constmap.h"
#include "alloc.h"
#include "case.h"

static constmap_hash hash(s,len)
char *s;
int len;
{
  unsigned char ch;
  constmap_hash h;
  h = 5381;
  while (len > 0) {
    ch = *s++ - 'A';
    if (ch <= 'Z' - 'A') ch += 'a' - 'A';
    h = ((h << 5) + h) ^ ch;
    --len;
  }
  return h;
}

char *constmap(cm,s,len)
struct constmap *cm;
char *s;
int len;
{
  constmap_hash h;
  int pos;
  h = hash(s,len);
  pos = cm->first[h & cm->mask];
  while (pos != -1) {
    if (h == cm->hash[pos])
      if (len == cm->inputlen[pos])
        if (!case_diffb(cm->input[pos],len,s))
	  return cm->input[pos] + cm->inputlen[pos] + 1;
    pos = cm->next[pos];
  }
  return 0;
}

int constmap_init(cm,s,len,flagcolon)
struct constmap *cm;
char *s;
int len;
int flagcolon;
{
  int i;
  int j;
  int k;
  int pos;
  constmap_hash h;
 
  cm->num = 0;
  for (j = 0;j < len;++j) if (!s[j]) ++cm->num;
 
  h = 64;
  while (h && (h < cm->num)) h += h;
  cm->mask = h - 1;
 
  cm->first = (int *) alloc(sizeof(int) * h);
  if (cm->first) {
    cm->input = (char **) alloc(sizeof(char *) * cm->num);
    if (cm->input) {
      cm->inputlen = (int *) alloc(sizeof(int) * cm->num);
      if (cm->inputlen) {
        cm->hash = (constmap_hash *) alloc(sizeof(constmap_hash) * cm->num);
        if (cm->hash) {
	  cm->next = (int *) alloc(sizeof(int) * cm->num);
	  if (cm->next) {
	    for (h = 0;h <= cm->mask;++h)
	      cm->first[h] = -1;
            pos = 0;
            i = 0;
            for (j = 0;j < len;++j)
              if (!s[j]) {
	        k = j - i;
	        if (flagcolon) {
		  for (k = i;k < j;++k)
		    if (s[k] == ':')
		      break;
		  if (k >= j) { i = j + 1; continue; }
		  k -= i;
		}
                cm->input[pos] = s + i;
                cm->inputlen[pos] = k;
                h = hash(s + i,k);
                cm->hash[pos] = h;
                h &= cm->mask;
                cm->next[pos] = cm->first[h];
                cm->first[h] = pos;
                ++pos;
                i = j + 1;
              }
            return 1;
	  }
	  alloc_free(cm->hash);
	}
        alloc_free(cm->inputlen);
      }
      alloc_free(cm->input);
    }
    alloc_free(cm->first);
  }
  return 0;
}

void constmap_free(cm)
struct constmap *cm;
{
  alloc_free(cm->next);
  alloc_free(cm->hash);
  alloc_free(cm->inputlen);
  alloc_free(cm->input);
  alloc_free(cm->first);
}
