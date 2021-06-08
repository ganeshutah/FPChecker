#define _GNU_SOURCE
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef ssize_t (*execve_func_t)(const char* filename, char* const argv[], char* const envp[]);

static execve_func_t old_execve = NULL;
static const char *nvcc_fpc = "/usr/workspace/wsa/laguna/fpchecker/FPChecker/bin/nvcc-fpc";

/** Return: one if the string t occurs at the end of the string s, and zero otherwise **/
int str_end(const char *s, const char *t)
{
    if (strlen(s) < strlen(t)) return 0;
    return 0 == strcmp(&s[strlen(s)-strlen(t)], t);
}

int isNVCC(const char* filename) {
  return str_end(filename, "/nvcc");
}

void printEnvironment(char* const envp[]) {
  size_t elems = 0;
  while (envp != NULL) {
    if (*envp == NULL)
      break;

    elems++;
    printf("VAR: %s\n", *envp);
    envp++;
  }
  printf("Elems: %lu\n", elems);
}

/** Copy the environment **/
void copy_env_variables(char* const envp[], char *** new_envp) {
  char **ptr = (char **)envp;
  size_t elems = 0;
  while (ptr != NULL) {
    if (*ptr == NULL)
      break;
    elems++;
    ptr++;
  }

  *new_envp = (char **)malloc(sizeof(char *)*elems+1); 
  for (size_t i=0; i < elems; ++i) {
    (*new_envp)[i] = (char *)malloc(strlen(envp[i]) * sizeof(char) + 1);
    if (strstr (envp[i], "LD_PRELOAD=") == NULL) { // do not copy ld_preload
      strcpy((*new_envp)[i], envp[i]);
    } else {
      strcpy((*new_envp)[i], "LD_PRELOAD=");
    }
  }
  (*new_envp)[elems] = NULL;
}

/** Wrapper for execve **/
int execve(const char* filename, char* const argv[], char* const envp[]) {
    // Copy env variables
    char ** new_envp;
    copy_env_variables(envp, &new_envp);
    //printEnvironment(envp);

    old_execve = dlsym(RTLD_NEXT, "execve");

    if (isNVCC(filename)) {
      //printf("Intercepting NVCC...\n");
      return old_execve(nvcc_fpc, argv, new_envp);
    }

    return old_execve(filename, argv, envp);
}
