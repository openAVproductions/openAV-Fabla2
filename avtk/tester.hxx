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

#ifndef OPENAV_AVTK_TESTER_HXX
#define OPENAV_AVTK_TESTER_HXX

#ifdef AVTK_TESTER

#include "pugl/pugl.h"

#include <string>
#include <vector>

namespace Avtk
{

class UI;

class AvtkEvent
{
public:
	AvtkEvent( const PuglEvent* e, double time ) :
		event( *e ),
		timestamp( time )
	{
	}

	PuglEvent event;
	double  timestamp;
};

/** Tester
 * The tester class records AVTK / PUGL events, and serializes them to JSON.
 * This allows program interaction to be recorded at the widget level, and then
 * replayed to test the program.
 */
class Tester
{
public:
	Tester( Avtk::UI* ui );

	/// start a recording pass, the test will be saved as avtkTests/<testName>.json
	void record( const char* testName );
	bool recording()
	{
		return recording_;
	}

	/// records the event being passed in
	void handle( const PuglEvent* event );

	/// stops the recording pass, and writes the JSON file
	void recordStop();

	/// runs a specific JSON test file. Set @param ignoreTimestamps to true for
	/// testing timing-independant UI behaviour.
	int runTest( const char* testName, bool ignoreTimestamps = false );

	void writeTest( const char* filename );

	/// process gets called by the UI repeatedly, when Tester is playing back,
	/// events are injected to the UI, otherwise no action is taken.
	void process();

private:
	UI* ui;
	bool playing_;
	bool recording_;
	double startRecTime;
	double startPlayTime;

	long playEventNum;

	std::string name;
	std::vector<AvtkEvent> events;

	double getTime();
};

}; // Avtk

#endif // AVTK_TESTER

#endif // OPENAV_AVTK_TESTER_HXX
