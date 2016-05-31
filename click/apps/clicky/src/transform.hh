#ifndef LCDF_TRANSFORM_HH
#define LCDF_TRANSFORM_HH
#include "rectangle.hh"
#include <click/string.hh>

class affine { public:

    affine();
    affine(const double[6]);
    affine(double scalex, double sheary, double shearx, double scaley, double shiftx, double shifty);
    // affine(const affine &)		generated by compiler
    // ~affine()			generated by compiler

    static affine mapping(const point &from1, const point &to1,
			  const point &from2, const point &to2);

    double operator[](int i) const	{ assert(i>=0&&i<6); return _m[i]; }
    bool null() const			{ return _null; }
    void check_null(double tolerance);

    void scale(double, double);
    void scale(const point &p)			{ scale(p.x(), p.y()); }
    void scale(double d)			{ scale(d, d); }
    void rotate(double);
    void translate(double, double);
    void translate(const point &p)		{ translate(p.x(), p.y()); }
    void shear(double);
    inline void transform(const affine &);

    inline affine scaled(double, double) const;
    affine scaled(const point &p) const		{ return scaled(p.x(), p.y()); }
    affine scaled(double d) const		{ return scaled(d, d); }
    inline affine rotated(double) const;
    inline affine translated(double, double) const;
    inline affine translated(const point &p) const;
    inline affine sheared(double) const;
    affine transformed(const affine &) const;

    // affine operator+(affine, const point &);
    // affine &operator+=(affine &, const point &);
    // affine operator*(affine, double);
    // affine &operator*=(affine &, double);
    // affine operator*(affine, const affine &);
    // affine &operator*=(affine &, const affine &);
    friend point operator*(const affine &, const point &);
    friend point &operator*=(point &, const affine &);
    // friend Bezier operator*(const Bezier &, const affine &);
    // friend Bezier &operator*=(Bezier &, const affine &);

    String unparse() const;

  private:

    // stored in PostScript order (along columns)
    double _m[6];
    bool _null;

    void real_apply_to(point &) const;
    point real_apply(const point &) const;

};


inline affine
affine::scaled(double x, double y) const
{
    affine t(*this);
    t.scale(x, y);
    return t;
}

inline affine
affine::rotated(double r) const
{
    affine t(*this);
    t.rotate(r);
    return t;
}

inline affine
affine::translated(double x, double y) const
{
    affine t(*this);
    t.translate(x, y);
    return t;
}

inline affine
affine::translated(const point &p) const
{
    return translated(p.x(), p.y());
}

inline affine
affine::sheared(double s) const
{
    affine t(*this);
    t.shear(s);
    return t;
}


inline affine &
operator+=(affine &t, const point &p)
{
    t.translate(p);
    return t;
}

inline affine
operator+(affine t, const point &p)
{
    return t += p;
}

inline affine &
operator*=(affine &t, double scale)
{
    t.scale(scale);
    return t;
}

inline affine
operator*(affine t, double scale)
{
    return t *= scale;
}

inline affine
operator*(const affine &t, const affine &tt)
{
    return t.transformed(tt);
}

inline affine &
operator*=(affine &t, const affine &tt)
{
    t = t.transformed(tt);
    return t;
}

inline void
affine::transform(const affine &t)
{
    *this *= t;
}


inline point &
operator*=(point &p, const affine &t)
{
    if (!t.null())
	t.real_apply_to(p);
    return p;
}

inline point
operator*(const affine &t, const point &p)
{
    return (t.null() ? p : t.real_apply(p));
}

#endif