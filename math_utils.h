/*
 * Copyright (C) 2000, 2013 Thierry Crozat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: criezy01@gmail.com
 */

#ifndef math_utils_h
#define math_utils_h

#include <assert.h>

namespace MathUtils {

/*! \fn template <class T> T abs(T)
 *
 * Return the absolute value of the given number.
 */
template <class T> inline T abs(T a) {
	return a < 0 ? -a : a;
}

// Avoid breaking strict-aliasing rules when interpreting a float as an int in isEqual
typedef union { int i_; float f_; } ULP_float;
// Avoid breaking strict-aliasing rules when interpreting a double as a long long in isEqual
typedef union { long long l_; double d_; } ULP_double;

/*! \fn bool isEqual(float a, float b, int ulp_error = 100)
 *
 * \return True if the two given numbers are equal within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of floats that can be represented between \p a and \p b). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isEqual(float, float, float, bool) function that takes a relative
 * or an absolute error.
 *
 * \see isEqual(float, float, float, bool)
 *
 * Original author : Bruce Dawson (function AlmostEqualUlpsFinal() --
 * "Comparing floating point numbers").
 *
 * http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 *
 \verbatim
	Some examples with relative is true and error is 1e-5
	A            B        ULP error  ULP(A-B) isEqual
	+1.000001    +1.00000     160     8        true
	+1.00001     +1.00000     84      84       true
	+0.99999     +1.00000     168     168      true
	+1.0001      +1.00000     84      839      false
	-1.00001     -1.00000     84      84       true
	-0.99999     -1.00000     168     168      true
	-1.0001      -1.00000     84      839      false
	+100001      +100000      128     128      true
	+99999       +100000      128     128      true
	+100010      +100000      128     1280     false
 \endverbatim
 *
 * Note that infinity and NaN tests are disabled, so comparisons may give
 * unexpected results when both numbers are infinite or NaN (if only one is,
 * this function will return false as expected in most cases). If you think
 * this is a problem, call isFinite() first. If this really is a problem,
 * see comments in source code.
 */
inline bool isEqual(float a, float b, int ulp_error) {
	// Check if the size of a float is the size of a int
	// it should be ok with most of the compilers.
	assert(sizeof(float) == sizeof(int));

	// There are several optional checks that you can do, depending
	// on what behavior you want from your floating point comparisons.
	// These checks should not be necessary and they are included
	// mainly for completeness.

	// These tests are commented out because they are relatively slow (about
	// 2/3 of the total runtime). Also, the #define are not used here because
	// the two functions with relative error still use them.
#ifdef INFINITYCHECK
	// If a or b are infinity (positive or negative) then
	// only return true if they are exactly equal to each other -
	// that is, if they are both infinities of the same sign.
	// This check is only needed if you will be generating
	// infinities and you don't want them 'close' to numbers
	// near FLT_MAX.
	if (isInfinite(a) || isInfinite(b))
		return a == b;
#endif

#ifdef NANCHECK
	// If a or b are a NAN, return false. NANs are equal to nothing,
	// not even themselves.
	// This check is only needed if you will be generating NANs
	// and you use a max_ulps greater than 4 million or you want to
	// ensure that a NAN does not equal itself.
	if (isNan(a) || isNan(b))
		return false;
#endif


	ULP_float a_int, b_int;

	a_int.f_ = a;
	// Make a_int lexicographically ordered as a twos-complement int
	if (a_int.i_ < 0)
		a_int.i_ = 0x80000000 - a_int.i_;
	// Make b_int lexicographically ordered as a twos-complement int
	b_int.f_ = b;
	if (b_int.i_ < 0)
		b_int.i_ = 0x80000000 - b_int.i_;

	// Now we can compare a_int and b_int to find out how far apart A and B are.
	// The trick is, d = a_int.i_ - b_int.i_ may overflow an int if a or b is
	// negative. When this happens, d will wrap around the min (or max) of an
	// int and give wrong results.
	//
	// For example, take isEqual(4.0000005, -1):
	//  4.0000005 ->  1082130433
	// -1         -> -1082130432 (two-complement = -1065353216)
	// delta = 1 082 130 433 - -1 065 353 216 = 2 147 483 649
	// which happens to be 2^31 + 1 (max int + 2), so it is wrapped back to
	// -2 147 483 647 instead. This is wrong, but not a problem here as we take
	// abs(). It could become a problem if the wrap around became so small that
	// it was below ulp_error, but for floats, the maximum gap is -1e38 to 1e38
	// and that leads to 47 387 342 (instead of a true delta of 4 247 579 954).
	// So this is wrong, but for small values of ulp_error, doesn't matter.
	//
	// However for isEqual(4, -1), this becomes:
	//  4 -> 1082130432
	// -1 -> -1082130432 (two-complement = -1065353216)
	// delta = 2 147 483 648 = 2^31 = max int + 1, this wraps to -2 147 483 648
	// Now, abs(delta) reverses the sign since delta is < 0, giving
	// 2 147 483 648... which is itself max int + 1 and wraps again to
	// -2 147 483 648! (in other words, since |min_int| > max_int, |min_int| is
	// ill-defined and happens to be min_int... this is documented in abs(3))
	// As a result, abs(delta) is still < 0 and isEqual(-1, 4) is true!
	//
	// The same would happen for every pair of values where the int encoding is
	// exactly the same, except for the sign (for example -2 and 2, -3 and 1.5,
	// and so on).
	//
	// Solution? Store the difference in a type that will always be big enough.
	// This fixes the first problem (which wasn't a problem, but wasn't correct)
	// and the second.
	return abs((long long)a_int.i_ - b_int.i_) <= ulp_error;
}

/*! \fn bool isEqual(double a, double b, long long ulp_error = 100)
 *
 * \return True if the two given numbers are equal within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of doubles that can be represented between \p a and \p b). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isEqual(double, double,double , bool) function that takes a
 * relative or an absolute error.
 *
 * \see  isEqual(double, double,double , bool)
 *
 * Original author : Bruce Dawson (function AlmostEqualUlpsFinal() --
 * "Comparing floating point numbers").
 *
 * http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

 * \return True if the two given numbers are equal within a given error.
 * If \p relative is true (default) the error is a relative error,
 * otherwise it is an absolute error. The error must be a positive number.
 *
 * Original author : Bruce Dawson (function AlmostEqualUlpsFinal() --
 * "Comparing floating point numbers").
 *
 * Note that infinity and NaN tests are disabled, so comparisons may give
 * unexpected results when both numbers are infinite or NaN (if only one is,
 * this function will return false as expected in most cases). If you think
 * this is a problem, call isFinite() first. If this really is a problem,
 * see comments in source code.
 */
inline bool isEqual(double a, double b, long long ulp_error = 100) {
	// Check if the size of a double is the size of a long long
	// it should be ok with most of the compilers.
	assert(sizeof(double) == sizeof(long long));

	// For more comments, see bool isEqual(float , float , int, bool)

#ifdef INFINITYCHECK
	if (isInfinite(a) || isInfinite(b))
		return a == b;
#endif

#ifdef NANCHECK
	if (isNan(a) || isNan(b))
		return false;
#endif

	ULP_double a_long, b_long;
	a_long.d_ = a;
	// Make a_long lexicographically ordered as a twos-complement long
	if (a_long.l_ < 0)
		a_long.l_ = 0x8000000000000000LL - a_long.l_;
	// Make b_long lexicographically ordered as a twos-complement long
	b_long.d_ = b;
	if (b_long.l_ < 0)
		b_long.l_ = 0x8000000000000000LL - b_long.l_;

	// Now we can compare a_long and b_long to find out how far apart A and B are.
	// We have the same problem of overflow as above, but there is no type
	// larger than a long long to store the result. Instead, we add the error
	// to the smallest of the two numbers and see if this is larger than the
	// biggest number. This may still overflow if (smallest + error) > max_int,
	// but this requires either the error to be very large (usually it is 100),
	// or the two numbers to be very close and very close to max_int. Actually,
	// given that max_int (in 64 bits) seems to correspond to a NaN in double,
	// I'm not sure what happens there. We could make some additional tests
	// comparing first (2^63-1 - smallest) to see if it is <= error, but that
	// is overkill, useless unless the two doubles are around 1e+308, and would
	// cause again underflow if the smallest is smaller than 0 (because
	// 2^63-1 - (-n) > 2^63-1), so another test on > 0 is needed.
	if (a_long.l_ > b_long.l_)
		return (b_long.l_ + ulp_error >= a_long.l_);
		// the correct code would be (not tested):
		// return (b_long.l_ > 0 && 9223372036854775807 - b_long.l_ <= ulp_error) || (b_long.l_ + ulp_error >= a_long.l_);
	else
		return (a_long.l_ + ulp_error >= b_long.l_);
}

/*! \fn bool isInfOrEqual(float a, float b, int ulp_error = 100)
 *
 * \return True if the first number is inferior or equal to the second one
 * within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of floats that can be represented between \p a and \p b). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isInfOrEqual(float, float, float , bool) function that takes
 * a relative or an absolute error.
 *
 * \see isInfOrEqual(float, float, float, bool)
 */
inline bool isInfOrEqual(float a, float b, int ulp_error = 100) {
	// Check if the size of a float is the size of a int
	// it should be ok with most of the compilers.
	assert(sizeof(float) == sizeof(int));

	// For more comments, see bool isEqual(float , float , int)

#ifdef INFINITYCHECK
	if (isInfinite(a) || isInfinite(b))
		return a == b;
#endif
#ifdef NANCHECK
	if (isNan(a) || isNan(b))
		return false;
#endif

	ULP_float a_int, b_int;
	a_int.f_ = a;
	// Make a_int lexicographically ordered as a twos-complement int
	if (a_int.i_ < 0)
		a_int.i_ = 0x80000000 - a_int.i_;
	// Make b_int lexicographically ordered as a twos-complement int
	b_int.f_ = b;
	if (b_int.i_ < 0)
		b_int.i_ = 0x80000000 - b_int.i_;

	// Now we can compare a_int and b_int to find out how far apart A and B are.
	// Cast to 64-bits to avoid the problem described in isEqual().
	return ((long long)a_int.i_ - b_int.i_) <= ulp_error;
}

/*! \fn bool isInfOrEqual(double a, double b, int ulp_error = 100)
 *
 * \return True if the first number is inferior or equal to the second one
 * within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of doubles that can be represented between \p a and \p b). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isInfOrEqual(double, double, double , bool) function that
 * takes a relative or an absolute error.
 *
 * \see isInfOrEqual(double, double, double, bool)
 */
inline bool isInfOrEqual(double a, double b, int ulp_error = 100) {
	// Check if the size of a double is the size of a long long
	// it should be ok with most of the compilers.
	assert(sizeof(double) == sizeof(long long));

	// For more comments, see bool isEqual(float , float , int)

//#ifdef INFINITYCHECK
//	if (isInfinite(a) || isInfinite(b))
//		return a == b;
//#endif

//#ifdef NANCHECK
//	if (isNan(a) || isNan(b))
//		return false;
//#endif

	ULP_double a_long, b_long;
	a_long.d_ = a;
	// Make a_long lexicographically ordered as a twos-complement long
	if (a_long.l_ < 0)
		a_long.l_ = 0x8000000000000000LL - a_long.l_;
	// Make b_long lexicographically ordered as a twos-complement long
	b_long.d_ = b;
	if (b_long.l_ < 0)
		b_long.l_ = 0x8000000000000000LL - b_long.l_;

	// Now we can compare a_long and b_long to find out how far apart A and B are.
	// Compare with error to avoid the problem described in isEqual().
	if (a_long.l_ > b_long.l_)
		return (b_long.l_ + ulp_error >= a_long.l_);
	else
		return true;
}


/*! \fn bool isSupOrEqual(float a, float b, int ulp_error = 100)
 *
 * \return True if the first number is superior or equal to the second one
 * within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of floats that can be represented between \p b and \p a). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isSupOrEqual(float, float, float , bool) function that takes
 * a relative or an absolute error.
 *
 * \see isSupOrEqual(float, float, float, bool)
 */
inline bool isSupOrEqual(float a, float b, int ulp_error = 100) {
	return isInfOrEqual(b, a, ulp_error);
}

/*! \fn bool isSupOrEqual(double a, double b, int ulp_error = 100)
 *
 * \return True if the first number is superior or equal to the second one
 * within a given error.
 *
 * The error is given as a ULP error (Unit in the Last Place, i.e. the number
 * of doubles that can be represented between \p b and \p a). See reference
 * below for more details.
 *
 * If you want to control the error, and since a ULP error is difficult to grasp,
 * you can use the isSupOrEqual(double, double, double , bool) function that
 * takes a relative or an absolute error.
 *
 * \see isSupOrEqual(double, double, double, bool)
 */
inline bool isSupOrEqual(double a, double b, int ulp_error = 100) {
	return isInfOrEqual(b, a, ulp_error);
}

} // namespace MathUtils

#endif
