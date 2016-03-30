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

#include "utils.hxx"

#include "avtk.hxx"
#include "theme.hxx"
#include <algorithm>
#include <cstring>
#include <sndfile.h>

namespace Avtk
{
int loadSample( std::string path, std::vector< float >& sample, bool printErrors )
{
#ifdef AVTK_SNDFILE
	SF_INFO info;
	memset( &info, 0, sizeof( SF_INFO ) );
	SNDFILE* const sndfile = sf_open( path.c_str(), SFM_READ, &info);
	if ( !sndfile ) {
		printf("Failed to open sample '%s'\n", path.c_str() );
		return -1;
	}

	if( !(info.channels == 1 || info.channels == 2) ) {
		int chnls = info.channels;
		printf("Loading sample %s, channels = %i\n", path.c_str(), chnls );
		return -1;
	}

	sample.resize( info.frames * info.channels );

	sf_seek(sndfile, 0ul, SEEK_SET);
	sf_read_float( sndfile, &sample[0], info.frames * info.channels );
	sf_close(sndfile);

	return OPENAV_OK;
#else
	if( printErrors ) {
		printf("AVTK compiled without SNDFILE support: cannot load audio sample.\n");
	}
	return OPENAV_ERROR;
#endif
}

int fileUpLevel( std::string path, std::string& newPath )
{
	int forwardSlash = path.rfind('/');
	newPath = path.substr( 0, forwardSlash-path.size() );
	if( newPath[newPath.size()-1] != '/' )
		newPath += '/';

	return OPENAV_OK;
}


int directories( std::string d, std::vector< std::string >& files, bool nameOnly, bool printErrors )
{
	tinydir_dir dir;
	if (tinydir_open(&dir, d.c_str() ) == -1) {
		if( printErrors )
			printf("Error opening dir %s", d.c_str() );
		tinydir_close(&dir);
		return OPENAV_ERROR;
	}

	while (dir.has_next) {
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1) {
			if( printErrors )
				printf("Error getting file from dir %s\n", d.c_str() );
			return OPENAV_ERROR;
		}

		if ( file.is_dir ) {
			// no . or ..
			if( strcmp(file.name, "..") != 0 && strcmp( "." ,file.name) != 0 ) {
				if ( nameOnly ) {
					files.push_back( file.name );
				} else {
					std::stringstream s;
					s << d << "/" << file.name;
					files.push_back( s.str() );
				}
			}
		}

		tinydir_next(&dir);
	}

	// sort them alphabetically
	std::sort( files.begin(), files.end() );

	return OPENAV_OK;
}

int directoryContents( std::string d, std::vector< std::string >& files, std::string& strippedFilenameStart, bool nameOnly, bool smartShortStrings, bool printErrors )
{
	files.clear();

	tinydir_dir dir;
	if (tinydir_open(&dir, d.c_str() ) == -1) {
		if( printErrors )
			printf("Error opening dir %s", d.c_str() );
		tinydir_close(&dir);
		return OPENAV_ERROR;
	}

	// if we have the full path, don't smart-remove the path!
	if( !nameOnly ) {
		smartShortStrings = false;
	}

	// for smartShortStrings, we keep the shortest common string from a directory,
	// and take those characters away from each item: providing a neat listing.
	std::string commonStart;
	int nCharSame = 0;
	bool tryCommonStart = true;

	while (dir.has_next) {
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1) {
			if( printErrors )
				printf("Error getting file from dir %s\n", d.c_str() );
			return OPENAV_ERROR;
		}

		if ( !file.is_dir ) {
			if ( nameOnly ) {
				files.push_back( file.name );
				if( tryCommonStart && commonStart.size() == 0 ) {
#ifdef AVTK_DEBUG_FILE_OPS
					printf("commonStart init %s\n", file.name );
#endif
					commonStart = file.name;
					nCharSame = commonStart.size();
				} else if( tryCommonStart ) {
					// compare with commonStart, and find common N characters
					int maxLen = strlen( commonStart.c_str() );
					if( strlen( file.name ) <= maxLen )
						maxLen = strlen( file.name );

					if( maxLen > nCharSame )
						maxLen = nCharSame;

					for(int i = 0; i < maxLen; i++ ) {
						if( commonStart[i] != file.name[i] ) {
#ifdef AVTK_DEBUG_FILE_OPS
							printf("char # %i is not equal!\n", i );
#endif
							nCharSame = i;
							break;
						}
					}

					if( nCharSame == 0 ) {
						tryCommonStart = false;
					} else {
						commonStart = commonStart.substr( 0, nCharSame );
#ifdef AVTK_DEBUG_FILE_OPS
						printf("Common chars = %i, %s\n", nCharSame, commonStart.c_str() );
#endif
					}
				}
			} else {
				std::stringstream s;
				s << d << "/" << file.name;
				files.push_back( s.str() );
			}
		}

		tinydir_next(&dir);
	}

	/// if smartShortStrings, we strip the starting nCharSame from every name
	// TODO: if there's only 1 file, or two files who's filenames totally match
	// except for the final part of the extension (eg foo.wav foo.wav.bak) don't
	// remove the whole foo.wav string: it essentially hides the file from the user!
	if( smartShortStrings ) {
		strippedFilenameStart = commonStart;

		for(int i = 0; i < files.size(); i++ ) {
			// remove common string
			int fSize = files.at(i).size();
			std::string tmp = files.at(i).substr( nCharSame );

			files.at(i) = tmp;

			/* We would need to remember *every* file's extension individually:
			    its possible, but demands an Avtk::File class or such to handle well
			// remove dot extension from file, eg: ".wav"
			int dotPos = files.at(i).rfind(".");
			if( dotPos != std::string::npos )
			{
			  files.at(i) = files.at(i).substr( 0, dotPos );
			  printf("dotPos of %s = %i\n", files.at(i).c_str(), dotPos );
			}
			*/
#ifdef AVTK_DEBUG_FILE_OPS
			printf("i : %s\n", files.at(i).c_str() );
#endif
		}
	}

	// sort them alphabetically
	std::sort( files.begin(), files.end() );

	tinydir_close(&dir);

	return OPENAV_OK;
}


} //  Avtk namespace
