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

#include "Defn.h"

static SEXP lunary(SEXP, SEXP, SEXP);
static SEXP lbinary(SEXP, SEXP, SEXP);
static SEXP binaryLogic(int code, SEXP s1, SEXP s2);


SEXP do_logic(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP ans;

	if( DispatchGroup("Ops",call, op, args, env, &ans) )
		return ans;

	switch (length(args)) {
	case 1:
		return lunary(call, op, CAR(args));
	case 2:
		return lbinary(call, op, args);
	default:
		error("binary operations require two arguments\n");
	}
}

static SEXP lbinary(SEXP call, SEXP op, SEXP args)
{
	SEXP x, y, dims, tsp, class, xnames, ynames;
	int mismatch, nx, ny, xarray, yarray, xts, yts;

	mismatch = 0;
	x = CAR(args);
	y = CADR(args);

	if (!isNumeric(x) || !isNumeric(y))
		errorcall(call, "operations are possible only for numeric or logical types\n");

	xarray = isArray(x);
	yarray = isArray(y);
	xts = isTs(x);
	yts = isTs(y);
	if (xarray || yarray) {
		if (xarray && yarray) {
			if (!conformable(x, y))
				error("binary operation non-conformable arrays\n");
			PROTECT(dims = getAttrib(x, R_DimSymbol));
		}
		else if (xarray) {
			PROTECT(dims = getAttrib(x, R_DimSymbol));
		}
		else if (yarray) {
			PROTECT(dims = getAttrib(y, R_DimSymbol));
		}
		PROTECT(xnames = getAttrib(x, R_DimNamesSymbol));
		PROTECT(ynames = getAttrib(y, R_DimNamesSymbol));
	}
	else {
		nx = length(x);
		ny = length(y);
		if(nx > 0 && ny > 0) {
			if(nx > ny) mismatch = nx % ny;
			else mismatch = ny % nx;
		}
		PROTECT(dims = R_NilValue);
		PROTECT(xnames = getAttrib(x, R_NamesSymbol));
		PROTECT(ynames = getAttrib(y, R_NamesSymbol));
	}
	if (xts || yts) {
		if (xts && yts) {
			if (!tsConform(x, y))
				errorcall(call, "Non-conformable time-series\n");
			PROTECT(tsp = getAttrib(x, R_TspSymbol));
			PROTECT(class = getAttrib(x, R_ClassSymbol));
		}
		else if (xts) {
			if (length(x) < length(y))
				ErrorMessage(call, ERROR_TSVEC_MISMATCH);
			PROTECT(tsp = getAttrib(x, R_TspSymbol));
			PROTECT(class = getAttrib(x, R_ClassSymbol));
		}
		else if (yts) {
			if (length(y) < length(x))
				ErrorMessage(call, ERROR_TSVEC_MISMATCH);
			PROTECT(tsp = getAttrib(y, R_TspSymbol));
			PROTECT(class = getAttrib(y, R_ClassSymbol));
		}
	}
	if(mismatch) warningcall(call, "longer object length\n\tis not a multiple of shorter object length\n");

	x = CAR(args) = coerceVector(x, LGLSXP);
	y = CADR(args) = coerceVector(y, LGLSXP);
	PROTECT(x = binaryLogic(PRIMVAL(op), x, y));

	if (xts || yts) {
		setAttrib(x, R_TspSymbol, tsp);
		setAttrib(x, R_ClassSymbol, class);
		UNPROTECT(2);
	}

	if (dims != R_NilValue) {
		setAttrib(x, R_DimSymbol, dims);
		if(xnames != R_NilValue)
			setAttrib(x, R_DimNamesSymbol, xnames);
		else if(ynames != R_NilValue)
			setAttrib(x, R_DimNamesSymbol, ynames);
	}
	else {
		if(length(x) == length(xnames))
			setAttrib(x, R_NamesSymbol, xnames);
		else if(length(x) == length(ynames))
			setAttrib(x, R_NamesSymbol, ynames);
	}
	UNPROTECT(4);
	return x;
}

static SEXP lunary(SEXP call, SEXP op, SEXP arg)
{
	SEXP x, dim, dimnames, names;
	int i, len;

	if (TYPEOF(arg) != LGLSXP)
		error("unary ! is only defined for logical vectors\n");
	len = LENGTH(arg);
	PROTECT(names = getAttrib(arg, R_NamesSymbol));
	PROTECT(dim = getAttrib(arg, R_DimSymbol));
	PROTECT(dimnames = getAttrib(arg, R_DimNamesSymbol));
	PROTECT(x = allocVector(LGLSXP, len));
	for (i = 0; i < len; i++)
		LOGICAL(x)[i] = (LOGICAL(arg)[i] == NA_LOGICAL) ?
			NA_LOGICAL : LOGICAL(arg)[i] == 0;
	if(names != R_NilValue) setAttrib(x, R_NamesSymbol, names);
	if(dim != R_NilValue) setAttrib(x, R_DimSymbol, dim);
	if(dimnames != R_NilValue) setAttrib(x, R_DimNamesSymbol, dimnames);
	UNPROTECT(4);
	return x;
}

SEXP do_logic2(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP s1, s2;
	float x1, x2;
	SEXP ans;

	if (length(args) != 2)
		error("binary &&/|| requires 2 arguments\n");

	s1 = CAR(args);
	s2 = CADR(args);
	PROTECT(ans = allocVector(LGLSXP, 1));

	switch (PRIMVAL(op)) {
	case 1:
		s1 = eval(s1, env);
		if (!isNumeric(s1))
			error("binary operator applied to invalid types\n");
		if ((x1 = asLogical(s1)) == NA_LOGICAL)
			error("missing value where logical needed\n");
		if (x1) {
			s2 = eval(s2, env);
			if (!isNumeric(s2))
				error("binary operator applied to invalid types\n");
			if ((x2 = asLogical(s2)) == NA_LOGICAL)
				error("missing value where logical needed\n");
			LOGICAL(ans)[0] = x2;
		}
		else
			LOGICAL(ans)[0] = x1;
		UNPROTECT(1);
		return ans;
	case 2:
		s1 = eval(s1, env);
		if (!isNumeric(s1))
			error("binary operator applied to invalid types\n");
		if ((x1 = asLogical(s1)) == NA_LOGICAL)
			error("missing value where logical needed\n");
		if (!x1) {
			s2 = eval(s2, env);
			if (!isNumeric(s2))
				error("binary operator applied to invalid types\n");
			if ((x2 = asLogical(s2)) == NA_LOGICAL)
				error("missing value where logical needed\n");
			LOGICAL(ans)[0] = x2;
		}
		else
			LOGICAL(ans)[0] = x1;
		UNPROTECT(1);
		return ans;
	}
	/*NOTREACHED*/
}

static SEXP binaryLogic(int code, SEXP s1, SEXP s2)
{
	int i, n, n1, n2;
	int x1, x2;
	SEXP ans;

	n1 = LENGTH(s1);
	n2 = LENGTH(s2);
	n = (n1 > n2) ? n1 : n2;
	if ( n1 == 0 || n2 == 0 ) {
		ans = allocVector(LGLSXP, 0);
		return ans;
	}
	ans = allocVector(LGLSXP, n);

	switch (code) {
	case 1:		/* AND */
		for (i = 0; i < n; i++) {
			x1 = LOGICAL(s1)[i % n1];
			x2 = LOGICAL(s2)[i % n2];
			if (x1 == 0 || x2 == 0)
				LOGICAL(ans)[i] = 0;
			else if (x1 == NA_LOGICAL || x2 == NA_LOGICAL)
				LOGICAL(ans)[i] = NA_LOGICAL;
			else
				LOGICAL(ans)[i] = 1;
		}
		break;
	case 2:		/* OR */
		for (i = 0; i < n; i++) {
			x1 = LOGICAL(s1)[i % n1];
			x2 = LOGICAL(s2)[i % n2];
			if ((x1 != NA_LOGICAL && x1) || (x2 != NA_LOGICAL && x2))
				LOGICAL(ans)[i] = 1;
			else if (x1 == 0 && x2 == 0)
				LOGICAL(ans)[i] = 0;
			else
				LOGICAL(ans)[i] = NA_LOGICAL;
		}
		break;
	}
	return ans;
}

static void checkValues(int *, int);
static int haveTrue;
static int haveFalse;
static int haveNA;

SEXP do_logic3(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP ans, s, t;
	int count, narm;

	if(DispatchGroup("Summary", call, op, args, env, &s))
		return s;

	ans = matchArg(R_NaRmSymbol, &args);
	narm = asLogical(ans);
	haveTrue = 0;
	haveFalse = 0;
	haveNA = 0;
	count = 0;

	for (s = args; s != R_NilValue; s = CDR(s)) {
		t = CAR(s);
		if (LGLSXP <= TYPEOF(t) && TYPEOF(t) <= CPLXSXP) {
			/* coerceVector protects its argument */
			/* so this actually works just fine */
			t = coerceVector(t, LGLSXP);
			checkValues(LOGICAL(t), LENGTH(t));
			count = count + LENGTH(t);
		}
		else if(!isNull(t))
			errorcall(call, "incorrect argument type\n");
	}
	s = allocVector(LGLSXP, 1L);
	if(narm) haveNA = 0;
	if (PRIMVAL(op) == 1) {				/* ALL */
		if (count == 0)
			LOGICAL(s)[0] = 1;
		else if (haveTrue && !haveFalse && !haveNA)
			LOGICAL(s)[0] = 1;
		else if (haveFalse)
			LOGICAL(s)[0] = 0;
		else
			LOGICAL(s)[0] = NA_LOGICAL;
	}
	else {						/* ANY */
		if (count == 0)
			LOGICAL(s)[0] = 0;
		else if (haveTrue)
			LOGICAL(s)[0] = 1;
		else if (haveFalse && !haveTrue && !haveNA)
			LOGICAL(s)[0] = 0;
		else
			LOGICAL(s)[0] = NA_LOGICAL;
	}
	return s;
}

static void checkValues(int * x, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		if (x[i] == NA_LOGICAL)
			haveNA = 1;
		else if (x[i])
			haveTrue = 1;
		else
			haveFalse = 1;
	}
}