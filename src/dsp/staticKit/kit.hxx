
#ifndef FABLA2_STATIC_KIT_HXX
#define FABLA2_STATIC_KIT_HXX

#include "hat.hxx"
#include "hat_open.hxx"
#include "hat_sizzle.hxx"
#include "ride.hxx"
#include "kick.hxx"
#include "snare.hxx"
#include "snare_clean.hxx"
#include "snare_rim_clean.hxx"

struct Samp
{
  int size;
  float* data;
};

static Samp samps[1] = {
{
  hat::size,
  hat::wavetable
},
};

#endif /* FABLA2_STATIC_KIT_HXX */
