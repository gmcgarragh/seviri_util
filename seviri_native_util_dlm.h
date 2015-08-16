#ifndef SEVIRI_NATIVE_UTIL_DLM_H
#define SEVIRI_NATIVE_UTIL_DLM_H

/* Message Numbers */
#define SEVIRI_PREPROC_ERROR		0
#define SEVIRI_PREPROC_NOSTRINGARRAY	-1

/* Useful macro */
#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

extern IDL_MSG_BLOCK msg_block;

/* Define the startup function that adds C functions to IDL along with the exit
   handler */

extern int  seviri_native_util_startup(void);
extern void seviri_native_util_exit_handler(void);

/* Define the wrapper function which calls seviri_native_util */
extern void IDL_CDECL seviri_preproc_dlm(int argc, IDL_VPTR argv[], char *argk);

#endif
