
/// This file tests the Sampler class

#include <stdio.h>
#include "qunit.hxx"

QUnit::UnitTest qunit = QUnit::UnitTest( QUnit::normal, true );

#include "../sampler.hxx"
#include "../pad.hxx"
#include "../sample.hxx"

using namespace Fabla2;

int main()
{
  printf("Fabla Testing Suite: %s\n", FABLA2_VERSION_STRING );
  
  Sample* samp = new Sample( 0, 44100, "Test", "test.wav");
  
  QUNIT_IS_TRUE( 2 == 2 );
  
  Pad* p = new Pad( 0, 44100 );
  p->add( samp );
  
  Sampler* s = new Sampler( 0, 44100 );
  
  s->play( 64, p );
  
  delete s;
  delete p;
  
  return 0;
}
