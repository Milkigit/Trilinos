// RAD package (Reverse Automatic Differentiation) --
// a package specialized for function and gradient evaluations.
// Written in 2004 by David M. Gay at Sandia National Labs, Albuquerque, NM.

#ifndef SACADO_RAD_H
#define SACADO_RAD_H

#include <stddef.h>
#include <math.h>

//#ifndef SACADO_NO_NAMESPACE
//namespace Sacado {
//#endif

 class ADvar;
 class ADvari;
 class ADvar1;
 class ADvar2;
 class ConstADvar;
 class Derp;
 class IndepADvar;

#ifndef RAD_AUTO_AD_Const
 struct
AD_IndepVlist {
	AD_IndepVlist *next;
	ADvari *v;
	};
#endif

 struct
ADmemblock {	// We get memory in ADmemblock chunks and never give it back,
		// but reuse it once computations start anew after call(s) on
		// ADcontext::Gradcomp() or ADcontext::Weighted_Gradcomp().
	ADmemblock *next;
	double memblk[1000];
	};

 class
ADcontext {	// A singleton class: one instance in radops.c
	ADmemblock *Busy, *Free;
	char *Mbase;
	size_t Mleft;
	ADmemblock First;
	void *new_ADmemblock(size_t);
 public:
#ifndef RAD_AUTO_AD_Const
	AD_IndepVlist *IVfirst;
	AD_IndepVlist **IVnextp;
	static AD_IndepVlist *AD_Indep_vars();
#endif
	ADcontext();
	void *Memalloc(size_t len);
	static void Gradcomp();
	static void Weighted_Gradcomp(int, ADvar**, double*);
	};

inline void *ADcontext::Memalloc(size_t len) {
		if (Mleft >= len)
			return Mbase + (Mleft -= len);
		return new_ADmemblock(len);
		}

 class
CADcontext: public ADcontext {
 protected:
	bool fpval_implies_const;
 public:
	friend class ADvar;
	CADcontext(): ADcontext() { fpval_implies_const = false; }
	static const double One, negOne;
	};

 class
Derp {		// one derivative-propagation operation
 public:
	friend class ADvarn;
	static Derp *LastDerp;
	Derp *next;
	const double *a;
	ADvari *b;
	ADvari *c;
	void *operator new(size_t);
	void operator delete(void*) {} /*Should never be called.*/
	inline Derp(){};
	Derp(ADvari *);
	Derp(const double *, ADvari *);
	Derp(const double *, ADvari *, ADvari *);
	/* c->aval += a * b->aval; */
	};

inline Derp::Derp(ADvari *c1): c(c1) {
		next = LastDerp;
		LastDerp = this;
		}

inline Derp::Derp(const double *a1, ADvari *c1): a(a1), c(c1) {
		next = LastDerp;
		LastDerp = this;
		}

inline Derp::Derp(const double *a1, ADvari *b1, ADvari *c1): a(a1), b(b1), c(c1) {
		next = LastDerp;
		LastDerp = this;
		}

 class
ADvari {	// implementation of an ADvar
 public:
	double Val;	// result of this operation
	double aval;	// adjoint -- partial of final result w.r.t. this Val
	void *operator new(size_t len) { return ADvari::adc.Memalloc(len); }
	void operator delete(void*) {} /*Should never be called.*/
	inline ADvari(double t): Val(t), aval(0.) {}
	inline ADvari(double t, double ta): Val(t), aval(ta) {}
	inline ADvari(): Val(0.), aval(0.) {}
	static ADcontext adc;
#ifdef RAD_AUTO_AD_Const
	friend class ADcontext;
	friend class ADvar1;
	friend class ADvar;
	friend class ConstADvar;
	friend class IndepADvar;
 private:
	ADvari *Next;
	static ADvari *First_ADvari, **Last_ADvari;
 protected:
	IndepADvar *padv;
 public:
	ADvari(const IndepADvar *, double);
#endif

	friend ADvari&  operator+(ADvari&);
	friend ADvari&  operator-(ADvari&);
	friend ADvari&  operator+ (ADvari&, ADvari&);
	friend ADvari&  operator+ (ADvari&, double);
	friend ADvari&  operator+ (double L, ADvari &R);
	friend ADvari&  operator- (ADvari&, ADvari&);
	friend ADvari&  operator- (ADvari&, double);
	friend ADvari&  operator- (double L, ADvari &R);
	friend ADvari&  operator* (ADvari&, ADvari&);
	friend ADvari&  operator* (ADvari&, double);
	friend ADvari&  operator* (double L, ADvari &R);
	friend ADvari&  operator/ (ADvari&, ADvari&);
	friend ADvari&  operator/ (ADvari&, double);
	friend ADvari&  operator/ (double, ADvari&);
	friend ADvari& atan(ADvari&);
	friend ADvari& atan2(ADvari&, ADvari&);
	friend ADvari& atan2(double, ADvari&);
	friend ADvari& atan2(ADvari&, double);
	friend ADvari& cos (ADvari&);
	friend ADvari& exp (ADvari&);
	friend ADvari& log (ADvari&);
	friend ADvari& pow (ADvari&, ADvari&);
	friend ADvari& pow (double, ADvari&);
	friend ADvari& pow (ADvari&, double);
	friend ADvari& sin (ADvari&);
	friend ADvari& sqrt(ADvari&);
	friend ADvari& tan (ADvari&);
	friend ADvari& fabs(ADvari&);
	friend int operator<(const ADvari&, const ADvari&);
	friend int operator<(const ADvari&, double);
	friend int operator<(double, const ADvari&);
	friend int operator<=(const ADvari&, const ADvari&);
	friend int operator<=(const ADvari&, double);
	friend int operator<=(double,const ADvari&);
	friend int operator==(const ADvari&, const ADvari&);
	friend int operator==(const ADvari&, double);
	friend int operator==(double, const ADvari&);
	friend int operator!=(const ADvari&, const ADvari&);
	friend int operator!=(const ADvari&, double);
	friend int operator!=(double, const ADvari&);
	friend int operator>=(const ADvari&, const ADvari&);
	friend int operator>=(const ADvari&, double);
	friend int operator>=(double, const ADvari&);
	friend int operator>(const ADvari&, const ADvari&);
	friend int operator>(const ADvari&, double);
	friend int operator>(double, const ADvari&);

	friend ADvari& ADf1(double f, double g, ADvari &x);
	friend ADvari& ADf2(double f, double gx, double gy, ADvari &x, ADvari &y);
	friend ADvari& ADfn(double f, int n, const ADvar *x, const double *g);
	};

 inline void* Derp::operator new(size_t len) { return ADvari::adc.Memalloc(len); }


 class
ADvar1: public ADvari {	// simplest unary ops
 public:
	Derp d;
	ADvar1(double val1): ADvari(val1) {}
	ADvar1(double val1, ADvari *c1): d(c1) { Val = val1; }
	ADvar1(double val1, const double *a1, ADvari *c1): d(a1,this,c1) { Val = val1; }
#ifdef RAD_AUTO_AD_Const
	ADvar1(const IndepADvar *, const IndepADvar &);
	ADvar1(const IndepADvar *, ADvari &);
#endif
	};

 class
ConstADvari: public ADvari {
 private:
	ConstADvari *prevcad;
	ConstADvari() {};	// prevent construction without value (?)
	static ConstADvari *lastcad;
 public:
	static CADcontext cadc;
	inline void *operator new(size_t len) { return ConstADvari::cadc.Memalloc(len); }
	inline ConstADvari(double t): ADvari(t) { prevcad = lastcad; lastcad = this; }
	static void aval_reset(void);
	};

 class
IndepADvar
{
 private:
	inline IndepADvar& operator=(const IndepADvar &x) {
		/* private to prevent assignment */
#ifdef RAD_AUTO_AD_Const
		if (cv)
			cv->padv = 0;
		cv = new ADvar1(this,x);
		return *this;
#else
#ifdef RAD_NO_EQ_ALIAS
		return ADvar_operatoreq(this,*x.cv);
#else
		cv = x.cv;
		return *this;
#endif
#endif /* RAD_AUTO_AD_Const */
		}
 protected:
	static void AD_Indep(const IndepADvar&);
	static void AD_Const(const IndepADvar&);
	ADvari *cv;
 public:
	typedef double value_type;
	friend class ADvar;
	friend class ADvar1;
	friend class ADvarn;
	friend class ADcontext;
	IndepADvar(double);
	IndepADvar(int);
	IndepADvar(long);
	IndepADvar& operator=(double);
#ifdef RAD_AUTO_AD_Const
	inline IndepADvar(const IndepADvar &x) { cv = x.cv ? new ADvar1(this, x) : 0; };
	inline IndepADvar() { cv = 0; }
	inline ~IndepADvar() {
			if (cv)
				cv->padv = 0;
		}
#else
	inline IndepADvar() {
#ifdef RAD_NO_EQ_ALIAS
		cv = 0;
#endif
		}
	inline ~IndepADvar() {}
	friend IndepADvar& ADvar_operatoreq(IndepADvar*, const ADvari&);
#endif

	friend void AD_Const(const IndepADvar&);
	friend ADvari& copy(const IndepADvar&);

	inline operator ADvari&() { return *cv; }
	inline operator ADvari&() const { return *(ADvari*)cv; }

	inline double val() const { return cv->Val; }
	inline double adj() const { return cv->aval; }
	static inline void Gradcomp() { ADcontext::Gradcomp(); }
	static inline void aval_reset() { ConstADvari::aval_reset(); }
	static inline void Weighted_Gradcomp(int n, ADvar **v, double *w)
				{ ADcontext::Weighted_Gradcomp(n, v, w); }
	};

 class
ADvar: public IndepADvar {		// an "active" variable
	void ADvar_ctr(double d);
 public:
	inline ADvar() { /* cv = 0; */ }
	inline ADvar(double d) { ADvar_ctr(d); }
	inline ADvar(int i)	{ ADvar_ctr((double)i); }
	inline ADvar(long i)	{ ADvar_ctr((double)i); }
	inline ~ADvar() {}
#ifdef RAD_AUTO_AD_Const
	friend class ADvar1;
	inline ADvar(const IndepADvar &x) { cv = x.cv ? new ADvar1(this, x) : 0; }
	inline ADvar(ADvari &x) { cv = &x; x.padv = this; }
	inline ADvar& operator=(const IndepADvar &x) {
		if (cv)
			cv->padv = 0;
		cv = new ADvar1(this,x);
		return *this;
		}
	inline ADvar& operator=(ADvari &x) {
		if (cv)
			cv->padv = 0;
		cv = new ADvar1(this, x);
		return *this;
		}
#else
	friend ADvar& ADvar_operatoreq(ADvar*, const ADvari&);
	friend void AD_Indep(const ADvar&);
#ifdef RAD_NO_EQ_ALIAS
	ADvar(const IndepADvar &x) { cv = x.cv ? new ADvar1(x.cv->Val, &CADcontext::One, x.cv) : 0; }
	ADvar(ADvari &x) { cv = new ADvar1(x.Val, &CADcontext::One, &x); }
	inline ADvar& operator=(const ADvari &x) { return ADvar_operatoreq(this,x); }
	inline ADvar& operator=(const IndepADvar &x)    { return ADvar_operatoreq(this,*x.cv); }
#else
	/* allow aliasing v and w after "v = w;" */
	inline ADvar(const IndepADvar &x) { cv = x.cv; }
	inline ADvar(ADvari &x) { cv = &x; }
	inline ADvar& operator=(const ADvari &x) { cv = (ADvari*)&x;   return *this; }
	inline ADvar& operator=(const IndepADvar &x)  { cv = (ADvari*)x.cv; return *this; }
#endif /* RAD_NO_EQ_ALIAS */
#endif /* RAD_AUTO_AD_Const */
	ADvar& operator=(double);
	ADvar& operator+=(ADvari&);
	ADvar& operator+=(double);
	ADvar& operator-=(ADvari&);
	ADvar& operator-=(double);
	ADvar& operator*=(ADvari&);
	ADvar& operator*=(double);
	ADvar& operator/=(ADvari&);
	ADvar& operator/=(double);
	inline static bool get_fpval_implies_const(void)
		{ return ConstADvari::cadc.fpval_implies_const; }
	inline static void set_fpval_implies_const(bool newval)
		{ ConstADvari::cadc.fpval_implies_const = newval; }
	inline static bool setget_fpval_implies_const(bool newval) {
		bool oldval = ConstADvari::cadc.fpval_implies_const;
		ConstADvari::cadc.fpval_implies_const = newval;
		return oldval;
		}
	static inline void Gradcomp() { ADcontext::Gradcomp(); }
	static inline void aval_reset() { ConstADvari::aval_reset(); }
	static inline void Weighted_Gradcomp(int n, ADvar **v, double *w)
				{ ADcontext::Weighted_Gradcomp(n, v, w); }
	};

#ifndef RAD_AUTO_AD_Const
 inline void AD_Indep(const ADvar&v) { IndepADvar::AD_Indep(*(const IndepADvar*)&v); }
#endif
 inline void AD_Const(const IndepADvar&v) { IndepADvar::AD_Const(v); }

 class
ConstADvar: public ADvar {
 private: // disable op=
	ConstADvar& operator+=(const ADvari&);
	ConstADvar& operator+=(double);
	ConstADvar& operator-=(const ADvari&);
	ConstADvar& operator-=(double);
	ConstADvar& operator*=(const ADvari&);
	ConstADvar& operator*=(double);
	ConstADvar& operator/=(const ADvari&);
	ConstADvar& operator/=(double);
	void ConstADvar_ctr(double);
 public:
	inline ConstADvar(double d)	{ ConstADvar_ctr(d); }
	inline ConstADvar(int i)	{ ConstADvar_ctr((double)i); }
	inline ConstADvar(long i)	{ ConstADvar_ctr((double)i); }
	ConstADvar(ADvari &x);
#ifdef RAD_AUTO_AD_Const
	ConstADvar(const IndepADvar &x) { cv = new ADvar1(this,x); }
#endif
	inline ~ConstADvar(){}
#ifdef RAD_NO_CONST_UPDATE
 private:
#endif
	ConstADvar();
	inline ConstADvar& operator=(double d) { cv->Val = d; return *this; }
	inline ConstADvar& operator=(const IndepADvar& d) { cv->Val = d.val(); return *this; }
 };

 class
ADvar1s: public ADvar1 { // unary ops with partial "a"
 public:
	double a;
	ADvar1s(double val1, double a1, ADvari *c1): ADvar1(val1,&a,c1), a(a1) {}
	};

 class
ADvar2: public ADvari {	// basic binary ops
 public:
	Derp dL, dR;
	ADvar2(double val1): ADvari(val1) {}
	ADvar2(double val1, ADvari *Lcv, const double *Lc, ADvari *Rcv, const double *Rc):
			ADvari(val1) {
		dR.next = Derp::LastDerp;
		dL.next = &dR;
		Derp::LastDerp = &dL;
		dL.a = Lc;
		dL.c = Lcv;
		dR.a = Rc;
		dR.c = Rcv;
		dL.b = dR.b = this;
		}
	};

 class
ADvar2q: public ADvar2 { // binary ops with partials "a", "b"
 public:
	double a, b;
	ADvar2q(double val1, double Lp, double Rp, ADvari *Lcv, ADvari *Rcv):
			ADvar2(val1), a(Lp), b(Rp) {
		dR.next = Derp::LastDerp;
		dL.next = &dR;
		Derp::LastDerp = &dL;
		dL.a = &a;
		dL.c = Lcv;
		dR.a = &b;
		dR.c = Rcv;
		dL.b = dR.b = this;
		}
	};

 class
ADvarn: public ADvari { // n-ary ops with partials "a"
 public:
	int n;
	double *a;
	Derp *Da;
	ADvarn(double val1, int n1, const ADvar *x, const double *g);
	};

inline ADvari &operator+(ADvari &T) { return T; }

inline int operator<(const ADvari &L, const ADvari &R) { return L.Val < R.Val; }
inline int operator<(const ADvari &L, double R) { return L.Val < R; }
inline int operator<(double L, const ADvari &R) { return L < R.Val; }

inline int operator<=(const ADvari &L, const ADvari &R) { return L.Val <= R.Val; }
inline int operator<=(const ADvari &L, double R) { return L.Val <= R; }
inline int operator<=(double L, const ADvari &R) { return L <= R.Val; }

inline int operator==(const ADvari &L, const ADvari &R) { return L.Val == R.Val; }
inline int operator==(const ADvari &L, double R) { return L.Val == R; }
inline int operator==(double L, const ADvari &R) { return L == R.Val; }

inline int operator!=(const ADvari &L, const ADvari &R) { return L.Val != R.Val; }
inline int operator!=(const ADvari &L, double R) { return L.Val != R; }
inline int operator!=(double L, const ADvari &R) { return L != R.Val; }

inline int operator>=(const ADvari &L, const ADvari &R) { return L.Val >= R.Val; }
inline int operator>=(const ADvari &L, double R) { return L.Val >= R; }
inline int operator>=(double L, const ADvari &R) { return L >= R.Val; }

inline int operator>(const ADvari &L, const ADvari &R) { return L.Val > R.Val; }
inline int operator>(const ADvari &L, double R) { return L.Val > R; }
inline int operator>(double L, const ADvari &R) { return L > R.Val; }

#ifndef RAD_AUTO_AD_Const
inline double Val(const AD_IndepVlist *x) { return x->v->Val; }
inline double Adj(const AD_IndepVlist *x) { return x->v->aval; }
#endif

inline ADvari& copy(const IndepADvar &x)
{ return *(new ADvar1(x.cv->Val, &CADcontext::One, (ADvari*)x.cv)); }

//#ifndef SACADO_NO_NAMESPACE
//} /* namespace Sacado */
//#endif
#endif /* SACADO_RAD_H */
