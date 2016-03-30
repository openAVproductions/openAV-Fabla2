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

#include "midi.hxx"

#include <stdio.h>
#include <cstring>

unsigned long SeqEventBase::privateID = 0;
unsigned long SeqEventList::privateID = 0;

SeqEventBase::SeqEventBase( float t, float dur )
{
	ID = privateID++;
	time = t;
	duration = dur;
}

MidiEvent::MidiEvent() :
	SeqEventBase( -1, 0 )
{
	memset( data, 0, sizeof(unsigned char) * 3);
}

MidiEvent::MidiEvent(float time, float duration, unsigned char* srcData) :
	SeqEventBase( time, duration )
{
	if ( srcData )
		setData( srcData );
	else // nuke data to zero
		memset( data, 0, sizeof(unsigned char) * 3);
}

void MidiEvent::setData(unsigned char* d)
{
	memcpy( data, d, sizeof(unsigned char) * 3 );
}


#define DEFAULT_NUM_EVENTS 1024

SeqEventList::SeqEventList(int s) :
	ID( privateID++ ),
	scene( s ),
	loopLengthBeats( 8 ),
	eventCount(0),
	eventIndex(0),
	eventCapacity( DEFAULT_NUM_EVENTS )
{
	events.reserve(DEFAULT_NUM_EVENTS);
}

SeqEventList::~SeqEventList()
{
	nonRtClear();
}

void SeqEventList::add( MidiEvent* m )
{
	float t = m->getTime();

	// insert the MidiEvent
	bool inserted = false;
	for(unsigned int i = 0; i < eventCount; i++) {
		if( t < events.at(i)->getTime() ) {
			events.insert( events.begin() + i, (SeqEventBase*)m );
			inserted = true;
			printf("inserted\n");
			break;
		}
	}

	// or append to back
	if ( !inserted )
		events.push_back( m );

	eventCount++;

	/*

	//printf("Event list:\n");
	for(unsigned int i = 0; i < eventCount; i++)
	{
	  //printf( "%f\n", events.at(i)->getTime() );
	}
	//printf("\n");

	*/
}

void SeqEventList::nonRtClear()
{
	for(unsigned int i = 0; i < eventCount; i++) {
		delete events.at(i);
	}
	eventIndex = 0;
	eventCount = 0;
}

int SeqEventList::getLoopLength()
{
	return loopLengthBeats;
}

void SeqEventList::setLoopLenght(int l)
{
	loopLengthBeats = l;
}

void SeqEventList::modify( MidiEvent m )
{

}

int SeqEventList::numEvents()
{
	return eventCount;
}

void SeqEventList::queueFromStart()
{
	eventIndex = 0;
}

SeqEventBase* SeqEventList::getNext()
{
	if ( eventIndex < eventCount )
		return events.at( eventIndex );

	return 0;
}

bool SeqEventList::moveToNextEvent()
{
	eventIndex++;

	if( eventIndex < eventCount ) {
		return true;
	}

	return false;
}


