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

#ifndef OPENAV_AVTK_UTILS_HXX
#define OPENAV_AVTK_UTILS_HXX

/** Utils
 * Utils provides a variety of utility functions that are cross-platform.
 * They're based on small libraries like tinydir and zix, and aim to serve
 * creating fully features UI's in a lightweight and cross platform way.
**/

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>

#include "tinydir.hxx"

namespace Avtk
{
int fileUpLevel( std::string path, std::string& newPath );

int loadSample( std::string path, std::vector< float >& sample, bool printErrors = true );
int directories( std::string d, std::vector< std::string >& files, bool nameOnly = true, bool printErrors = true );

/** lists the contents of a directory @param directory, and stores the
 * resuliting filenames in the provided @param output vector. The options are
 * provided to allow for better presentation of the contents:<br/>
 * - @param nameOnly (provides only file *name* without path).<br/>
 * - @param smartShortStrings (remove starting N characters if common in all files).<br/>
 * - @param printErrors (prints errors if file doesn't exist etc).
 */
int directoryContents(  std::string directory,
                        std::vector< std::string >& output,
                        std::string& strippedFilenameStart,
                        bool nameOnly = true,
                        bool smartShortStrings = true,
                        bool printErrors = true );
};

#endif // OPENAV_AVTK_UTILS_HXX

