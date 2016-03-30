/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "common.hxx"

#include <stdio.h>
#include <cassert>

void avtk_debug( int warnLevel, const char* name, const char* file, const char* func, int line,
                 const char* format, ... )
{
	if ( warnLevel == DEBUG_LEVEL_ERROR ) {
		printf( "[\033[1;31m%s\033[0m] %s:%i: ", name, func, line );
	} else if ( warnLevel == DEBUG_LEVEL_WARN ) {
		printf( "[\033[1;33m%s\033[0m] %s:%i: ", name, func, line );
	} else if ( warnLevel == DEBUG_LEVEL_DEVELOPER ) {
		printf( "[\033[1;34m%s\033[0m] %s:%i: ", name, func, line );
	} else {
		printf( "[\033[1;32m%s\033[0m] %s:%i: ", name, func, line );
	}
	printf( "\033[0m" );

	if ( format ) {
		va_list args;
		va_start( args, format );
		vfprintf( stdout, format, args );
		va_end( args );
	}
	//printf( "\n" );
}

