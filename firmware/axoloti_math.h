/**
 * Copyright (C) 2013, 2014 Johannes Taelman
 *
 * This file is part of Axoloti.
 *
 * Axoloti is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Axoloti is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Axoloti. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _AXOLOTI_MATH_H
#define _AXOLOTI_MATH_H

#include "ch.h"
#include "hal.h"

#include <stdint.h>
#include "math.h"

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wstrict-aliasing"
//#define __GNUC__ TRUE
#include "arm_math.h"
//#undef __CORE_CM4_SIMD_H
//#include "core_cm4_simd.h"
//#pragma GCC diagnostic pop

#define SINETSIZE 1024
extern int16_t sinet[SINETSIZE + 1];
#define SINE2TSIZE 4096
extern int32_t sine2t[SINE2TSIZE + 1];
#define WINDOWSIZE 1024
extern int16_t windowt[WINDOWSIZE + 1];
#define PITCHTSIZE 257
extern uint32_t pitcht[PITCHTSIZE];
#define EXPTSIZE 256
extern uint16_t expt[EXPTSIZE];
#define LOGTSIZE 256
#define LOGTSIZEN 8
extern uint16_t logt[LOGTSIZE];

typedef union {
  int32_t i;
  float f;
  struct {
    uint32_t mantissa :23;
    uint32_t exponent :8;
    uint32_t sign :1;
  } parts;
} Float_t;

void axoloti_math_init(void);

/* Calculate pseudo-random 32 bit number based on linear congruential method. */
uint32_t GenerateRandomNumber(void);

uint32_t FastLog(uint32_t f);

//int32_t ___SMMUL (int32_t op1, int32_t op2);
//int32_t ___SMMLA (int32_t op1, int32_t op2, int32_t op3);

__attribute__( ( always_inline ) ) __STATIC_INLINE int32_t ___SMMUL (int32_t op1, int32_t op2)
{
  int32_t result;

  __ASM volatile ("smmul %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2));
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE int32_t ___SMMLA (int32_t op1, int32_t op2, int32_t op3)
{
  int32_t result;

  __ASM volatile ("smmla %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE int32_t ___SMMLS (int32_t op1, int32_t op2, int32_t op3)
{
  int32_t result;

  __ASM volatile ("smmls %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

__attribute__ ( ( always_inline ) ) __STATIC_INLINE float _VSQRTF(float op1) {
  float result;
  __ASM volatile ("vsqrt.f32 %0, %1" : "=w" (result) : "w" (op1) );
  return(result);
}

#define MTOF(pitch, frequency)    \
{                                 \
  int32_t p1 = (int32_t)(pitch);  \
  int32_t p=__SSAT(p1,28);        \
  uint32_t pi = p>>21;            \
  int32_t y1 = pitcht[128+pi];    \
  int32_t y2 = pitcht[128+1+pi];  \
  int32_t pf= (p&0x1fffff)<<10;   \
  int32_t pfc = INT32_MAX - pf;   \
  uint32_t r;                     \
  r = ___SMMUL(y1,pfc);           \
  r = ___SMMLA(y2,pf,r);          \
  frequency= (r<<1);              \
}

#define MTOFEXTENDED(pitch, frequency)\
{                                 \
  int32_t p1 = (int32_t)(pitch);  \
  int32_t p=__SSAT(p1,29);        \
  uint32_t pi = p>>21;            \
  int32_t y1 = pitcht[128+pi];    \
  int32_t y2 = pitcht[128+1+pi];  \
  int32_t pf= (p&0x1fffff)<<10;   \
  int32_t pfc = INT32_MAX - pf;   \
  uint32_t r;                     \
  r = ___SMMUL(y1,pfc);           \
  r = ___SMMLA(y2,pf,r);          \
  frequency= (r<<1);              \
}

#define SINE2TINTERP(phase, output) \
{                                   \
  uint32_t p = (uint32_t)(phase);   \
  uint32_t pi = p>>20;              \
  int32_t y1 = sine2t[pi];          \
  int32_t y2 = sine2t[1+pi];        \
  int32_t pf= (p&0xfffff)<<11;      \
  int32_t pfc = INT32_MAX - pf;     \
  int32_t rr;                       \
  rr = ___SMMUL(y1,pfc);            \
  rr = ___SMMLA(y2,pf,rr);          \
  output = (rr<<1);                 \
}

#define HANNING2TINTERP(phase, output) \
{ uint32_t p = phase;               \
  uint32_t pi = p>>22;              \
  int32_t y1 = windowt[pi];         \
  int32_t y2 = windowt[1+pi];       \
  int32_t pf= (p&0x3fffff)<<9;      \
  int32_t pfc = INT32_MAX - pf;     \
  int32_t rr;                       \
  rr = ___SMMUL(y1<<16,pfc);        \
  rr = ___SMMLA(y2<<16,pf,rr);      \
  output = (rr<<1);                 \
}

__attribute__ ( ( always_inline ) ) __STATIC_INLINE int32_t ConvertIntToFrac(int i) {
  return (i<<21);
}

__attribute__ ( ( always_inline ) ) __STATIC_INLINE int32_t ConvertFracToInt(int i) {
  return (i>>21);
}

__attribute__ ( ( always_inline ) ) __STATIC_INLINE int32_t ConvertFloatToFrac(float f) {
  return (int32_t)(f*(1<<21));
}

#endif
