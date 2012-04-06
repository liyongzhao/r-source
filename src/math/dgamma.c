/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Mathlib.h"

double dgamma(double x, double shape, double scale)
{
	if (shape <= 0.0 || scale <= 0.0)
		DOMAIN_ERROR;
	if (x < 0.0)
		return 0.0;
	if (x == 0.0) {
		if (shape < 1.0)
			POS_RANGE_ERROR;
		if (shape > 1.0) {
			return 0.0;
		}
		return 1.0 / scale;
	}
	x = x / scale;
	return exp((shape - 1) * log(x) - lgamma(shape) - x) / scale;
}