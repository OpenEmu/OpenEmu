/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/debug.h"
#include "common/rational.h"
#include "common/util.h"
#include "common/algorithm.h"

namespace Common {

Rational::Rational() {
	_num   = 1;
	_denom = 1;
}

Rational::Rational(int num) {
	_num   = num;
	_denom = 1;
}

Rational::Rational(int num, int denom) {
	assert(denom != 0);

	if (denom > 0) {
		_num   = num;
		_denom = denom;
	} else {
		_num   = -num;
		_denom = -denom;
	}

	cancel();
}

void Rational::cancel() {
	// Cancel the fraction by dividing both the num and the denom
	// by their greatest common divisor.

	const int gcd = Common::gcd(_num, _denom);

	_num   /= gcd;
	_denom /= gcd;
}

Rational &Rational::operator=(const Rational &right) {
	_num   = right._num;
	_denom = right._denom;

	return *this;
}

Rational &Rational::operator=(int right) {
	_num   = right;
	_denom = 1;

	return *this;
}

Rational &Rational::operator+=(const Rational &right) {
	// Cancel common factors to avoid unnecessary overflow.
	// Note that the result is *not* always normalized.
	const int gcd = Common::gcd(_denom, right._denom);

	_num    = _num * (right._denom / gcd);
	_denom  = _denom / gcd;
	_num   += right._num * _denom;
	_denom *= right._denom;

	cancel();

	return *this;
}

Rational &Rational::operator-=(const Rational &right) {
	// Cancel common factors to avoid unnecessary overflow.
	// Note that the result is *not* always normalized.
	const int gcd = Common::gcd(_denom, right._denom);

	_num    = _num * (right._denom / gcd);
	_denom  = _denom / gcd;
	_num   -= right._num * _denom;
	_denom *= right._denom;

	cancel();

	return *this;
}

Rational &Rational::operator*=(const Rational &right) {
	// Cross-cancel to avoid unnecessary overflow;
	// the result then is automatically normalized
	const int gcd1 = gcd(_num, right._denom);
	const int gcd2 = gcd(right._num, _denom);

	_num   = (_num    / gcd1) * (right._num    / gcd2);
	_denom = (_denom  / gcd2) * (right._denom  / gcd1);

	return *this;
}

Rational &Rational::operator/=(const Rational &right) {
	return *this *= right.getInverse();
}

Rational &Rational::operator+=(int right) {
	return *this += Rational(right);
}

Rational &Rational::operator-=(int right) {
	return *this -= Rational(right);
}

Rational &Rational::operator*=(int right) {
	return *this *= Rational(right);
}

Rational &Rational::operator/=(int right) {
	return *this /= Rational(right);
}

const Rational Rational::operator-() const {
	return Rational(-_num, _denom);
}

const Rational Rational::operator+(const Rational &right) const {
	Rational tmp = *this;
	tmp += right;
	return tmp;
}

const Rational Rational::operator-(const Rational &right) const {
	Rational tmp = *this;
	tmp -= right;
	return tmp;
}

const Rational Rational::operator*(const Rational &right) const {
	Rational tmp = *this;
	tmp *= right;
	return tmp;
}

const Rational Rational::operator/(const Rational &right) const {
	Rational tmp = *this;
	tmp /= right;
	return tmp;
}

const Rational Rational::operator+(int right) const {
	Rational tmp = *this;
	tmp += right;
	return tmp;
}

const Rational Rational::operator-(int right) const {
	Rational tmp = *this;
	tmp -= right;
	return tmp;
}

const Rational Rational::operator*(int right) const {
	Rational tmp = *this;
	tmp *= right;
	return tmp;
}

const Rational Rational::operator/(int right) const {
	Rational tmp = *this;
	tmp /= right;
	return tmp;
}

bool Rational::operator==(const Rational &right) const {
	return (_num == right._num) && (_denom == right._denom);
}

bool Rational::operator!=(const Rational &right) const {
	return (_num != right._num) || (_denom != right._denom);
}

bool Rational::operator>(const Rational &right) const {
	return (_num * right._denom) > (right._num * _denom);
}

bool Rational::operator<(const Rational &right) const {
	return (_num * right._denom) < (right._num * _denom);
}

bool Rational::operator>=(const Rational &right) const {
	return (_num * right._denom) >= (right._num * _denom);
}

bool Rational::operator<=(const Rational &right) const {
	return (_num * right._denom) <= (right._num * _denom);
}

bool Rational::operator==(int right) const {
	return (_denom == 1) && (_num == right);
}

bool Rational::operator!=(int right) const {
	return (_denom != 1) || (_num != right);
}

bool Rational::operator>(int right) const {
	return *this > Rational(right, 1);
}

bool Rational::operator<(int right) const {
	return *this < Rational(right, 1);
}

bool Rational::operator>=(int right) const {
	return *this >= Rational(right, 1);
}

bool Rational::operator<=(int right) const {
	return *this <= Rational(right, 1);
}

void Rational::invert() {
	assert(_num != 0);

	SWAP(_num, _denom);

	if (_denom < 0) {
		_denom = -_denom;
		_num = -_num;
	}
}

Rational Rational::getInverse() const {
	Rational inverse = *this;

	inverse.invert();

	return inverse;
}

int Rational::toInt() const {
	return _num / _denom;
}

double Rational::toDouble() const {
	return ((double)_num) / ((double)_denom);
}

frac_t Rational::toFrac() const {
	return (_num * FRAC_ONE) / _denom;
}

const Rational operator+(int left, const Rational &right) {
	Rational tmp(left);
	tmp += right;
	return tmp;
}

const Rational operator-(int left, const Rational &right) {
	Rational tmp(left);
	tmp -= right;
	return tmp;
}

const Rational operator*(int left, const Rational &right) {
	Rational tmp(left);
	tmp *= right;
	return tmp;
}

const Rational operator/(int left, const Rational &right) {
	Rational tmp(left);
	tmp /= right;
	return tmp;
}

void Rational::debugPrint(int debuglevel, const char *caption) const {
	debug(debuglevel, "%s %d/%d", caption, _num, _denom);
}

bool operator==(int left, const Rational &right) {
	return right == left;
}

bool operator!=(int left, const Rational &right) {
	return right != left;
}

bool operator>(int left, const Rational &right) {
	return right < left;
}

bool operator<(int left, const Rational &right) {
	return right > left;
}

bool operator>=(int left, const Rational &right) {
	return right <= left;
}

bool operator<=(int left, const Rational &right) {
	return right >= left;
}

} // End of namespace Common
