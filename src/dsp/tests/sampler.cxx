
/// This file tests the Sampler class

#include "../sampler.hxx"
#include "../pad.hxx"
#include "../sample.hxx"

using namespace Fabla2;

int main()
{
  Sample* samp = new Sample( 0, 44100, "Test", "test.wav");
  
  Pad* p = new Pad( 0, 44100 );
  p->add( samp );
  
  Sampler* s = new Sampler( 0, 44100 );
  
  delete s;
  delete p;
  //delete samp;
  
  return 0;
}
