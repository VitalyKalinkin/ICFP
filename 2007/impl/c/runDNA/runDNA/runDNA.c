/*
 * Usage:
 *
 *   * reads the prefix from stdin
 *   * ouputs the RNA to stdout
 *
 * Read the Makefile to build the examples.
 *
 */


#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define DEBUG 1
#define MAX_COST (3 * 1000000000U)

#include <assert.h>

int show_debugging = 0;
unsigned cost = 0; // cost must be at least 32-bits long

/* Simple struct to make inserts quickly. */

typedef struct
{
  char *buffer;
  int references;
} BufferSrc;

BufferSrc *
new_buffer_src ( char *buffer )
{
  BufferSrc *new;

  if (buffer == NULL)
    return NULL;

  new = malloc(sizeof(BufferSrc));
  new->buffer = buffer;
  new->references = 0;
  return new;
}

void
buffer_src_deref ( BufferSrc *buffer )
{
  assert(buffer->references > 0);
  if (-- buffer->references > 0)
    return;

  free (buffer->buffer);
  free (buffer);
}

/* Simple list for small things */

typedef struct ListNode ListNode;
typedef struct List List;

struct List {
  ListNode *head;
  ListNode *tail;
  int len;
};

struct ListNode {
  union {
    int number;
    List *strl;
    struct {
      /* B for base, K for skip, / for search, ( for open, ) for close. */
      char type;
      union {
        int  num; /* used for K */
        char base; /* used for B */
        char *search; /* used for / */
      } data;
    } pattern;
    struct {
      /* B for base, P for protect(ref n_l), | for length */
      char type;
      union {
        char base;
        struct {
          int l;
          int n;
        } ref;
        int len;
      } data;
    } template;
    /* For STRL */
    struct {
      char *core;
      int len;
      BufferSrc *src;
    } buffer;
  } data;
  ListNode *next;
  ListNode *prev;
};

void
list_init ( List *n )
{
  n->head = n->tail = NULL;
  n->len = 0;
}

List *
list_new ( )
{
  List *n = malloc(sizeof(List));
  list_init(n);
  return n;
}

void
list_add_head ( List *l )
{
  ListNode *tmp = malloc(sizeof(ListNode));

  assert(l);

  tmp->prev = NULL;
  tmp->next = l->head;

  if (l->head)
    l->head->prev = tmp;
  else
    l->tail = tmp;

  l->head = tmp;

  l->len ++;
}

void
list_add_tail ( List *l )
{
  ListNode *tmp = malloc(sizeof(ListNode));

  assert(l);

  tmp->next = NULL;
  tmp->prev = l->tail;

  if (l->head)
    l->tail->next = tmp;
  else
    l->head = tmp;

  l->tail = tmp;

  l->len ++;
}

void
list_remove_head ( List *l )
{
  ListNode *tmp = l->head->next;

  assert(l);
  assert(l->head);
  assert(l->tail);

  free (l->head);
  l->head = tmp;

  if (!l->head)
    l->tail = l->head;
  else
    l->head->prev = NULL;

  l->len--;
}

void
list_remove_tail ( List *l )
{
  ListNode *tmp = l->tail->prev;

  assert(l);
  assert(l->head);
  assert(l->tail);

  free (l->tail);
  l->tail = tmp;

  if (!l->tail)
    l->head = l->tail;
  else
    l->tail->next = NULL;

  l->len--;
}

void
list_clean ( List *l )
{
  assert(l);

  while (l->head) {
    ListNode *tmp;
    tmp = l->head->next;
    free (l->head);
    l->head = tmp;
  }
  l->tail = l->head;
  l->len = 0;
}

void
list_free ( List *l )
{
  assert(l);
  list_clean(l);
  free (l);
}

ListNode *
list_ref ( List *l, int n )
{
  ListNode *node;

  assert(l);

  for (node = l->head; node; node = node->next)
    if (n-- == 0)
      return node;
  return NULL;
}

int
list_length ( List *s )
{
#if 0
  int len = 0;
  ListNode *tmp;
#endif

  assert(s);
  return s->len;
#if 0
  /* thsi was the old version, notice how I tested this */

  for (tmp = s->head; tmp; tmp = tmp->next)
    len ++;

  assert(len >= 0);

  if(s->len != len)
  {
    fprintf(stderr, "%d %d\n", s->len, len);
    exit(1);
  }

  return len;
#endif
}

/* STRL */

List dna;

unsigned int
strl_length ( List *s )
{
  unsigned int len = 0;
  ListNode *tmp;
  for (tmp = s->head; tmp; tmp = tmp->next)
    len += tmp->data.buffer.len;
  return (len);
}

void
show_strl ( List *strl )
{
  ListNode *tmp = strl->head;
  unsigned int i = 0;
  unsigned int len;

  for (len = 0; tmp && len < 10; len ++) {
    assert(tmp->data.buffer.len > 0);
    assert(tmp->data.buffer.len > i);
    fprintf(stderr, "%c", tmp->data.buffer.core[i]);
    if (++ i == tmp->data.buffer.len) {
      tmp = tmp->next;
      i = 0;
      if (!tmp) {
        fprintf(stderr, " (%d bases in %u lists)\n", strl_length(strl), list_length(strl));
        return;
      }
    }
  }

  fprintf(stderr, "... (%u bases in %d lists)\n", strl_length(strl), list_length(strl));
}

/* STRL HANDLING */

List *
strl_subseq ( List *s, int start, int end )
{
  ListNode *read;
  List *new = list_new();

  int len = end - start;

  for (read = s->head; read && read->data.buffer.len <= start; read = read->next)
    start -= read->data.buffer.len;

  assert(!read || start < read->data.buffer.len);

  while (read && len > 0) {
    list_add_tail(new);

    assert(read->data.buffer.len > 0);

    new->tail->data.buffer.core = read->data.buffer.core + start;

    new->tail->data.buffer.len = read->data.buffer.len - start;
    if (new->tail->data.buffer.len > len)
      new->tail->data.buffer.len = len;

    assert(new->tail->data.buffer.len > 0);

    new->tail->data.buffer.src = read->data.buffer.src;

    assert(read->data.buffer.src);
    assert(read->data.buffer.src->references > 0);
    read->data.buffer.src->references ++;
    len -= new->tail->data.buffer.len;
    read = read->next;
    start = 0;
  }

  return new;
}

void
strl_prepend_from_buffer_src ( List *str, BufferSrc *src, char *start, int len )
{
  list_add_head(str);
  assert(len > 0);
  src->references ++;
  str->head->data.buffer.core = start;
  str->head->data.buffer.len = len;
  str->head->data.buffer.src = src;
}

#define MIN_BUFSIZE 256

void
prepend_char ( List *s, char c )
{
  char *buffer;

  if (s->head && s->head->data.buffer.src->references == 1 && s->head->data.buffer.src->buffer < s->head->data.buffer.core) {
    s->head->data.buffer.core --;
    s->head->data.buffer.len ++;
    *s->head->data.buffer.core = c;
    assert(s->head->data.buffer.core >= s->head->data.buffer.src->buffer);
    return;
  }

  buffer = malloc(MIN_BUFSIZE);
  buffer[MIN_BUFSIZE - 1] = c;
  strl_prepend_from_buffer_src(s, new_buffer_src(buffer), buffer + MIN_BUFSIZE - 1, 1);
}

void
prepend_asnat ( List *s, int num )
{
  char *buffer = NULL;
  int bufsize = 0;
  int bufpos = 0;
  for (;;)
    {
      if (bufsize == bufpos)
        {
          bufsize = bufsize ? bufsize * 2 : 32;
          buffer = realloc(buffer, bufsize);
        }
      if (num == 0) {
        buffer[bufpos ++] = 'P';
        strl_prepend_from_buffer_src(s, new_buffer_src(buffer), buffer, bufpos);
        return;
      } else if (num % 2 == 0) {
        buffer[bufpos ++] = 'I';
      } else {
        buffer[bufpos ++] = 'C';
      }
      num = num / 2;
    }
}

int
protect_char_length ( char c, int times )
{
  if (! times)
    return 1;
  switch (c)
    {
    case 'I':
      return protect_char_length('C', times - 1);
      break;
    case 'C':
      return protect_char_length('F', times - 1);
      break;
    case 'F':
      return protect_char_length('P', times - 1);
      break;
    case 'P':
      return protect_char_length('I', times - 1) + protect_char_length('C', times - 1);
      break;
    default:
      fprintf(stderr, "Invalid base found, probably because of bugs\n");
    }
  return 0;
}

int
protect_length ( ListNode *s, int times )
{
  int len = 0;
  int skip = 0;

  if (!s)
    return 0;
 
  /* Using a lower bound here: if you quote t times a sequence of len l, you'll
   * get a sequence at least l * 2 ^ ( times / 4 - 1) bytes long.  We check
   * to make sure that this is less than 25M.  And we take log_2 at both sides
   * and ignore the log_2(l). */

  if (times / 4 - 1 > 25)
    {
      fprintf(stderr, "Requested protect of %d times (buffer has %d+ bytes), things would fail.\n", times, s->data.buffer.len);
      exit(EXIT_FAILURE);
    }

  for (;;)
    {
      len += protect_char_length(s->data.buffer.core[skip ++], times);
      if (len > 25000000)
        {
          fprintf(stderr, "Protect of times %d would alloc more than 25M bases, aborting\n", times);
          exit(EXIT_FAILURE);
        }

      assert(len >= 0);
      if (skip == s->data.buffer.len) {
        s = s->next;
        if (!s)
          return len;
        skip = 0;
      }
    }
}

void
strl_remove_head ( List *s )
{
  if (!s->head)
    return;
  buffer_src_deref(s->head->data.buffer.src);
  list_remove_head(s);
}

void
strl_free ( List *s )
{
  while (s->head)
    strl_remove_head(s);
  list_free(s);
}

void
protect_to_buffer_char ( char c, int times, char **dst )
{
  if (times == 0) {
    **dst = c;
    (*dst) ++;
    return;
  }

  switch (c)
    {
    case 'I':
      protect_to_buffer_char('C', times - 1, dst);
      break;
    case 'C':
      protect_to_buffer_char('F', times - 1, dst);
      break;
    case 'F':
      protect_to_buffer_char('P', times - 1, dst);
      break;
    case 'P':
      protect_to_buffer_char('I', times - 1, dst);
      protect_to_buffer_char('C', times - 1, dst);
      break;
    default:
      fprintf(stderr, "Invalid base found, probably because of bugs\n");
    }
}

void
protect_to_buffer ( char *dst, ListNode *head, int times )
{
  while (head)
    {
      int i;
      for (i = 0; i < head->data.buffer.len; i++)
        protect_to_buffer_char(head->data.buffer.core[i], times, &dst);
      head = head->next;
    }
}

void
prepend_protect ( List *s, List *protect, int times )
{
  char *buffer;
  int bufsize;

  if (times == 0)
    {
      ListNode *tmp;

      for (tmp = protect->tail; tmp; tmp = tmp->prev)
        strl_prepend_from_buffer_src(s, tmp->data.buffer.src, tmp->data.buffer.core, tmp->data.buffer.len);

      return;
    }

  bufsize = protect_length(protect->head, times);
  assert(bufsize <= 25000000);
  buffer = malloc(bufsize);

  /* for a call to protect with a level of at least 1, the cost is equal to the length of the
     returned DNA string – however, a call to protect with level 0 is cost-free. */
  cost += bufsize;

  protect_to_buffer(buffer, protect->head, times);
  strl_prepend_from_buffer_src(s, new_buffer_src(buffer), buffer, bufsize);
}

char
dna_get_current ()
{
  return *dna.head->data.buffer.core;
}

void
dna_skip ( int s )
{
  while (dna.head) {
    if (s >= dna.head->data.buffer.len) {
      s -= dna.head->data.buffer.len;
      strl_remove_head(&dna);
    } else {
      dna.head->data.buffer.core += s;
      dna.head->data.buffer.len  -= s;
      assert(dna.head->data.buffer.len > 0);
      return;
    }
  }
}

int
dna_match ( char *str )
{
  ListNode *tmp = dna.head;
  int skip = 0;
  int len = 0;

  if (!tmp)
    return 0;

  while (*str) {
    if (skip == tmp->data.buffer.len) {
      skip = 0;
      tmp = tmp->next;
      if (!tmp)
        return 0;
    }
    if (*str != tmp->data.buffer.core[skip])
      return 0;

    str ++;
    skip ++;
    len ++;
  }

  dna_skip(len);
  return 1;
}

/* If it has less than an average of OPTIMIZE_THRESHOLD bytes per list,
 * it gets optimized. */

#define OPTIMIZE_THRESHOLD 50000

void
strl_optimize ( List *s )
{
  unsigned int len = strl_length(s);
  char *buffer;
  int pos = 0;

  if (len > 25000000) {
    fprintf(stderr, "Program exceeds maximum number of bases: %u\n", len);
    exit(EXIT_FAILURE);
  }

  if (len == 0 || len > list_length(s) * OPTIMIZE_THRESHOLD)
    return;

  fprintf(stderr, "Optimize (len %u)\n", len);

  buffer = malloc(len);
  if (!buffer)
    {
      fprintf(stderr, "malloc fails: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

  while (s->head) {
    memcpy(buffer + pos, s->head->data.buffer.core, s->head->data.buffer.len);
    pos += s->head->data.buffer.len;
    strl_remove_head(s);
  }

  strl_prepend_from_buffer_src(s, new_buffer_src(buffer), buffer, len);
}

/* READ DNA */

void
dna_read_file ( FILE *in )
{
  unsigned int bufsize = 0;
  unsigned int bufpos = 0;
  char *buffer = NULL;

  for (;;)
    {
      int c = fgetc(in);
      if (c == EOF)
        {
          if (buffer)
            strl_prepend_from_buffer_src(&dna, new_buffer_src(buffer), buffer, bufpos);
          return;
        }
      if (c == 'I' || c == 'C' || c == 'F' || c == 'P')
        {
          if (bufpos == bufsize)
            {
              bufsize = bufsize ? bufsize * 2 : 1024;
              buffer = realloc(buffer, bufsize);
            }
          buffer[bufpos ++] = c;
        }
    }
}

void
dna_init ()
{
  FILE *in;

  show_debugging = getenv("DEBUG") != NULL;

  list_init(&dna);

  in = fopen("endo.dna", "r");
  if (!in)
    {
      fprintf(stderr, "endo.dna: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

  dna_read_file(in);
  fclose(in);

  dna_read_file(stdin);
}

/* OUTPUT RNA */

int rna_push_count = 0;

void
show_rna ()
{
  if (!show_debugging)
    return;
  fprintf(stderr, "len(rna) = %d\n", rna_push_count);
}

void
rna_push ()
{
  int i;
  for (i = 0; i < 7; i ++) {
    putchar(dna_get_current());
    dna_skip(1);
  }
  rna_push_count ++;
}

/* CONVENIENCE FUNCTIONS */

void
finish ()
{
  exit(0);
}

int
dna_nat ()
{
  if (dna_match("P")) {
    return 0;
  } else if (dna_match("I") || dna_match("F")) {
    return 2 * dna_nat();
  } else if (dna_match("C")) {
    return 2 * dna_nat() + 1;
  } else {
    finish();
    return 0;
  }
}

char *dna_const ()
{
  char *buffer = NULL;
  int   bufsize = 0;
  int   bufpos = 0;

  for (;;)
    {
      if (bufpos == bufsize) {
        bufsize = bufsize ? bufsize * 2 : 32;
        buffer = realloc(buffer, bufsize + 1);
      }
      if (dna_match("C")) {
        buffer[bufpos ++] = 'I';
      } else if (dna_match("F")) {
        buffer[bufpos ++] = 'C';
      } else if (dna_match("P")) {
        buffer[bufpos ++] = 'F';
      } else if (dna_match("IC")) {
        buffer[bufpos ++] = 'P';
      } else {
        buffer[bufpos] = 0;
        return(buffer);
      }
    }
}


/* PATTERNS */

void
pattern_alloc ( char type, List *pat )
{
  list_add_tail(pat);
  pat->tail->data.pattern.type = type;
}

void
pattern_base ( char base, List *pat )
{
  pattern_alloc('B', pat);
  pat->tail->data.pattern.data.base = base;

  // in pattern each base consumed has a cost of 1
  cost ++;
}

void
pattern_skip ( int num, List *pat )
{
  pattern_alloc('K', pat);
  pat->tail->data.pattern.data.num = num;
}

void
pattern_search ( char *search, List *pat )
{
  pattern_alloc('/', pat);
  pat->tail->data.pattern.data.search = search;
}

void
pattern_open ( List *pat )
{
  pattern_alloc('(', pat);
}

void
pattern_close ( List *pat )
{
  pattern_alloc(')', pat);
}

void
pattern_free ( ListNode *pat )
{
  if (pat->data.pattern.type == '/')
    free (pat->data.pattern.data.search);
}

void
pattern_clean ( List *pat )
{
  ListNode *tmp;
  for (tmp = pat->head; tmp; tmp = tmp->next)
    pattern_free(tmp);
  list_clean(pat);
}

void
pattern ( List *pat )
{
  int lvl = 0;

  for (;;) {
    if (dna_match("C")) {
      pattern_base('I', pat);
    } else if (dna_match("F")) {
      pattern_base('C', pat);
    } else if (dna_match("P")) {
      pattern_base('F', pat);
    } else if (dna_match("IC")) {
      pattern_base('P', pat);
    } else if (dna_match("IP")) {
      pattern_skip(dna_nat(), pat);
    } else if (dna_match("IF")) {
      dna_skip(1);
      pattern_search(dna_const(), pat);
    } else if (dna_match("IIP")) {
      pattern_open(pat);
      lvl ++;
    } else if (dna_match("IIC") || dna_match("IIF")) {
      if (lvl == 0) {
        return;
      } else {
        pattern_close(pat);
        lvl --;
      }
    } else if (dna_match("III")) {
      rna_push();
    } else {
      finish();
    }
  }
}

/* TEMPLATES */

void
template_malloc ( char type, List *tem )
{
  list_add_tail(tem);
  tem->tail->data.template.type = type;
}

void
template_base ( char base, List *tem )
{
  template_malloc('B', tem);
  tem->tail->data.template.data.base = base;

  // in template each base consumed has a cost of 1
  cost ++;
}

void
template_ref ( int l, int n, List *tem )
{
  assert(l >= 0);
  assert(n >= 0);
  template_malloc('P', tem);
  tem->tail->data.template.data.ref.l = l;
  tem->tail->data.template.data.ref.n = n;
}

void
template_len ( int n, List *tem )
{
  assert(n >= 0);
  template_malloc('|', tem);
  tem->tail->data.template.data.len = n;
}

void
template ( List *tem )
{
  for (;;) {
    if (dna_match("C")) {
      template_base('I', tem);
    } else if (dna_match("F")) {
      template_base('C', tem);
    } else if (dna_match("P")) {
      template_base('F', tem);
    } else if (dna_match("IC")) {
      template_base('P', tem);
    } else if (dna_match("IF") || dna_match("IP")) {
      int l = dna_nat();
      int n = dna_nat();
      template_ref(l, n, tem);
    } else if (dna_match("IIC") || dna_match("IIF")) {
      return;
    } else if (dna_match("IIP")) {
      template_len(dna_nat(), tem);
    } else if (dna_match("III")) {
      rna_push();
    } else {
      finish();
    }
  }
}

/* MATCH REPLACE */

void
matchreplace_skip ( ListNode **node, int *skip, int *i, int advance )
{
  if (!*node)
    return;
  *i += advance;
  while (*skip + advance >= (*node)->data.buffer.len)
    {
      advance = advance - ((*node)->data.buffer.len - *skip);
      *skip = 0;
      *node = (*node)->next;
      if (!*node)
        return;
    }
  *skip += advance;
  assert(*skip < (*node)->data.buffer.len);
}

int
at_match ( char *str, ListNode *tmp, int i )
{
  while (*str)
    {
      if (*str != tmp->data.buffer.core[i])
        return 0;
      str ++;
      i ++;
      if (i == tmp->data.buffer.len) {
        tmp = tmp->next;
        if (!tmp)
          return *str == 0;
        i = 0;
      }
    }
  return 1;
}

int
dna_search ( char *str, int i )
{
  ListNode *tmp = dna.head;
  int len = i;

  while (tmp && i >= tmp->data.buffer.len) {
    i -= tmp->data.buffer.len;
    tmp = tmp->next;
  }

  len -= i;

  while (tmp)
    {
      if (at_match(str, tmp, i))
        return strlen(str) + len + i;
      i ++;
      if (i == tmp->data.buffer.len) {
        len += i;
        i = 0;
        tmp = tmp->next;
      }
    }
  return -1;
}

void
env_clean ( List *env )
{
  while (env->head)
    {
      strl_free(env->head->data.strl);
      list_remove_head(env);
    }
}

void
show_environment ( List *e )
{
  int i = 0;
  ListNode *tmp;

  if (!show_debugging)
    return;

  for (tmp = e->head; tmp; tmp = tmp->next) {
    fprintf(stderr, "e[%d] = ", i ++);
    show_strl(tmp->data.strl);
  }
}

void
matchreplace ( List *pat, List *tem )
{
  void replace ( List *tpl, List *env );

  int i = 0, search;
  List e;
  List c;

  ListNode *dna_i = dna.head;
  int dna_i_skip = 0;

  list_init(&e);
  list_init(&c);

  while (pat->head)
    {
      switch (pat->head->data.pattern.type)
        {
        case 'B':
          if (dna_i && dna_i->data.buffer.core[dna_i_skip] == pat->head->data.pattern.data.base) {
            matchreplace_skip(&dna_i, &dna_i_skip, &i, 1);
            /* during matchreplace, a comparison of the current base
               with a constant base pattern has a cost of 1 */
            cost ++;
          } else {
            goto matchreplace_fail;
          }
          break;
        case 'K':
          matchreplace_skip(&dna_i, &dna_i_skip, &i, pat->head->data.pattern.data.num);
          if (!dna_i)
            goto matchreplace_fail;
          // skips are cost free
          break;
        case '/':
          search = dna_search(pat->head->data.pattern.data.search, i);
          if (search == -1)
            {
              // during matchreplace, a failed search has a cost of len (dna) - i
              cost += strl_length(&dna) - i;
              goto matchreplace_fail;
            }
          matchreplace_skip(&dna_i, &dna_i_skip, &i, search - i);
          // during matchreplace, a succesful search has a cost of n − i
          cost += search - i;
          break;
        case '(':
          list_add_head(&c);
          c.head->data.number = i;
          break;
        case ')':
          list_add_tail(&e);
          e.tail->data.strl = strl_subseq(&dna, c.head->data.number, i);
          assert(strl_length(e.tail->data.strl) == i - c.head->data.number);
          list_remove_head(&c);
          break;
        default:
          fprintf(stderr, "Bad pattern in matchreplace\n");
        }
      pattern_free(pat->head);
      list_remove_head(pat);
    }

  if (show_debugging)
    fprintf(stderr, "succesful match of length %d\n", i);
  show_environment(&e);
  list_clean(&c);
  dna_skip(i);
  replace(tem, &e);
  env_clean(&e);
  return;

matchreplace_fail:
  env_clean(&e);
  list_clean(&c);
  if (show_debugging)
    fprintf(stderr, "failed match\n");
}

/* Note that we go backwards in tpl, appending everything
 * directly to the DNA. */

void
replace ( List *tpl, List *env )
{
  ListNode *tmp;
  while (tpl->tail)
    {
      switch (tpl->tail->data.template.type)
        {
        case 'B':
          prepend_char(&dna, tpl->tail->data.template.data.base);
          break;
        case 'P':
          tmp = list_ref(env, tpl->tail->data.template.data.ref.n);
          if (tmp)
            prepend_protect(&dna, tmp->data.strl, tpl->tail->data.template.data.ref.l);
          break;
        case '|':
          tmp = list_ref(env, tpl->tail->data.template.data.len);
          prepend_asnat(&dna, tmp ? strl_length(tmp->data.strl) : 0);
          break;
        default:
          fprintf(stderr, "Invalid template found, probably because of bugs\n");
        }
      list_remove_tail(tpl);
    }
}

void
show_dna ()
{
  if (!show_debugging)
    return;
  fprintf(stderr, "dna = ");
  show_strl(&dna);
}

void
show_pattern ( List *pat )
{
  ListNode *tmp;

  if (!show_debugging)
    return;

  fprintf(stderr, "pattern  ");
  for (tmp = pat->head; tmp; tmp = tmp->next)
    switch (tmp->data.pattern.type)
      {
      case 'B':
        fprintf(stderr, "%c", tmp->data.pattern.data.base);
        break;
      case 'K':
        fprintf(stderr, "!%d", tmp->data.pattern.data.num);
        break;
      case '/':
        fprintf(stderr, "?\"%s\"", tmp->data.pattern.data.search);
        break;
      case '(':
      case ')':
        fprintf(stderr, "%c", tmp->data.pattern.type);
        break;
      default:
        fprintf(stderr, "Bad pattern in show_pattern\n");
      }
  fprintf(stderr, "\n");
}

void
show_template ( List *tem )
{
  ListNode *tmp;

  if (!show_debugging)
    return;

  fprintf(stderr, "template ");
  for (tmp = tem->head; tmp; tmp = tmp->next)
    switch (tmp->data.template.type)
      {
      /* B for base, P for protect(ref n_l), | for length */
      case 'B':
        fprintf(stderr, "%c", tmp->data.template.data.base);
        break;
      case 'P':
        if (tmp->data.template.data.ref.l > 0)
          fprintf(stderr, "\\%d_%d", tmp->data.template.data.ref.n, tmp->data.template.data.ref.l);
        else
          fprintf(stderr, "\\%d", tmp->data.template.data.ref.n);
        break;
      case '|':
        fprintf(stderr, "|%d|", tmp->data.template.data.len);
        break;
      default:
        fprintf(stderr, "Bad template in show_template\n");
      }
  fprintf(stderr, "\n");
}

/* LOGIC */

void
execute ()
{
  int it = 0;
  List pat;
  List tem;

  int max_steps = getenv("NSTEPS") != NULL ? atoi(getenv("NSTEPS")) : -1;

  while (it != max_steps && cost < MAX_COST) {
    if (show_debugging || it % 100000 == 0)
      fprintf(stderr, "\niteration %d\n", it);
    strl_optimize(&dna);
    list_init(&pat);
    list_init(&tem);
    show_dna();
    pattern(&pat);
    show_pattern(&pat);
    template(&tem);
    show_template(&tem);
    matchreplace(&pat, &tem);
    pattern_clean(&pat);
    list_clean(&tem);
    show_rna ();
    it ++;
  }
  if(cost >= MAX_COST) {
    fprintf(stderr, "*** Cost exceeded!\n");
    exit(EXIT_FAILURE);
  }
  if (show_debugging)
    fprintf(stderr, "Clean up...\n");
  while (dna.head)
    strl_remove_head(&dna);
}

void
toggle_debugging ( int sig )
{
  show_debugging = !show_debugging;
//  signal(SIGUSR1, toggle_debugging);
}

int
main ()
{
  //signal(SIGUSR1, toggle_debugging);
  dna_init();
  execute();
  return 0;
}
