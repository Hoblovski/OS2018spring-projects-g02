/*
    Copyright 2016 Robert Elder Software Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License"); you may not 
    use this file except in compliance with the License.  You may obtain a copy 
    of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software 
    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the 
    License for the specific language governing permissions and limitations 
    under the License.
*/

#include <assert.h>
#include <stdarg.h>
#include "../builtin/c99libc_in_c89.h"

int putchar(int);

int printf(const char *, ...);

int printf(const char * fmt, ...){
	va_list arglist;
	int rtn = 0;
	va_start(arglist, fmt);
	rtn = vprintf(fmt, arglist);
	va_end( arglist );
	return rtn;
}

int vprintf(const char * fmt, va_list va){
	/*  Keep trying to print safely using the stack as temporary storage: */
	char try1[4];
	char *result1 = &try1[0];
	unsigned int chars_required = vsnprintf(&try1[0], 4, fmt, va);
	if(chars_required < 4){
		while(*result1){
			putchar(*result1);
			result1++;
		}
	}else{
		char try2[16];
		unsigned int chars_required = vsnprintf(&try2[0], 16, fmt, va);
		char * result2 = &try2[0];
		if(chars_required < 16){
			while(*result2){
				putchar(*result2);
				result2++;
			}
		}else{
			char try3[200];
			unsigned int chars_required = vsnprintf(&try3[0], 200, fmt, va);
			char * result3 = &try3[0];
			if(chars_required < 200){
				while(*result3){
					putchar(*result3);
					result3++;
				}
			}else{

				char try4[1000];
				unsigned int chars_required = vsnprintf(&try4[0], 1000, fmt, va);
				char * result4 = &try4[0];
				if(chars_required < 1000){
					while(*result4){
						putchar(*result4);
						result4++;
					}
				}else{

					assert(0 && "Print buffer too small.");
				}
			}
		}
	}
	return 0;
}

int vsprintf(char * buf, const char * fmt, va_list va){
	/*  TODO:  Set this number to max int in the future.  Technically, there is 
	    no bound on number of characters, which is why sprintf is unsafe.
	    The number passed in has nothing to do with the actual buffer size
	    (because we don't know the buffer size, that is why sprintf is terrible.)
	*/
	vsnprintf(buf, 1048576, fmt, va);
	return 0;
}

int snprintf(char * buf, size_t n, const char * fmt, ...){
	va_list arglist;
	int rtn = 0;
	va_start(arglist, fmt);
	rtn = vsnprintf(buf, n, fmt, arglist);
	va_end( arglist );
	return rtn;
}

int vsnprintf(char * buf, size_t n, const char * fmt, va_list va){
	return c89_vsnprintf(buf, n, fmt, va);
}

int sprintf(char * buf, const char * fmt, ...){
	va_list arglist;
	int rtn;
	va_start(arglist, fmt);
	rtn = vsprintf(buf, fmt, arglist);
	va_end(arglist);
	return rtn;
}
