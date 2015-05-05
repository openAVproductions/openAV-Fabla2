/*
 * Author: Harry van Haaren 2014
 *         harryhaaren@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "pad.hxx"

#include "fabla2.hxx"
#include "sample.hxx"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef FABLA2_COMPONENT_TEST
#include "tests/qunit.hxx"
extern QUnit::UnitTest qunit;
#endif 


namespace Fabla2
{

Pad::Pad( Fabla2DSP* d, int rate, int ID ) :
  dsp( d ),
  sr(rate),
  ID_( ID ),
  muteGroup_( 0 ),
  offGroup_( 0 ),
  triggerMode_( TM_GATED ),
  loaded_( false ),
  
  sampleSwitchSystem( SS_NONE ),
  sampleLayerCounter(0)
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
  
  volume = 0.75f;
  
  for(int i = 0; i < 4; ++i)
    sends[i] = 0.f;
  
  samples.reserve( 8 );
}

void Pad::remove( Sample* s )
{
  assert( s );
  
  for( int i = 0; i < samples.size(); ++i)
  {
    if( samples.at(i) == s )
    {
      samples.erase( samples.begin() + i );
      printf("Pad remove() sample at %i : sample name %s\n", i, s->getName() );
      delete s;
    }
  }
}

void Pad::setName( const char* n )
{
  // char name[21], keep a newline in the last place
  name[20] = '\n';
  
  int chars = strlen( n );
  if( chars > 20 )
  {
    // add better handling of long names?
    // first 3 chars .. last 15 chars - .wav extension?
    chars = 20;
  }
  memcpy(name, n, sizeof(char)*chars);
  
  dsp->writePadsState( bank_, ID_, this );
}

void Pad::add( Sample* s )
{
  assert( s );
  
  loaded_ = true;
  
  //printf("%s, b %i, p %i, s = %i\n", __PRETTY_FUNCTION__, bank_, ID_, s );
  //printf( "Pad::add() %s, total #samples on pad = %i\n", s->getName(), samples.size() );
  samples.push_back( s );
  
  // request DSP to refresh UI layers for this pad
  if( dsp )
  {
    dsp->padRefreshLayers( bank_, ID_ );
    dsp->writePadsState( bank_, ID_, this );
  }
  // update the sample state to the UI
  //dsp->writeSampleState( bank_, ID_, samples.size()-1, this, s );
  
}

Sample* Pad::layer( int id )
{
  if( id < samples.size() && id >= 0 )
    return samples.at(id);
  return 0;
}

int Pad::lastPlayedLayer()
{
  return sampleLayerCounter;
}

void Pad::checkAll()
{
  printf("%s : Starting...\n", __PRETTY_FUNCTION__ );
  for(int bi = 0; bi < nLayers(); bi++ )
  {
    Sample* s = layer( bi );
    if( !s )
    {
      printf("%s : Pad::layer( %i ) returns NULL sample\n", __PRETTY_FUNCTION__, bi );
      return;
    }
    else
    {
      printf("%s : Pad::layer( %i ) has Sample %s\n", __PRETTY_FUNCTION__, bi, s->getName() );
    }
  }
  printf("%s : Done.\n", __PRETTY_FUNCTION__ );
}

Sample* Pad::getPlaySample( float velocity )
{
#ifdef FABLA2_COMPONENT_TEST
  QUNIT_IS_TRUE( samples.size() > 0 );
#endif
  
  /// Logic to do round-robin / velocity mapping here
  if( samples.size() > 0 )
  {
    //printf("playing pad %i, with switch mode %i\n", ID_, int(sampleSwitchSystem) );
    //printf("playing pad %i with sampleLayerCounter %i\n", ID_, sampleLayerCounter );
    
    if( sampleSwitchSystem == SS_NONE )
    {
      //printf("playing pad SS_NONE, layer %i\n", sampleLayerCounter);
      return samples.at( sampleLayerCounter );
    }
    else if( sampleSwitchSystem == SS_ROUND_ROBIN )
    {
      // first update the sample-counter, wrap it if needed. Later we play the
      // sample we just updated to, and its stored in sampleLayerCounter for UI
      sampleLayerCounter++;
      if( sampleLayerCounter >= samples.size() )
        sampleLayerCounter = 0;
      
      //printf("playing pad SS_ROUND_ROBIN %i\n", sampleLayerCounter);
      Sample* tmp = samples.at( sampleLayerCounter );
      return tmp;
    }
    else if( sampleSwitchSystem == SS_VELOCITY_LAYERS )
    {
      // iter trough samples, return first that was add()-ed that applies
      for(int i = 0; i < samples.size(); i++ )
      {
        if( samples.at(i)->velocity( velocity ) )
        {
          // remember last played layer, for UI updates
          sampleLayerCounter = i;
          //printf("playing pad SS_VELOCITY_LAYERS %i\n", sampleLayerCounter );
          return samples.at(i);
        }
        else
        {
          //printf("SS_VELOCITY_LAYERS : sample at %i, *NOT* ok for velocity\n", i );
        }
      }
      
    }
  }
  
  // if no sample is loaded, or a velocity outside the mapped regions is played
  return 0;
}

void Pad::clearAllSamples()
{
  for(int i = 0; i < samples.size(); i++)
  {
    delete samples.at(i);
  }
  samples.clear();
  loaded_ = false;
}

void Pad::switchSystem( SAMPLE_SWITCH_SYSTEM ss )
{
  sampleSwitchSystem = ss;
}

Pad::~Pad()
{
#ifdef FABLA2_COMPONENT_TEST
  printf("%s\n", __PRETTY_FUNCTION__ );
#endif
}

};
