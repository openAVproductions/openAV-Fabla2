
/// This file tests the Sampler class

#include <stdio.h>
#include "qunit.hxx"

QUnit::UnitTest qunit = QUnit::UnitTest( QUnit::normal, true );

#include "../sampler.hxx"
#include "../pad.hxx"
#include "../sample.hxx"
#include "../yasper.hxx"

using namespace Fabla2;

class Tmp
{
  public:
    Tmp(){}
    
    void add( Sample* sptr )
    {
      samples.push_back(sptr);
    }
    
    //yasper::ptr<Sample> s;
    std::vector< yasper::ptr<Sample> > samples;
};

int main()
{
  printf("Fabla Testing Suite: %s\n", FABLA2_VERSION_STRING );
  
  {
    Tmp c;
    c.add( new Sample( 0, 44100, "Test", "test.wav") );
  }
  
  /*
  QUNIT_IS_TRUE( 2 == 2 );
  
  Pad* p = new Pad( 0, 44100, 0);
  p->add( samp );
  
  Sampler* s = new Sampler( 0, 44100 );
  
  s->play( p, 64 );
  
#define FABLA2_TEST_BUF_SIZE 128
  float audioL[FABLA2_TEST_BUF_SIZE];
  float audioR[FABLA2_TEST_BUF_SIZE];
  // -fsanitze=address test
  //audioL[FABLA2_TEST_BUF_SIZE] = 0;
  
  for(int i = 0; i < 25000; i++ )
  {
    if( i % 500 == 0 )
      s->play( p, 64 );
    
    s->process( FABLA2_TEST_BUF_SIZE, audioL, audioR );
  }
  delete s;
  delete p;
  */
  
  return 0;
}
