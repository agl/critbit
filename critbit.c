#define _POSIX_C_SOURCE  200112
#define uint8  uint8_t
#define uint32  uint32_t

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <errno.h>

typedef struct {
  void *child[2];
  uint32 byte;
  uint8 otherbits;
} critbit0_node;

typedef struct {
  void *root;
} critbit0_tree;

  int
  critbit0_contains(critbit0_tree *t, const char *u) {
    const uint8 *ubytes = (void *) u;
    const size_t ulen = strlen(u);
    uint8 *p = t->root;

    
      if (!p) return 0;

    
      while (1 & (intptr_t) p) {
        critbit0_node *q = (void *) (p - 1);
        
          uint8 c = 0;
          if (q->byte < ulen) c = ubytes[q->byte];
          const int direction = (1 + (q->otherbits | c)) >> 8;
    
        p = q->child[direction];
      }

    
      return 0 == strcmp(u, (const char *) p);

  }

int critbit0_insert(critbit0_tree *t, const char *u)
{
  const uint8 *const ubytes = (void *) u;
  const size_t ulen = strlen(u);
  uint8 *p = t->root;

  
    if (!p) {
      char *x;
      int a = posix_memalign((void **) &x, sizeof(void *), ulen + 1);
      if (a) return 0;
      memcpy(x, u, ulen + 1);
      t->root = x;
      return 2;
    }

  
    while (1 & (intptr_t) p) {
      critbit0_node *q = (void *) (p - 1);
      
        uint8 c = 0;
        if (q->byte < ulen) c = ubytes[q->byte];
        const int direction = (1 + (q->otherbits | c)) >> 8;
  
      p = q->child[direction];
    }

  
    
      uint32 newbyte;
      uint32 newotherbits;
    
      for (newbyte = 0; newbyte < ulen; ++newbyte) {
        if (p[newbyte] != ubytes[newbyte]) {
          newotherbits = p[newbyte] ^ ubytes[newbyte];
          goto different_byte_found;
        }
      }
    
      if (p[newbyte] != 0) {
        newotherbits = p[newbyte];
        goto different_byte_found;
      }
      return 1;
    
     different_byte_found:
  
    
      while (newotherbits & (newotherbits - 1)) newotherbits &= newotherbits - 1;
      newotherbits ^= 255;
      uint8 c = p[newbyte];
      int newdirection = (1 + (newotherbits | c)) >> 8;
  

  
    
      critbit0_node *newnode;
      if (posix_memalign((void **) &newnode, sizeof(void *), sizeof(critbit0_node))) return 0;
    
      char *x;
      if (posix_memalign((void **) &x, sizeof(void *), ulen + 1)) {
        free(newnode);
        return 0;
      }
      memcpy(x, ubytes, ulen + 1);
    
      newnode->byte = newbyte;
      newnode->otherbits = newotherbits;
      newnode->child[1 - newdirection] = x;
  
    
      void **wherep = &t->root;
      for (;;) {
        uint8 *p = *wherep;
        if (!(1 & (intptr_t) p)) break;
        critbit0_node *q = (void *) (p - 1);
        if (q->byte > newbyte) break;
        if (q->byte == newbyte && q->otherbits > newotherbits) break;
        uint8 c = 0;
        if (q->byte < ulen) c = ubytes[q->byte];
        const int direction = (1 + (q->otherbits | c)) >> 8;
        wherep = q->child + direction;
      }
    
      newnode->child[newdirection] = *wherep;
      *wherep = (void *) (1 + (char *) newnode);
  


  return 2;
}

int critbit0_delete(critbit0_tree *t, const char *u) {
  const uint8 *ubytes = (void *) u;
  const size_t ulen = strlen(u);
  uint8 *p = t->root;
  void **wherep = &t->root;
  void **whereq = 0;
  critbit0_node *q = 0;
  int direction = 0;

  
    if (!p) return 0;

  
    while (1 & (intptr_t) p) {
      whereq = wherep;
      q = (void *) (p - 1);
      uint8 c = 0;
      if (q->byte < ulen) c = ubytes[q->byte];
      direction = (1 + (q->otherbits | c)) >> 8;
      wherep = q->child + direction;
      p = *wherep;
    }

  
    if (0 != strcmp(u, (const char *) p)) return 0;
    free(p);

  
    if (!whereq) {
      t->root = 0;
      return 1;
    }
  
    *whereq = q->child[1 - direction];
    free(q);


  return 1;
}

static void
traverse(void *top) {
  
    uint8 *p = top;
  
    if (1 & (intptr_t) p) {
      critbit0_node *q = (void *) (p - 1);
      traverse(q->child[0]);
      traverse(q->child[1]);
      free(q);
    } else {
      free(p);
    }

}

void critbit0_clear(critbit0_tree *t)
{
  if (t->root) traverse(t->root);
  t->root = NULL;
}

static int
allprefixed_traverse(uint8 *top,
                     int (*handle) (const char *, void *), void *arg) {
  
    if (1 & (intptr_t) top) {
      critbit0_node *q = (void *) (top - 1);
      for (int direction = 0; direction < 2; ++direction)
        switch(allprefixed_traverse(q->child[direction], handle, arg)) {
          case 1: break;
          case 0: return 0;
          default: return -1;
        }
      return 1;
    }

  
    return handle((const char *) top, arg);

}

int
critbit0_allprefixed(critbit0_tree *t, const char *prefix,
                     int (*handle) (const char *, void *), void *arg) {
  const uint8 *ubytes = (void *) prefix;
  const size_t ulen = strlen(prefix);
  uint8 *p = t->root;
  uint8 *top = p;

  if (!p) return 1; /* S = $\emptyset$ */
  
    while (1 & (intptr_t) p) {
      critbit0_node *q = (void *) (p - 1);
      uint8 c = 0;
      if (q->byte < ulen) c = ubytes[q->byte];
      const int direction = (1 + (q->otherbits | c)) >> 8;
      p = q->child[direction];
      if (q->byte < ulen) top = p;
    }

  
    for (size_t i = 0; i < ulen; ++i) {
      if (p[i] != ubytes[i]) return 1;
    }


  return allprefixed_traverse(top, handle, arg);
}

