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

#ifndef OPENAV_AVTK_COMMON_HXX
#define OPENAV_AVTK_COMMON_HXX

#include <stdarg.h>

#ifndef OPENAV_NAME
#define NAME "Avtk"
#endif

enum DEBUG_LEVEL {
	DEBUG_LEVEL_DEVELOPER = 0, // on #ifdef AVTK_DEBUG at compile time only
	DEBUG_LEVEL_NOTE,
	DEBUG_LEVEL_WARN,
	DEBUG_LEVEL_ERROR,
};

void avtk_debug( int warnLevel, const char* name, const char* file, const char* func, int line,
                 const char* format = 0, ... );

#ifdef AVTK_DEBUG
#define AVTK_DEV( format, args... ) avtk_debug( DEBUG_LEVEL_DEVELOPER, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )
#else
#define AVTK_DEV( format, args... )
#endif

#define AVTK_NOTE( format, args... ) avtk_debug( DEBUG_LEVEL_NOTE, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )
#define AVTK_WARN( format, args... ) avtk_debug( DEBUG_LEVEL_WARN, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )
#define AVTK_ERROR( format, args... ) avtk_debug( DEBUG_LEVEL_ERROR, NAME, __FILE__, __FUNCTION__, __LINE__, format, ## args )

#endif // OPENAV_AVTK_COMMON_HXX

