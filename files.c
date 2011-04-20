/******************************************************************************
  Copyright (c) 1994,1995,1996 Weizmann Institute. All rights reserved.
  This is version 1.9 of the File Utilities Package (FUP), suitable for use
  with all versions of MOO1.8.0, up to beta2 inclusive.
  It has also been tested successfully with version MOO1.8.1.
  This file includes routines for file management and direct file access
  developed and written by Jaime Prilusky and Gustavo Glusman, Weizmann
  Institute of Science, Israel.
  This software is intended to work with the MOO server written by
  Pavel Curtis, Xerox PARC (Pavel@Xerox.Com), depending on routines
  and code from the MOO server itself.
  For questions, suggestions  and comments, please  send email to
  lsprilus@weizmann.weizmann.ac.il (Jaime Prilusky)
  Gustavo@bioinformatics.weizmann.ac.il (Gustavo Glusman)

  filemkdir and filermdir in collaboration with Jeremy Cooper <jeremy@crl.com>
  Thanks to Alex Stewart for valuable suggestions for improvements.

  fileinsert added by hErV� Collin <herve@hawaii.edu>
  and corrected by Karl Schaan <schaan@altern.org>

  filecut added by Karl Schaan <schaan@altern.org>
 *****************************************************************************/
#include <ctype.h>
#include "my-time.h"
#include "my-string.h"
#include "config.h"
#include "functions.h"
#include "log.h"
#include "random.h"
#include "storage.h"
#include "utils.h"
#include "dirent.h"
#include "unparse.h"
#include "list.h"
#include "regexpr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <errno.h>
#include "version.h"
#include <string.h>

/* #define INCLUDE_FILERUN */
/* #define INCLUDE_FILECHMOD */
#define EXTERN_FILES_DIR   "files/"
#define EXTERN_BIN_DIR     "bin/"
#define EXTERN_FILES_DIR_READ_ONLY
#define CREATE_NEW_DIR_MODE		0755
#define OneK 1024
#define BUF_LEN OneK * 2
#define MAX_INT 32760
#define TRUE    1
#define FALSE   0

extern int fputs();
extern struct passwd *getpwuid();
extern struct group  *getgrgid();
extern int system();
extern int access();
extern int atoi();

extern Var do_match(Var arglist, int reverse);

int
matches(char *subject, const char *pattern)
{
    Var ans, req;
    int  result;
    
    req = new_list(2);
    req.v.list[1].type = TYPE_STR;
    req.v.list[2].type = TYPE_STR;
    req.v.list[1].v.str  = str_dup(subject);
    req.v.list[2].v.str  = str_dup(pattern);
    ans = do_match(req, 0);
    result = is_true(ans);
    free_var(ans);
    free_var(req);
    return result;
}

void
remove_LAST_character(theStr)
char    *theStr;
{
 theStr[strlen(theStr)-1] = '\0';
}

void
remove_special_characters(theStr)
char	*theStr;
{
    register char *cp,*cp2;
    char buf[BUF_LEN];
    int currlen = 0;

    cp = theStr;
    cp2 = buf;
    while (( *cp ) && (currlen < BUF_LEN)) {
        switch (*cp) {
        case '&':
        case '|':
        case ';':
        case '<':
        case '>':
        case '(':
        case ')':
        case '\'':
        case '\\':
        case '"':
        case '`':
        case ':':
        case '$':
        case '!':
        case ' ':
            cp++;
            break;
        default: {
            *cp2++ = *cp++;
            currlen++;
            }
        }
    }
    *cp2 = '\0';
    strcpy( theStr, buf);
}

int
build_dir_name(thePathStr, theDirName, spec)
char *thePathStr;
char *theDirName;
char spec;
{
    char external_files  [BUF_LEN];
    char localthePathStr [BUF_LEN];
    struct stat st;

    if (strlen(thePathStr) > BUF_LEN)  return E_INVARG;

    strcpy(localthePathStr, thePathStr);
    remove_special_characters(localthePathStr);
    if (( strstr(localthePathStr,"/.")) ||
       (!strncmp(localthePathStr,".",1))) {
     return E_PERM;
    }
    strcpy(external_files,EXTERN_FILES_DIR);
    sprintf(theDirName,"%s%s", external_files, localthePathStr);
    
    if (stat(theDirName, &st) != 0) return E_INVARG;

    errno = 0;
    switch (spec)
	    {
	    case 'd':
	      if (!(st.st_mode & S_IFDIR)) return E_INVIND; 	      
	      break;
	    case 'r':
	      if ((access (theDirName, R_OK)) !=0) return E_PERM;
	      break;
	    case 'w':
	      if ((access (theDirName, W_OK)) !=0) return E_PERM;
	      break;
	    case 'x':
	      if ((access (theDirName, X_OK)) !=0) return E_PERM;
	      break;
	    default:
	        return E_ARGS;
	    }
     return E_NONE;
}

int
build_file_name(thePathStr, theNameStr, theFileName, spec)
char *thePathStr;
char *theNameStr;
char *theFileName;
char spec;
{
    char external_files  [BUF_LEN];
    char localthePathStr [BUF_LEN];
    char localtheNameStr [BUF_LEN];
    struct stat st;

#ifdef EXTERN_FILES_DIR_READ_ONLY
    if (strlen(thePathStr) == 0) {
       switch (spec)
       {
        case 'w':
        case 'd':
           return E_PERM;
           break;
       }
    }
#endif

    if ((strlen(thePathStr) > BUF_LEN) || 
        (strlen(theNameStr) > BUF_LEN))  return E_INVARG;

    strcpy(localthePathStr, thePathStr);
    strcpy(localtheNameStr, theNameStr);
    remove_special_characters(localthePathStr);
    remove_special_characters(localtheNameStr);
    
    if (( strstr(localthePathStr,"/.")) ||
       (!strncmp(localthePathStr,".",1)) ||
       (strstr(localtheNameStr,"/"))) {
     return E_PERM;
    }
    strcpy(external_files,EXTERN_FILES_DIR);
    sprintf(theFileName,"%s%s/%s", external_files, localthePathStr, 
                                   localtheNameStr);
    
    if (stat(theFileName, &st) != 0) return E_INVARG;

    errno = 0;
    switch (spec)
	    {
	    case 'd':
	      if (!(st.st_mode & S_IFDIR)) return E_INVIND; 	      
	      break;
	    case 'r':
	      if ((access (theFileName, R_OK)) !=0) return E_PERM;
	      break;
	    case 'w':
	      if ((access (theFileName, W_OK)) !=0) return E_PERM;
	      break;
	    case 'x':
	      if ((access (theFileName, X_OK)) !=0) return E_PERM;
	      break;
	    default:
	        return E_ARGS;
	    }
     return E_NONE;
}

static package
bf_fileexists(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename) */
        char infileName[BUF_LEN];
        Var ret;
        
        ret.type = TYPE_INT;
        ret.v.num = 1;
        if (build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,'r') != E_NONE) {
			ret.v.num = 0;        
		}
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_filelength(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename) */
        FILE *f ;
        char infileName[BUF_LEN];
        int num_lines = -1;
        char buffer[BUF_LEN];
        Var ret;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }
		f = fopen(infileName, "r");
        for (num_lines = 0; fgets(buffer, BUF_LEN, f); num_lines++);
        fclose(f);
        free_var(arglist);
        ret.type = TYPE_INT;
        ret.v.num = num_lines;
        return make_var_pack(ret);
}

static package
bf_filesize(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename) */
        char infileName[BUF_LEN];
        struct stat st;
        Var ret;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }
	    if (stat(infileName, &st) != 0) {
	        free_var(arglist);
	        return make_error_pack(E_INVARG);
	    }
        ret.type = TYPE_INT;
        ret.v.num = (long)st.st_size;
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_filewrite(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, list, [start, end]) */

        FILE *inFile = NULL;
        FILE *outFile = NULL;
        char infileName[BUF_LEN];
        char outfileName[BUF_LEN];
        int i, thelength;
        int index;
        int start_line = 1;
        int end_line   = MAX_INT;
        char buffer[BUF_LEN];
        Var ret, theline;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'w');
        if (result == E_PERM) {
                free_var(arglist);
                return make_error_pack(result);
        }

        ret.type = TYPE_INT;
        ret.v.num = 1;
        theline.type = TYPE_STR;

        sprintf(outfileName,"%s.%li", infileName,time(0));

        if (arglist.v.list[0].v.num > 3) 
           start_line = arglist.v.list[4].v.num;

       thelength = arglist.v.list[3].v.list[0].v.num;

        if (arglist.v.list[0].v.num > 4) {
              end_line = arglist.v.list[5].v.num;
           } else {
              end_line = arglist.v.list[4].v.num + thelength - 1;
           }

        if ((outFile = fopen(outfileName, "w")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
         }

        inFile = fopen(infileName, "r");
        index = 1;
        if (inFile) {
            while ((index < start_line) && (!feof(inFile))) {
               fgets(buffer, BUF_LEN, inFile);
               fputs(buffer,outFile);
               index++;
                }
            while ((index <= end_line) && (!feof(inFile))) {
               fgets(buffer, BUF_LEN, inFile);
               index++;
                }
         }

    for (i = 1; i <= thelength; i++) {
        switch (arglist.v.list[3].v.list[i].type) {
          case TYPE_INT:
            fprintf (outFile, "%d\n",  arglist.v.list[3].v.list[i].v.num);
            break;
          case TYPE_FLOAT:
            fprintf (outFile, "%g\n",  *(arglist.v.list[3].v.list[i].v.fnum));
            break;
          case TYPE_OBJ:
            fprintf (outFile, "#%d\n",  arglist.v.list[3].v.list[i].v.obj);
            break;
          case TYPE_STR:
            fprintf (outFile, "%s\n",  arglist.v.list[3].v.list[i].v.str);
            break;
          case TYPE_ERR:
              fprintf (outFile, "%s\n", unparse_error( arglist.v.list[3].v.list[i].v.err));
            break;
          case TYPE_LIST:
            fprintf (outFile,  "%s\n", "{list}");
            break;
          default:
            fprintf (outFile,  "%s\n", "*** unrecognized VAR TYPE (this should never happen) ***");
        }
      }

        if (inFile) {
            while (!feof(inFile)) {
               fgets(buffer, BUF_LEN, inFile);
               if (!feof(inFile)) {fputs(buffer,outFile);}
                }
        }

        if (outFile) {fclose(outFile);}
        if (inFile)  {fclose(inFile);} 
        rename(outfileName,infileName);
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_fileread(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename [start, end]) */

        FILE *f;
        char infileName[BUF_LEN];
        char buffer[BUF_LEN];
        Var ret, theline;
        int index;
        int start_line = 1;
        int end_line   = MAX_INT;
         int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

        if (arglist.v.list[0].v.num > 2)
                start_line = arglist.v.list[3].v.num;

        if (arglist.v.list[0].v.num > 3)
                end_line = arglist.v.list[4].v.num;
     
        if ((f = fopen(infileName, "r")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
        }

        ret.type = TYPE_LIST;
        ret = new_list(0);
        theline.type = TYPE_STR;

        index = 1;
        while ((index < start_line) && (!feof(f))) {
                fgets(buffer, BUF_LEN, f);
                index++;
                }

        while ((index <= end_line) && (!feof(f))) {
                fgets(buffer, BUF_LEN, f);
                if (!feof(f)) {
                   buffer[strlen(buffer)-1] = '\0';
                   theline.v.str = str_dup(buffer);
                   ret = listappend(ret, theline);
                   }
                index++;
                }

        fclose(f);
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_fileappend(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, list) */

        FILE *outFile = NULL;
        char outfileName[BUF_LEN];
        int i, thelength;
        Var ret, theline;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            outfileName,
                            'w');
        if (result == E_PERM) {
                free_var(arglist);
                return make_error_pack(result);
        }

        ret.type = TYPE_INT;
        ret.v.num = 1;
        theline.type = TYPE_STR;

        if ((outFile = fopen(outfileName, "a")) == 0) {
            free_var(arglist);
            return make_error_pack(E_INVARG);
        }

      thelength = arglist.v.list[3].v.list[0].v.num;

    for (i = 1; i <= thelength; i++) {
        switch (arglist.v.list[3].v.list[i].type) {
          case TYPE_INT:
            fprintf (outFile, "%d\n",  arglist.v.list[3].v.list[i].v.num);
            break;
          case TYPE_FLOAT:
            fprintf (outFile, "%g\n",  *(arglist.v.list[3].v.list[i].v.fnum));
            break;
          case TYPE_OBJ:
            fprintf (outFile, "#%d\n",  arglist.v.list[3].v.list[i].v.obj);
            break;
          case TYPE_STR:
            fprintf (outFile, "%s\n",  arglist.v.list[3].v.list[i].v.str);
            break;
          case TYPE_ERR:
              fprintf (outFile, "%s\n", unparse_error( arglist.v.list[3].v.list[i].v.err));
            break;
          case TYPE_LIST:
            fprintf (outFile,  "%s\n", "{list}");
            break;
          default:
            fprintf (outFile,  "%s\n", "*** unrecognized VAR TYPE (this should never happen) ***");
        }
      }

        if (outFile) {fclose(outFile);}
        free_var(arglist);
        return make_var_pack(ret);
}

/* new option for inserting lines */
static package
bf_fileinsert(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, list, start, end) */
 
        FILE *inFile = NULL;
        FILE *outFile = NULL;
        char infileName[BUF_LEN];
        char outfileName[BUF_LEN];
        int i, thelength;
        int index;
        int start_line = 1;
        int end_line   = MAX_INT;
        char buffer[BUF_LEN];
        Var ret, theline;
        int result;

        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'w');
        if (result == E_PERM) {
                free_var(arglist);
                return make_error_pack(result);
        }

        ret.type = TYPE_INT;
        ret.v.num = 1;
        theline.type = TYPE_STR;

        sprintf(outfileName,"%s.%li", infileName,time(0));

        if (arglist.v.list[0].v.num > 3) 
           start_line = arglist.v.list[4].v.num;

       thelength = arglist.v.list[3].v.list[0].v.num;

        if (arglist.v.list[0].v.num > 4) {
              end_line = arglist.v.list[5].v.num;
           } else {
              end_line = arglist.v.list[4].v.num + thelength - 1;
           }


        if ((outFile = fopen(outfileName, "w")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
         }

        inFile = fopen(infileName, "r");
        index = 1;
        if (inFile) {
            while ((index < start_line) && (!feof(inFile))) {
               fgets(buffer, BUF_LEN, inFile);
               fputs(buffer,outFile);
               index++;
                }
        }

    for (i = 1; i <= thelength; i++) {
        switch (arglist.v.list[3].v.list[i].type) {
          case TYPE_INT:
            fprintf (outFile, "%d\n",  arglist.v.list[3].v.list[i].v.num);
            break;
          case TYPE_FLOAT:
            fprintf (outFile, "%g\n",  *(arglist.v.list[3].v.list[i].v.fnum));
            break;
          case TYPE_OBJ:
            fprintf (outFile, "#%d\n",  arglist.v.list[3].v.list[i].v.obj);
            break;
          case TYPE_STR:
            fprintf (outFile, "%s\n",  arglist.v.list[3].v.list[i].v.str);
            break;
          case TYPE_ERR:
              fprintf (outFile, "%s\n", unparse_error( arglist.v.list[3].v.list[i].v.err));
            break;
          case TYPE_LIST:
            fprintf (outFile,  "%s\n", "{list}");
            break;
          default:
            fprintf (outFile,  "%s\n", "*** unrecognized VAR TYPE (this should never happen) ***");
        }
      }

        if (inFile) {
            while (!feof(inFile)) {
               fgets(buffer, BUF_LEN, inFile);
               if (!feof(inFile)) {fputs(buffer,outFile);}
                }
        }

        if (outFile) {fclose(outFile);}
        if (inFile)  {fclose(inFile);} 
        rename(outfileName,infileName);
        free_var(arglist);
        return make_var_pack(ret);
}


static package
bf_filecut(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, [start, end]) */

        FILE *inFile = NULL;
        FILE *outFile = NULL;
        char infileName[BUF_LEN];
        char outfileName[BUF_LEN];
        int i, thelength;
        int index;
        int start_line = 1;
        int end_line   = MAX_INT;
        char buffer[BUF_LEN];
        Var ret, theline;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'w');
        if (result == E_PERM) {
                free_var(arglist);
                return make_error_pack(result);
        }

        ret.type = TYPE_INT;
        ret.v.num = 1;
        theline.type = TYPE_STR;

        sprintf(outfileName,"%s.%li", infileName,time(0));

        if (arglist.v.list[0].v.num > 2)
           start_line = arglist.v.list[3].v.num;

        if (arglist.v.list[0].v.num > 3) {
              end_line = arglist.v.list[4].v.num;
           } else {
              end_line = arglist.v.list[3].v.num + thelength - 1;
           }

        if ((outFile = fopen(outfileName, "w")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
         }

        inFile = fopen(infileName, "r");
        index = 1;
        if (inFile) {
            while ((index < start_line) && (!feof(inFile))) {
               fgets(buffer, BUF_LEN, inFile);
               fputs(buffer,outFile);
               index++;
                }
            while ((index <= end_line) && (!feof(inFile))) {
               fgets(buffer, BUF_LEN, inFile);
               index++;
                }
         }

        if (inFile) {
            while (!feof(inFile)) {
               fgets(buffer, BUF_LEN, inFile);
               if (!feof(inFile)) {fputs(buffer,outFile);}
                }
        }

        if (outFile) {fclose(outFile);}
        if (inFile)  {fclose(inFile);}
        rename(outfileName,infileName);
        free_var(arglist);
        return make_var_pack(ret);
}


static package
bf_filedelete(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename) */
        char infileName[BUF_LEN];
        Var ret;
         int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'w');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

    if ((remove(infileName)) != 0) {
        free_var(arglist);
        return make_error_pack(E_INVARG);
    }

        free_var(arglist);
        ret.type = TYPE_INT;
        ret.v.num = 1;
        return make_var_pack(ret);
}


static package
bf_filelist(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory) */

        typedef struct dirent MYDIRENT ;
        DIR *dirp;
        DIR *subdir;
        MYDIRENT *dp;
        char rootDir [BUF_LEN];
        char dirName [BUF_LEN];
        Var ret, listOfDirs, listOfFiles, theline;
        int srchlen = 0;
        int result;
        result = build_dir_name(arglist.v.list[1].v.str,
                            rootDir,
                            'd');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

        if (!(dirp = opendir (rootDir))) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
        }

       if (arglist.v.list[0].v.num > 1) {
           srchlen = strlen(arglist.v.list[2].v.str);
        }
        ret.type = TYPE_LIST;
        ret = new_list(0);
        listOfDirs.type = TYPE_LIST;
        listOfDirs = new_list(0);
        listOfFiles.type = TYPE_LIST;
        listOfFiles = new_list(0);
        theline.type = TYPE_STR;

        while ((dp = readdir (dirp)) != 0) {
           if (strncmp(dp->d_name,".",1)) {
               sprintf(dirName,"%s/%s", rootDir,dp->d_name);
               if ((subdir = opendir(dirName))) {
                    closedir(subdir);
                    theline.v.str = str_dup(dp->d_name);
                    listOfDirs = listappend(listOfDirs, theline);
                }
                else {
                  if ((srchlen == 0) || 
                      (matches(dp->d_name,arglist.v.list[2].v.str))) {
                      theline.v.str = str_dup(dp->d_name);
                      listOfFiles = listappend(listOfFiles, theline);
                  } 
                }
            }
        }
        closedir (dirp);
        ret = listappend(ret, listOfFiles);
        ret = listappend(ret, listOfDirs);
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_filegrep(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, pattern, [option]) */

        FILE *f;
        char infileName[BUF_LEN];
        char buffer[BUF_LEN];
        int line_num = 0;
        Var ret, theline, anum, slist, nlist;
        int strings = TRUE;
        int numbers = FALSE;
        int showfound = TRUE;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }
        
        if(arglist.v.list[0].v.num == 4) {
          if(strstr(arglist.v.list[4].v.str,"n")) {
             numbers = TRUE;
             if(!(strstr(arglist.v.list[4].v.str,"s"))) {strings = FALSE;}
          }
          if(strstr(arglist.v.list[4].v.str,"v")) {showfound = FALSE;}
        }

        if ((f = fopen(infileName, "r")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
        }

        slist.type = TYPE_LIST;
        slist = new_list(0);
        nlist.type = TYPE_LIST;
        nlist = new_list(0);
        ret.type = TYPE_LIST;
        ret = new_list(0);

        theline.type = TYPE_STR;
        anum.type = TYPE_INT;
        while (!feof(f)) {
            fgets(buffer, BUF_LEN, f);
            line_num++;
            
            if (matches(buffer,arglist.v.list[3].v.str) == showfound) {
                    if ((strings == TRUE) && (!feof(f))) {
                      buffer[strlen(buffer)-1] = '\0';
                      theline.v.str = str_dup(buffer);
                      slist = listappend(slist, theline);
                    }
                    if ((numbers == TRUE) && (!feof(f))) {
                      anum.v.num = line_num;
                      nlist = listappend(nlist, anum);
                    }
                 }
              }

        fclose(f);
        free_var(arglist);
        ret = listappend(ret, slist);
        ret = listappend(ret, nlist);
        return make_var_pack(ret);
}

static package
bf_fileextract(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, start_pattern, end_pattern [,extra_pattern]) */

        FILE *f;
        char infileName[BUF_LEN];
        char buffer[BUF_LEN];
        Var ret, theline;
        Var startList, endList;
        Var startLine, endLine;
        int numOfLine = 0;
        int status = 1;
        int requiredPattern = (arglist.v.list[0].v.num > 4);
        int result;
        
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            infileName,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

        if ((strlen(arglist.v.list[2].v.str) == 0) ||
            (strlen(arglist.v.list[3].v.str) == 0) ||
            (strlen(arglist.v.list[4].v.str) == 0) ||
            (strlen(arglist.v.list[arglist.v.list[0].v.num].v.str) == 0)) {
          free_var(arglist);
          return make_error_pack(E_INVARG); 
        }

         if ((f = fopen(infileName, "r")) == 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
        }

       ret.type = TYPE_LIST;
        ret = new_list(0);
        theline.type = TYPE_STR;

        startList = new_list(0);
        startList.type = TYPE_LIST;

        endList = new_list(0);
        endList.type = TYPE_LIST;
        
        startLine.type = TYPE_INT;
        endLine.type = TYPE_INT;
 
        while (!feof(f)) {
            fgets(buffer, BUF_LEN, f);
            numOfLine++;
            
            if (status == 1) {
               if (matches(buffer,arglist.v.list[3].v.str)) {
	                startLine.v.num = numOfLine;
                        if (requiredPattern == TRUE) {
	            	    status = 2;}
                        else {
                            status = 3;}
	            }
            }
            
            if (status == 2) {
               if (matches(buffer,arglist.v.list[arglist.v.list[0].v.num].v.str)) {
	            	status = 3;
	            }
            }
            
            if ((status == 2) || (status == 3)) {
              if (matches(buffer,arglist.v.list[4].v.str)) {
                        if (status == 3) {
                          startList = listappend(startList, startLine);
                          endLine.v.num = numOfLine;
                          endList = listappend(endList, endLine);
                        }
                        status = 1;
                     }
            }
        }

        ret = listappend(ret,startList);
        ret = listappend(ret,endList);        
        fclose(f);
        free_var(arglist);
        return make_var_pack(ret);
}

static package
bf_fileversion(Var arglist, Byte next, void *vdata, Objid progr)
{
    Var ret;
    ret.type = TYPE_STR;
    ret.v.str = str_dup(FUP_version);
    free_var(arglist);
    return make_var_pack(ret);
}

static package
bf_filerename(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, oldFilename, newFilename) */
        char oldFilename[BUF_LEN];
        char newFilename[BUF_LEN];
        Var ret;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            oldFilename,
                            'w');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[3].v.str,
                            newFilename,
                            'w');
        if ((result != E_NONE) && (result != E_INVARG)){
                free_var(arglist);
                return make_error_pack(result);
        }

    if ((rename(oldFilename,newFilename)) != 0) {
        return make_error_pack(E_INVARG);
        }
        ret.type = TYPE_INT;
        ret.v.num = 1;
        free_var(arglist);
        return make_var_pack(ret);
}

#ifdef INCLUDE_FILECHMOD

static package
bf_filechmod(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename, filemode) */
        char theRequestedAction[BUF_LEN];
        char filename[BUF_LEN];
        char external_files  [BUF_LEN];
        Var ret;
        struct stat st;
        mode_t  mode;
        char filemode[BUF_LEN];
        int r1, r2; 
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            filename,
                            'w');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

        remove_special_characters(arglist.v.list[3].v.str);
        if (strlen(arglist.v.list[3].v.str) == 0) {
                   free_var(arglist);
                   return make_error_pack(E_INVARG); }

        strcpy(external_files,EXTERN_FILES_DIR);
        sprintf(theRequestedAction,"chmod %s %s%s/%s\n",
                                   arglist.v.list[3].v.str,
                                   external_files,
                                   arglist.v.list[1].v.str,
                                   arglist.v.list[2].v.str);

    if ((system(theRequestedAction)) == 0) {
          return make_error_pack(E_INVARG);
        }

	stat(filename, &st);
        ret.type = TYPE_STR;
        mode = st.st_mode;
        if (S_ISREG(st.st_mode))  mode = st.st_mode - 32768;
        if (S_ISDIR(st.st_mode))  mode = st.st_mode - 16384;
        if (S_ISCHR(st.st_mode))  mode = st.st_mode -  8192;
        if (S_ISBLK(st.st_mode))  mode = st.st_mode - 24576;
        if (S_ISSOCK(st.st_mode)) mode = st.st_mode - 49152;
        if (mode != st.st_mode) {
           r1 = mode / 8;
           r1 = r1 - ((r1 / 8) * 8);
           r2 = mode - ((mode/8) * 8);
           sprintf(filemode,"%ld%d%d",(long)mode/64,r1,r2);
           ret.v.str = str_dup(filemode);
           }
        else ret.v.str = str_dup("????");
        free_var(arglist);
        return make_var_pack(ret);
}

#endif

static package
bf_fileinfo(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (directory, filename) */
        char filename[BUF_LEN];
        Var ret, atime, mtime, ctime, fsize, ftype, fmode, fuid, fgid;
        struct stat st;
        struct passwd *pw;
        struct group *grp;
        mode_t  mode;
        int r0, r1, r2; 
        char filemode[BUF_LEN];
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            filename,
                            'r');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

        if (stat(filename, &st) != 0) {
           free_var(arglist);
           return make_error_pack(E_INVARG);
        }

        fsize.type = TYPE_INT;
        fsize.v.num = (long)st.st_size;

        ftype.type = TYPE_STR;
        ftype.v.str = str_dup("???");
        mode = st.st_mode;
        if (S_ISREG(st.st_mode)) {
           ftype.v.str = str_dup("reg");
           mode = st.st_mode - 32768;
           }
        if (S_ISDIR(st.st_mode)) {
           ftype.v.str = str_dup("dir");
           mode = st.st_mode - 16384;
           }

        if (S_ISFIFO(st.st_mode)) { oklog("FIFO %ld\n",(long)st.st_mode);
           }

        if (S_ISCHR(st.st_mode)) {
           ftype.v.str = str_dup("chr");
           mode = st.st_mode - 8192;
           }

        if (S_ISBLK(st.st_mode)) {
           ftype.v.str = str_dup("blk");
           mode = st.st_mode - 24576;
           }

        if (S_ISLNK(st.st_mode)) { oklog("LNK  %ld\n",(long)st.st_mode);
           }

        if (S_ISSOCK(st.st_mode)) {
           ftype.v.str = str_dup("sck");
           mode = st.st_mode - 49152;
           }

        fmode.type = TYPE_STR;
        fmode.v.str = str_dup("????");
        if (mode != st.st_mode) {
           r0 = mode / 512;
           r1 = mode / 8;
           r1 = r1 - ((r1 / 8) * 8);
           r2 = mode - ((mode/8) * 8);
           sprintf(filemode,"%ld%d%d",(long)mode/64,r1,r2);
           fmode.v.str = str_dup(filemode);
           }

        pw = getpwuid((short)st.st_uid);
        fuid.type = TYPE_STR;
        fuid.v.str = str_dup(pw->pw_name);

        grp = getgrgid((short)st.st_gid);
        fgid.type = TYPE_STR;
        fgid.v.str = str_dup(grp->gr_name);

        atime.type = TYPE_INT;
        atime.v.num = st.st_atime;

        mtime.type = TYPE_INT;
        mtime.v.num = st.st_mtime;

        ctime.type = TYPE_INT;
        ctime.v.num = st.st_ctime;
        
        ret.type = TYPE_LIST;
        ret = new_list(0);
        ret = listappend(ret, fsize); /* total size of file, bytes */
        ret = listappend(ret, ftype); /* file type */
        ret = listappend(ret, fmode); /* file mode */
        ret = listappend(ret, fuid);  /* user ID of owner */
        ret = listappend(ret, fgid);  /* group ID of owner */
        ret = listappend(ret, atime); /* file last access time */
        ret = listappend(ret, mtime); /* file last modify time */
        ret = listappend(ret, ctime); /* file last change time */

        free_var(arglist);
        return make_var_pack(ret);
}

#ifdef INCLUDE_FILERUN

static package
bf_filerun(Var arglist, Byte next, void *vdata, Objid progr)
{ /* (filename, arguments) */
        char theRequestedAction[BUF_LEN];
        Var ret, theArgs, theline;
        int i, numOfArgs, result;
        char fileName[BUF_LEN];
        char external_bin [BUF_LEN];

        theline.type = TYPE_STR;
        theArgs.type = TYPE_LIST;
        theArgs = new_list(0);
        numOfArgs = arglist.v.list[0].v.num;
        for (i = 1; i <= numOfArgs; i++) {
	        switch (arglist.v.list[i].type) {
	          case TYPE_STR:
                theline.v.str = str_dup(arglist.v.list[i].v.str);
                remove_special_characters( theline.v.str); 

                if (( strstr(theline.v.str,"/.")) ||
                   (!strncmp(theline.v.str,".",1)) ||
                   (!strncmp(theline.v.str,"/",1))) {
                    free_var(arglist);
                    free_var(theline);
                    return make_error_pack(E_PERM);
                    }

                theArgs = listappend(theArgs,theline);
	        break;
	          case TYPE_LIST:
                    if (arglist.v.list[i].v.list[0].v.num < 2) {
                        theline.v.str = str_dup("");
                        theArgs = listappend(theArgs,theline);
                    } else {
                    if ((arglist.v.list[i].v.list[1].type != TYPE_STR) ||
                       (arglist.v.list[i].v.list[2].type != TYPE_STR)) {
			            free_var(arglist);
                                    free_var(theline);
			            return make_error_pack(E_TYPE); 
                       }
                    result = build_file_name(arglist.v.list[i].v.list[1].v.str,
                                             arglist.v.list[i].v.list[2].v.str,
                                             fileName,
                                             'r');
                        theline.v.str = str_dup(fileName);
                        theArgs = listappend(theArgs,theline);
					}
	            break;
	          default:
                free_var(arglist);
                free_var(theline);
                return make_error_pack(E_INVARG);
	        }        
        }
       
       numOfArgs = theArgs.v.list[0].v.num;
       strcpy(external_bin,EXTERN_BIN_DIR);
       sprintf(theRequestedAction,"%s%s ",external_bin,theArgs.v.list[1].v.str);

        if ((numOfArgs > 1) && (strlen(theArgs.v.list[2].v.str)!=0)) {
                 sprintf(theRequestedAction,"cat %s | %s%s",
                                   theArgs.v.list[2].v.str,
                                   external_bin,
                                   theArgs.v.list[1].v.str);
        } else {
                 sprintf(theRequestedAction,"%s%s ",
                                   external_bin,
                                   theArgs.v.list[1].v.str);
        }

        for (i = 4; i <= numOfArgs; i++) {
        sprintf(theRequestedAction,"%s %s",
                                   theRequestedAction,
                                   theArgs.v.list[i].v.str);
        }

        if ((numOfArgs > 2) && (strlen(theArgs.v.list[3].v.str))){
            sprintf(theRequestedAction,"%s > %s",
                                   theRequestedAction,
                                   theArgs.v.list[3].v.str);
        }

        sprintf(theRequestedAction,"%s 2>&1", theRequestedAction);
        system(theRequestedAction);

        ret.type = TYPE_INT;
        ret.v.num = 1; /* always !! */
        free_var(arglist);
        free_var(theline);
        return make_var_pack(ret);
}
#endif

static package
bf_filemkdir(Var arglist, Byte next, void *vdata, Objid progr)
{  /* filemkdir(base-directory-name, new-directory-name) */
      char newdirName[BUF_LEN];
      mode_t create_mode = CREATE_NEW_DIR_MODE;
      Var ret;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            newdirName,
                            'd');
        if ((result != E_NONE) && (result != E_INVARG)){
                free_var(arglist);
                return make_error_pack(result);
        }

    if ((mkdir(newdirName, create_mode)) != 0) {
            free_var(arglist);
            return make_error_pack(E_PERM); }

    ret.type = TYPE_INT;
    ret.v.num = 1;
    free_var(arglist);
    return make_var_pack(ret);
}

static package
bf_filermdir(Var arglist, Byte next, void *vdata, Objid prog)
{  /* filermdir(base-directory-name, directory-name) */
        char rmDirName[BUF_LEN];
        Var  ret;
        int result;
        result = build_file_name(arglist.v.list[1].v.str,
                            arglist.v.list[2].v.str,
                            rmDirName,
                            'd');
        if (result != E_NONE) {
                free_var(arglist);
                return make_error_pack(result);
        }

    if ((rmdir(rmDirName)) != 0) {
        free_var(arglist);
        return make_error_pack(E_PERM); 
    }
    free_var(arglist);
    ret.type = TYPE_INT;
    ret.v.num = 1;
    return make_var_pack(ret);
}

static package
bf_fileerror(Var arglist, Byte next, void *vdata, Objid progr)
{
    Var ret;
    ret.type = TYPE_STR;
    ret.v.str = str_dup(strerror(errno));
    free_var(arglist);
    return make_var_pack(ret);
}

void
register_files(void)
{ 
    (void) register_function("fileappend",  3,  3, bf_fileappend,  TYPE_STR, TYPE_STR, TYPE_LIST);

#ifdef INCLUDE_FILECHMOD
    (void) register_function("filechmod",   3,  3, bf_filechmod,   TYPE_STR, TYPE_STR, TYPE_STR);
#endif

    (void) register_function("filedelete",  2,  2, bf_filedelete,  TYPE_STR, TYPE_STR);
    (void) register_function("fileerror",   0,  0, bf_fileerror);
    (void) register_function("fileexists",  2,  2, bf_fileexists,  TYPE_STR, TYPE_STR);
    (void) register_function("fileextract", 4,  5, bf_fileextract, TYPE_STR, TYPE_STR, TYPE_STR, TYPE_STR, TYPE_STR);
    (void) register_function("filegrep",    3,  4, bf_filegrep,    TYPE_STR, TYPE_STR, TYPE_STR, TYPE_STR);
    (void) register_function("fileinfo",    2,  2, bf_fileinfo,    TYPE_STR, TYPE_STR);
    (void) register_function("filelength",  2,  2, bf_filelength,  TYPE_STR, TYPE_STR);
    (void) register_function("filelist",    1,  2, bf_filelist,    TYPE_STR, TYPE_STR);
    (void) register_function("filemkdir",   2,  2, bf_filemkdir,   TYPE_STR, TYPE_STR);
    (void) register_function("fileread",    2,  4, bf_fileread,    TYPE_STR, TYPE_STR, TYPE_INT, TYPE_INT);
    (void) register_function("filerename",  3,  3, bf_filerename,  TYPE_STR, TYPE_STR, TYPE_STR);
    (void) register_function("filermdir",   2,  2, bf_filermdir,   TYPE_STR, TYPE_STR);

#ifdef INCLUDE_FILERUN
    (void) register_function("filerun",     1, -1, bf_filerun,     TYPE_STR, TYPE_LIST, TYPE_LIST); 
#endif

    (void) register_function("filesize",    2,  2, bf_filesize,    TYPE_STR, TYPE_STR);
    (void) register_function("fileversion", 0,  0, bf_fileversion);
    (void) register_function("filewrite",   3,  5, bf_filewrite,   TYPE_STR, TYPE_STR, TYPE_LIST, TYPE_INT, TYPE_INT);
    (void) register_function("fileinsert",  3,  5, bf_fileinsert,  TYPE_STR, TYPE_STR, TYPE_LIST, TYPE_INT, TYPE_INT);
    (void) register_function("filecut",     2,  4, bf_filecut,     TYPE_STR, TYPE_STR, TYPE_INT,  TYPE_INT);
}
