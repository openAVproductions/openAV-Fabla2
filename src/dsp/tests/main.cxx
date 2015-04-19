
/// This file tests the Sampler class

#include <stdio.h>
#include "qunit.hxx"

QUnit::UnitTest qunit = QUnit::UnitTest( QUnit::normal, true );

#include "plotter.h"

#include "../sampler.hxx"
#include "../pad.hxx"
#include "../sample.hxx"
#include "../yasper.hxx"
#include "../voice.hxx"

using namespace Fabla2;

int main()
{
  printf("Fabla Testing Suite: %s\n", FABLA2_VERSION_STRING );
  
  Sample* samp = new Sample( 0, 44100, "Test", "test.wav");
  
  samp->attack  = 0;
  samp->decay   = 0;
  samp->sustain = 0;
  samp->release = 0;
  
  Pad* p = new Pad( 0, 44100, 0);
  p->add( samp );
  
  Sampler* s = new Sampler( 0, 44100 );
  
  Voice* v = new Voice( 0, 44100 );
  
  v->play( 0, 0, 0, p, 1 );
  
#define FABLA2_TEST_BUF_SIZE 44100
  float audioL[FABLA2_TEST_BUF_SIZE];
  float audioR[FABLA2_TEST_BUF_SIZE];
  // -fsanitze=address test
  //audioL[FABLA2_TEST_BUF_SIZE] = 0;
  
  for(int i = 0; i < 1; i++ )
  {
    s->process( FABLA2_TEST_BUF_SIZE, audioL, audioR );
  }
  
  plotter_write( "out.dat", FABLA2_TEST_BUF_SIZE, audioL );
  
  delete s;
  delete p;
  
  return 0;
}
