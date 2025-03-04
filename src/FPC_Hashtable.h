#ifndef SRC_FPC_HASHTABLE_H_
#define SRC_FPC_HASHTABLE_H_

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

/*----------------------------------------------------------------------------*/
/* Hash table item                                                            */
/*----------------------------------------------------------------------------*/

/** This structure defines different events and the location **/
typedef struct _FPC_ITEM_S_ {
  char *file_name;
  uint64_t line;
  uint64_t infinity_pos;
  uint64_t infinity_neg;
  uint64_t nan;
  uint64_t division_zero;
  uint64_t cancellation;
  uint64_t comparison;
  uint64_t underflow;
  uint64_t latent_infinity_pos;
  uint64_t latent_infinity_neg;
  uint64_t latent_underflow;
  struct _FPC_ITEM_S_ *next;
} _FPC_ITEM_T_;

//typedef struct _FPC_ITEM_S_ _FPC_ITEM_T_;

/** Program name and input **/
extern int _FPC_PROG_INPUTS;
extern char ** _FPC_PROG_ARGS;


/*----------------------------------------------------------------------------*/
/* Hash table type                                                            */
/*----------------------------------------------------------------------------*/

typedef struct _FPC_HTABLE_S {
  uint64_t size;
  uint64_t n; // number of items
  struct _FPC_ITEM_S_ **table;
} _FPC_HTABLE_T;

//typedef struct _FPC_HTABLE_S _FPC_HTABLE_T;

/*----------------------------------------------------------------------------*/
/* Initialization                                                             */
/*----------------------------------------------------------------------------*/

_FPC_HTABLE_T *_FPC_HT_CREATE_(int64_t size)
{
  _FPC_HTABLE_T *hashtable = NULL;
  int64_t i;

  if(size < 1)
    return NULL;

  // Allocate the table itself
  if(( hashtable = (_FPC_HTABLE_T*)malloc(sizeof(_FPC_HTABLE_T))) == NULL) {
    printf("#FPCHECKER: hash table out of memory error!");
    exit(EXIT_FAILURE);
  }

  // Allocate pointers to the head nodes
  if( (hashtable->table =
      (struct _FPC_ITEM_S_ **)malloc((size_t)((int64_t)sizeof(_FPC_ITEM_T_ *) * size))) == NULL) {
      //(struct _FPC_ITEM_T_ **)malloc(sizeof(_FPC_ITEM_T_ *) * size)) == NULL) {
    printf("#FPCHECKER: hash table out of memory error!");
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < size; i++) {
    hashtable->table[i] = NULL;
  }

  hashtable->size = (uint64_t)size;
  hashtable->n = 0;

  return hashtable;
}

/*----------------------------------------------------------------------------*/
/* Hash function                                                              */
/*----------------------------------------------------------------------------*/

int _FPC_HT_HASH_( _FPC_HTABLE_T *hashtable, _FPC_ITEM_T_ *val)
{
  uint64_t key = (uint64_t)(val->file_name);
  key += val->line;
  return (int)(key % hashtable->size);
}

/*----------------------------------------------------------------------------*/
/* Key-value pair creation                                                    */
/*----------------------------------------------------------------------------*/

_FPC_ITEM_T_ *_FPC_HT_NEWPAIR_(_FPC_ITEM_T_ *val)
{
  _FPC_ITEM_T_ *newpair = NULL;

  if((newpair = (_FPC_ITEM_T_ *)malloc(sizeof(_FPC_ITEM_T_ ))) == NULL) {
    printf("#FPCHECKER: hash table out of memory error!");
    exit(EXIT_FAILURE);
  }

  newpair->file_name            = val->file_name;
  newpair->line                 = val->line;
  newpair->infinity_pos         = val->infinity_pos;
  newpair->infinity_neg         = val->infinity_neg;
  newpair->nan                  = val->nan;
  newpair->division_zero        = val->division_zero;
  newpair->cancellation         = val->cancellation;
  newpair->comparison           = val->comparison;
  newpair->underflow            = val->underflow;
  newpair->latent_infinity_pos  = val->latent_infinity_pos;
  newpair->latent_infinity_neg  = val->latent_infinity_neg;
  newpair->latent_underflow     = val->latent_underflow;

  newpair->next = NULL;

  return newpair;
}

/*----------------------------------------------------------------------------*/
/* Comparison                                                                 */
/*----------------------------------------------------------------------------*/

int _FPC_ITEMS_EQUAL_(_FPC_ITEM_T_ *x, _FPC_ITEM_T_ *y)
{
  if ((x->file_name == y->file_name) && (x->line == y->line))
      return 1;
  return 0;
}

/*----------------------------------------------------------------------------*/
/* Insert a key-value pair into a hash table                                  */
/*----------------------------------------------------------------------------*/

void _FPC_HT_SET_(_FPC_HTABLE_T *hashtable, _FPC_ITEM_T_ *newVal)
{
  int bin = 0;
  _FPC_ITEM_T_ *newpair = NULL;
  _FPC_ITEM_T_ *next    = NULL;
  _FPC_ITEM_T_ *last    = NULL;

  bin = _FPC_HT_HASH_(hashtable, newVal);
  next = hashtable->table[bin];

  while(next != NULL && !_FPC_ITEMS_EQUAL_(newVal, next)) {
    last = next;
    next = next->next;
  }

  // There's already a pair
  if(next != NULL && _FPC_ITEMS_EQUAL_(newVal, next)) {
    // Increment values
    next->infinity_pos         += newVal->infinity_pos;
    next->infinity_neg         += newVal->infinity_neg;
    next->nan                  += newVal->nan;
    next->division_zero        += newVal->division_zero;
    next->cancellation         += newVal->cancellation;
    next->comparison           += newVal->comparison;
    next->underflow            += newVal->underflow;
    next->latent_infinity_pos  += newVal->latent_infinity_pos;
    next->latent_infinity_neg  += newVal->latent_infinity_neg;
    next->latent_underflow     += newVal->latent_underflow;

  } else  { // Nope, could't find it
    newpair = _FPC_HT_NEWPAIR_(newVal);
    (hashtable->n)++;

    if (next == hashtable->table[bin]) {
      // We're at the start of the linked list in this bin
      newpair->next = next;
      hashtable->table[bin] = newpair;
    } else if ( next == NULL ) {
      // We're at the end of the linked list in this bin
      last->next = newpair;
    } else {
      // We're in the middle of the list.
      newpair->next = next;
      last->next = newpair;
    }
  }
}

/*----------------------------------------------------------------------------*/
/* Print hash table                                                           */
/*----------------------------------------------------------------------------*/

void _FPC_PRINT_HASH_TABLE_(_FPC_HTABLE_T *hashtable)
{
  // Create directory
  //struct stat st = {0};
  struct stat st;
  char dir_name[] = ".fpc_logs";
  if (stat(dir_name, &st) == -1) { // dir doesn't exists
    //printf("#FPCHECKER: overwriting traces...\n");
    //char p[64];
    //p[0] = '\0';
    //sprintf(p, "rm -rf %s", dir_name);
    //system(p);
    mkdir(dir_name, 0775);
  }

  // Set filename
  //size_t len=256;
  // According to Linux manual:
  // Each element of the hostname must be from 1 to 63 characters long
  // and the entire hostname, including the dots, can be at most 253
  // characters long.
  char nodeName[256];
  nodeName[0] = '\0';
  if(gethostname(nodeName, 256) != 0)
    strcpy(nodeName, "node-unknown");

  // On Linux: The maximum length for a file name is 255 bytes. 
  // The maximum combined length of both the file name and path name is 4096 bytes.
  char fileName[5000];
  fileName[0] = '\0';
  strcpy(fileName, ".fpc_logs/fpc_");
  //strcpy(fileName, "fpc_");
  strcat(fileName, nodeName);

  // Maximum size for PID: we assume 2,000,000,000
  int pid = (int)getpid();
  char pidStr[11];
  pidStr[0] = '\0';
  sprintf(pidStr, "%d", pid);
  strcat(fileName, "_");
  strcat(fileName, pidStr);
  strcat(fileName, ".json");

  // Get program name and input
  int str_size = 0;
  for (int i=0; i < _FPC_PROG_INPUTS; ++i)
    str_size += strlen(_FPC_PROG_ARGS[i]) + 1;
  char *prog_input = (char *)malloc((sizeof(char) * str_size) + 1);
  prog_input[0] = '\0';
  for (int i=0; i < _FPC_PROG_INPUTS; ++i) {
    strcat(prog_input, _FPC_PROG_ARGS[i]);
    strcat(prog_input, " ");
  }

  // Prepare to print table
  uint64_t n = hashtable->n;
  uint64_t printed = 0;

  FILE *fp;
  fp = fopen(fileName, "w");

  fprintf(fp, "[\n");

  for (int i=0; (uint64_t)i < hashtable->size; ++i) {
    _FPC_ITEM_T_ *next;
    next = hashtable->table[i];

    while(next != NULL) {
      fprintf(fp, "  {\n");
      fprintf(fp, "\t\"input\": \"%s\",\n", prog_input);
      fprintf(fp, "\t\"file\": \"%s\",\n", next->file_name);
      fprintf(fp, "\t\"line\": %lu,\n", next->line);

      fprintf(fp, "\t\"infinity_pos\": %lu,\n", next->infinity_pos);
      fprintf(fp, "\t\"infinity_neg\": %lu,\n", next->infinity_neg);
      fprintf(fp, "\t\"nan\": %lu,\n", next->nan);
      fprintf(fp, "\t\"division_zero\": %lu,\n", next->division_zero);
      fprintf(fp, "\t\"cancellation\": %lu,\n", next->cancellation);
      fprintf(fp, "\t\"comparison\": %lu,\n", next->comparison);
      fprintf(fp, "\t\"underflow\": %lu,\n", next->underflow);
      fprintf(fp, "\t\"latent_infinity_pos\": %lu,\n", next->latent_infinity_pos);
      fprintf(fp, "\t\"latent_infinity_neg\": %lu,\n", next->latent_infinity_neg);
      fprintf(fp, "\t\"latent_underflow\": %lu\n", next->latent_underflow);

      next = next->next;
      printed++;

      if (printed == n)
        fprintf(fp, "  }\n");
      else
        fprintf(fp, "  },\n");
    }
  }

  fprintf(fp, "]\n");
  fclose(fp);
}

#endif /* SRC_FPC_HASHTABLE_H_ */
