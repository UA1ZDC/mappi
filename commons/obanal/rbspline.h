#ifndef rbspline_h
#define rbspline_h
#include <qpoint.h>
#include <qpolygon.h>
/*  Subroutine to generate a rational B-spline curve using an uniform open knot vector

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: rbspline.c
	Language: C
	Subroutines called: knot.c, rbasis.c, fmtmul.c
	Book reference: Chapter 4, Alg. p. 297

    b[]         = array containing the defining polygon vertices
                  b[1] contains the x-component of the vertex
                  b[2] contains the y-component of the vertex
                  b[3] contains the z-component of the vertex
	h[]			= array containing the homogeneous weighting factors 
    k           = order of the B-spline basis function
    nbasis      = array containing the basis functions for a single value of t
    nplusc      = number of knot values
    npts        = number of defining polygon vertices
    p[,]        = array containing the curve points
                  p[1] contains the x-component of the point
                  p[2] contains the y-component of the point
                  p[3] contains the z-component of the point
    p1          = number of points to be calculated on the curve
    t           = parameter value 0 <= t <= npts - k + 1
    x[]         = array containing the knot vector
*/
void rbasis(int c,float t,int npts,int *x,float *h,float *r);

void rbspline(int npts,int k, int p1, const QPolygonF &b,float *h,float *p, int );
void rbspline(int npts,int k, int p1, float *b,float *h,float *p, int );
void knot(int n, int c,int *x);

void calcspline( const QPolygonF &vxy, QPolygonF *res, double d);



#endif
