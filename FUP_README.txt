 *****************************************************************************
  Copyright (c)1994,1995,1996, The Weizmann Institute of Science.
  See the enclosed copyright notice.
  This file documents the File Utilities Package developed and written by
  Jaime Prilusky and Gustavo Glusman, Weizmann Institute of Science, Israel.
  For questions, suggestions and comments, please  send email to
  lsprilus@weizmann.weizmann.ac.il (Jaime Prilusky)
  Gustavo@bioinformatics.weizmann.ac.il (Gustavo Glusman)
 *****************************************************************************


File Utilities Package 1.9 - August '02


INTRODUCTION:

The "File Utilities Package" is a set of routines for direct file
management and access from inside the MOO,  developed and written by
Jaime Prilusky and Gustavo Glusman, Weizmann Institute of Science,
Israel.

This software is intended to work with the MOO server written by Pavel
Curtis, Xerox PARC (Pavel@Xerox.Com), depending on routines and code
from the MOO server itself. It has been tested with MOO version 1.8.0b2.


SECURITY:

All the built-in functions provided in the File Utilities Package used to 
require wizardly permissions, but since version 1.4 this is left to the 
database. Take a look to the file_handler file for an example of the 
implementation of permissions at the database level.

A simple $file_handler wrapper is provided. A more complex version,
including a disk-quota system, is available too.
Additionally, file read/write operations are allowed only over the directory
subtree rooted at the 'base directory', called 'files' by default, and
execute operations are allowed only from the directory called 'bin' by default.
To achieve this, all paths are stripped of spaces, and then rejected if:
  a) the first character is a '/' or a '.'  OR
  b) the path includes the substring '/.'.


DESCRIPTION:

1) List of primitives provided:
  Modify files:       filewrite, fileappend, filedelete, filerename,
                      *filechmod, filemkdir, filermdir, fileinsert,
                      filecut
  Gather information: fileversion, fileread, fileexists, filelength,
                      filesize, filelist, filegrep, fileextract, fileinfo,
                      fileerror
  Execute commands:   *filerun

 * denotes that the function is optional at compile time

2) Standard return errors:
  a) Any primitive called with incorrect number of arguments returns E_ARGS.
  b) Any primitive called with arguments of the wrong type returns E_INVARG.
  c) Any primitive called by a programmer that isn't a wizard returns E_PERM.
  d) Any attempt to access a file outside the hierarchy returns E_PERM.
  e) Any attempt to access a file that doesn't exist returns E_INVARG.
     (Except for fileexists, see below.)
  f) Any attempt to remove a directory that doesn't exist returns E_INVIND.

3) Description of the primitives:

To make the description easier, we'll assume we have the following files:

 files/notes
            /foo.text   <-- this is a text file
            /foox.test  <-- this is a text file
            /bar        <-- this is an empty subdirectory
 files/misc

We'll also assume that foo.text reads:
+--
|  Copyright (c) 1994 Weizmann Institute. All rights reserved.
|  This file documents the File Utilities Package developed and written by
|  Jaime Prilusky and Gustavo Glusman, Weizmann Institute of Science, Israel.
|  For questions, suggestions and comments, please  send email to
|  lsprilus@weizmann.weizmann.ac.il (Jaime Prilusky)
|  Gustavo@bioinformatics.weizmann.ac.il (Gustavo Glusman)
+--
and foox.test reads:
+--
| line 1
| line 2
| end
+--


str fileversion()
Returns a string representing the version of the currently installed FUP.
The format is x.y, where x is the major release number and y is the minor
release number.
Example:
  fileversion()  => "1.9"


num fileexists(str PATH, str NAME)
Returns 1 iff files/PATH/NAME exists, 0 otherwise.
Examples:
  fileexists("notes","foo.text")  => 1
  fileexists("misc","foox.test")  => 0


str fileerror()
Returns a string describing the UNIX error message, which reports the last
error encountered during a call to a system or library function.
Examples:
  fileerror()  => "Error 0"   (No error)
  fileerror()  => "Interrupted system call"
  fileerror()  => "No such file or directory"


list fileinfo(str PATH, str NAME)
Returns a list with assorted system information about the relevant file/directory.
Examples:
  fileinfo("notes","") 
   => {512, "dir", "755", "lsprilus", "staff", 788541296, 788346820, 788346820}
  fileinfo("notes","foo.text") 
   => {376, "reg", "644", "lsprilus", "staff", 788541674, 788541674, 788541674}
The information provided is: size, type, mode, owner, group, file last access time,
file last modify time, and file last change time. Check 'man stat' for more info.


str filechmod(str PATH, str NAME, str MODE)
Sets the mode of the relevant file/directory.
Examples:
  filechmod("notes","foo.text","bleh") 
   => "644"  (This just returns the existing value.)
  filechmod("notes","foo.text","640") 
   => "640"  (It returns the new value.)


list filelist(str PATH [, str NAME])
Returns the list of files and subdirectories in files/PATH, not recursively.
If NAME is provided, only files matching NAME as regexp will be returned.
All existing subdirectories will be returned in any case.
Examples:
  filelist("")                => {{},{"notes","misc"}}
  filelist("notes")           => {{"foo.text","foox.test"},{"bar"}}
  filelist("notes/bar")       => {{},{}}
  filelist("notes","oo.%.")   => {{"foox.test"},{"bar"}}
  filelist("misc")            => {{},{}}


num filelength(str PATH, str NAME)
Returns the number of lines of files/PATH/NAME.
Example:
  filelength("notes","foo.text")  => 6


num filesize(str PATH, str NAME)
Returns the number of characters of files/PATH/NAME.
Example:
  filesize("notes","foo.text")  => 388


num filedelete(str PATH, str NAME)
Irretrievably deletes files/PATH/NAME.
Example:
  filedelete("notes","foo.text")  => 1 if successful.


num filemkdir(str PATH, str NAME)
Creates a new directory: files/PATH/NAME.
Example:
  filemkdir("notes","mydir")  => 1 if successful.


num filermdir(str PATH, str NAME)
Removes the directory: files/PATH/NAME, if it's empty.
Example:
  filermdir("notes","mydir")  => 1 if successful.
  filermdir("notes","mydir")  => E_PERM if unsuccessful.
    Hint: use fileerror() to find the reason for failure.

num filerename(str PATH, str OLDNAME, str NEWNAME)
Renames files/PATH/OLDNAME to NEWNAME.
Example:
  filerename("notes","foo.text","blah.blah")  => 1 if successful.


list fileread(str PATH, str NAME [, num START [, num END]])
Returns a list of strings which represent lines read from files/PATH/NAME,
from START to END, which default to the beginning and the end of the file
respectively.
Examples:
  fileread("notes","foox.test")      => {"line 1","line 2","end"}
  fileread("notes","foox.test",2)    => {"line 2","end"}
  fileread("notes","foox.test",2,2)  => {"line 2"}
  fileread("notes","foox.test",3,2)  => {}
  fileread("notes","foox.test",5,6)  => {}


num fileappend(str PATH, str NAME, list TEXT)
Appends TEXT to files/PATH/NAME.
Creates the file if it didn't exist previously.
Examples:
  fileappend("notes","foox.test",{"hehe","hoho"})  => 1 if successful.


num filewrite(str PATH, str NAME, list TEXT [, num START [, num END]])
Writes TEXT on files/PATH/NAME.
Creates the file if it didn't exist previously.
Assuming LENGTH is the number of lines in TEXT:
-If neither START nor END are provided, the file is overwritten with TEXT.
-If START is provided, but END isn't, LENGTH lines of the file starting from
 START are overwritten with TEXT. This may extend the file length.
-If both START and END are provided, the lines from START to END are replaced
 with TEXT. This may extend the file length or reduce it.
If the operation succeeds, it returns 1.
Examples:
(the operations are not sequential.
 The file starts as: {"line 1","line 2","end"})
  Operation                                          File contents after
  filewrite("notes","foox.test",{"test"})           {"test"}
  filewrite("notes","foox.test",{"te","st"},2)      {"line 1","te","st"}
  filewrite("notes","foox.test",{"te","st"},2,2)    {"line 1","te","st","end"}
  filewrite("notes","foox.test",{"test"},2,3)       {"line 1","test"}
  filewrite("notes","foox.test",{},2,2)             {"line 1","end"}

num fileinsert(str PATH, str NAME, list TEXT, num START)
Insertes TEXT on files/PATH/NAME.
Creates the file if it didn't exist previously.
Assuming LENGTH is the number of lines in TEXT:
-If START is not provided, TEXT line are inserted on the first line.
-If START is provided, TEXT lines are inserted starting from START taking as many lines as LENGTH(TEXT) is, pushing the existing lines down the same amount.
If the operation succeeds, it returns 1.
Examples:
 The file starts as: {"line1","line2","end"})
  Operation                                                File contents after
  fileinsert("notes","foox.test", {"line0"})               {"line0","line1","line2","end"}
  fileinsert("notes","foox.test", {"line3"}, 3)            {"line1","line2","line3","end"}
   fileinsert("notes","foox.test", {"line3", "line4"}, 3)  {"line1","line2","line3","line4","end"} 

num filecut(str PATH, str NAME [, num START [, num END]])
Cuts lines on files/PATH/NAME.
Assuming LENGTH is the number of lines from START to END:
-If neither START nor END are provided, the file is overwritten.
-If START is provided, but END isn't, LENGTH lines of the file starting from
 START are cutted.
-If both START and END are provided, the lines from START to END are cutted.
If the operation succeeds, it returns 1.
Examples:
(the operations are not sequential.)
 The file starts as: {"line 1","line 2","end"})
  Operation                                          File contents after
  filecut("notes","foox.test")                          {}
  filecut("notes","foox.test",2)                        {"line 1"}
  filecut("notes","foox.test",2,2)                      {"line 1","end"}
  filecut("notes","foox.test",2,3)                      {"line 1"}

list filegrep(str PATH, str NAME, str REGEXP [, str SWITCHES])
Returns a list of strings and line numbers, which represent lines read from
files/PATH/NAME, and that match REGEXP.
SWITCHES defaults to "s".
-If SWITCHES includes "s", the matching lines will be returned.
-If SWITCHES includes "n", the numbers of the matching lines will be returned.
-If SWITCHES includes "v", the condition is reversed, and lines not matching
 will be returned.
Examples:
  filegrep("notes","foo.text","Weizmann") 
	=> {{"  Copyright (c) 1994 Weizmann Institute. All rights reserved.",
         "  Jaime Prilusky and Gustavo Glusman, Weizmann Institute of
          Science, Israel.", "  lsprilus@weizmann.weizmann.ac.il (Jaime
          Prilusky)", "  Gustavo@bioinformatics.weizmann.ac.il (Gustavo
          Glusman)"}, {}}
  filegrep("notes","foo.text","Weizmann","n") 
    => {{}, {1, 3, 5, 6}}  
  filegrep("notes","foo.text","Weizmann","ns") 
	=> {{"  Copyright (c) 1994 Weizmann Institute. All rights reserved.",
         "  Jaime Prilusky and Gustavo Glusman, Weizmann Institute of
          Science, Israel.", "  lsprilus@weizmann.weizmann.ac.il (Jaime
          Prilusky)", "  Gustavo@bioinformatics.weizmann.ac.il (Gustavo
          Glusman)"}, {1, 3, 5, 6}}
  filegrep("notes","foo.text","Weizmann","vn") 
    => {{}, {2, 4, 7, 8}} 
  

list fileextract(str PATH, str NAME, str REGEXP1, str REGEXP2 [, str REGEXP3])
Returns a list of starts and ends of sections of files/PATH/NAME, that fulfill
the following requirements:
-the first line of the section matches REGEXP1,
-the last line of the section matches REGEXP2, and
-at least one line of the section matches REGEXP3, if provided.
If a line matches REGEXP1 and REGEXP2 (and REGEXP3 if provided), it can
constitute a section by itself.
Examples:
  fileextract("notes","foo.text","a","x")          => {{}, {}}
    (there isn't a line with an "x")
  fileextract("notes","foo.text","Copy","email")   => {{1}, {4}}
    (the section from line 1 to line 4 fits)
  fileextract("notes","foo.text","a","b")          => {{1, 3}, {2, 6}}
    (the sections from line 1 to line 2 and from line 3 to line 6 fit)
  fileextract("notes","foo.text","a","b","est")    => {{3}, {6}}
    (of these, only the section from 3 to 6 has a line that matches "est")


num filerun(str EXECUTABLE [, list {str PATH1, str INFILE}
                           [, list {str PATH2, str OUTFILE}
                           [, str/list PARAMETER]*]])
Checks a whole set of security issues, including the requirement that
'EXECUTABLE' be found in the 'bin' directory. If all is ok, it issues a
system call equivalent to the Unix command:
  cat PATH1/INFILE | EXECUTABLE PARAMETER(s) > PATH2/OUTFILE
Examples:
  filerun("cal")
  filerun("grep",{},{"temp","output"},"wizard",{"notes","*"})
  filerun("lpr",{"notes","info"},{},"-Plaser")

*** WARNING: filerun() ISN'T fully secure at this stage, and it is provided
*** as an option only for managers of trusted systems.


CUSTOMIZATION:

You can change the name of the base directory for the external file
hierarchy, which is 'hard-coded' into the server upon compilation.
To change it from its default ('files'), edit the line:

#define EXTERN_FILES_DIR   "files/"
 
in the file files.c.
You can also make this base directory either read-only or read-and-write.
It is read-only by default. To make it writable from the MOO, comment the line:

#define EXTERN_FILES_DIR_READ_ONLY

in the file files.c.
The default mode for new directories can be configured too:

#define CREATE_NEW_DIR_MODE  0755

The default name for the directory that holds the executables is defined in:

#define EXTERN_BIN_DIR    "bin/"

also in files.c.

By default, the filechmod() and filerun() builtins are disabled. To enable any of them, uncomment the lines

/* #define INCLUDE_FILERUN */
/* #define INCLUDE_FILECHMOD */

in the file files.c, as needed.

The BUFFER LEN for arguments passed in functions such as fileread() etc..
is set to 1024 * 2 K by default.
If you are dealing with large database and end up with very long lines, it might
not be enough. To increase it, edit files.c
and change the line:

#define BUF_LEN OneK * 2

by replacing 2 by whatever number is needed.
Note: no numbers higher than 5 have ever been tested: use it at your own risks 

INSTALLATION:

If you're using the MOOFUP package, FUP is already installed. Skip steps 1-6.
(please note that only the 1.8 version is available then). 

1) Copy the file files.c to the source directory.

2) Add the following line to the block of external definitions in bf_register.h:

extern void register_files(void); 

3) Add the following line to the definition of bi_function_registries in
   the file functions.c:

    register_files,

4) Changes to Makefile.in:

  a) Add files.c to the CSRCS list.
  
  b) Add the lines: (suggestion - after the definition of execute.o)

files.o : files.c my-stdarg.h config.h functions.h my-stdio.h bf_register.h \
  execute.h opcode.h options.h program.h structures.h log.h server.h \
  network.h storage.h streams.h utils.h

5) Add the following line to the file server.c:

  oklog("          (Using File Utilities Package version %s)\n", FUP_version);
   
immediately before the lines:

  oklog("          (Task timeouts measured in %s seconds.)\n",
      virtual_timer_available() ? "server CPU" : "wall-clock");

6) Add the following line to the file version.c:

const char     *FUP_version = "1.9";

Suggestion - put it before the line:

const char     *server_version = "1.8.0b2";

7) Add the following line to the file version.h:

extern const char     *FUP_version;

Suggestion - put it before the line:

extern const char *server_version;

8) IF you wish to enable the filerun() builtin, edit the file files.c and
   uncomment the line #define INCLUDE_FILERUN.

9) IF you wish to enable the filechmod() builtin, edit the file files.c and
   uncomment the line #define INCLUDE_FILECHMOD.

10) Configure and make.

11) Create a subdirectory of the directory where the executable will
   reside, named 'files'. If you are using filerun(), create another one called
   'bin' (see CUSTOMIZATION above).

 *****************************************************************************
   DISCLAIMER: You can use this software, but you do it at your own risk.
   We provide no guarantee that this will compile successfully, or work
   as expected, in any system/environment, and take no responsibility for
   any loss of information or any other damage caused by using this software,
   or by stopping to use it.
 *****************************************************************************
