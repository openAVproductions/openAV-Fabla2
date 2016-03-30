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
#ifndef OPENAV_AVTK_MIDI_H
#define OPENAV_AVTK_MIDI_H

#include <vector>

/** SeqEvent
 *  Simple bass class for deriving sequencable events from. Allows for future
 *  expansion of the Sequencer to hold arbitrary new types of events.
 */
class SeqEventBase
{
public:
	/// create new EventBase: optionally specify time and duration
	SeqEventBase( float time = -1, float duration = -1 );
	virtual ~SeqEventBase() {};

	/// get the event's unique ID
	unsigned long getID();

	/// get/set the time of the event
	float getTime()
	{
		return time;
	}
	void setTime(float t)
	{
		time = t;
	}

	/// returns integer beat from start of loop
	int getBeat()
	{
		return int(time);
	}

	float getDuration()
	{
		return duration;
	}

	/// returns amount of time from the previous beat as float between 0-1.
	/// 0 is exactly on the beat, while 0.25 is 25% of the way from the current
	/// beat towards the next beat
	float getOffset()
	{
		return ( time - int(time) );
	}

protected:
	void setUniqueID( unsigned long uid )
	{
		ID = uid;
	}

private:
	unsigned long ID;
	static unsigned long privateID;

	/// The timestamp of any SeqEvent.
	float time;

	/// If duration makes sense, the events end time is time+duration
	float duration;
};

/** MidiEvent
 *  Represents a single MIDI event. Always 3 bytes, interpret as standard MIDI.
 */
class MidiEvent : public SeqEventBase
{
public:
	/// create empty midi event
	MidiEvent();

	/// creates a new MidiEvent specifying start time & duration: optional data
	MidiEvent( float time, float duration, unsigned char* d = 0);

	void setData( unsigned char* d );
	unsigned char data[3];
};

/** SeqEventList
 *  Holds a list of SeqEvents, allows for adding / swapping contents.
 *  MidiLooper uses instances to hold & interact with clip contents.
 */
class SeqEventList
{
public:
	SeqEventList(int scene);
	~SeqEventList();

	int getLoopLength();
	void setLoopLenght(int l);

	void add( MidiEvent* m );
	void modify( MidiEvent m );

	int numEvents();
	SeqEventBase* getNext();

	/// move to next event. True if there was one, false if not
	bool moveToNextEvent();

	/// restart the clip from the beginning
	void queueFromStart();

	/// clear all contents, freeing memory: non-RT safe
	void nonRtClear();

private:
	unsigned long ID;
	static unsigned long privateID;

	int scene;

	int loopLengthBeats;

	/// number of events in clip
	unsigned int eventCount;

	/// current index of event
	unsigned int eventIndex;

	/// holds pointers to each event
	int eventCapacity;
	std::vector<SeqEventBase*> events;
};



#endif // OPENAV_AVTK_MIDI_H

