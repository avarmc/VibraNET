// VectorT.h: interface for the Vector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTORT_H__15A180A0_FD4A_11D4_8C92_00C0DFF689EF__INCLUDED_)
#define AFX_VECTORT_H__15A180A0_FD4A_11D4_8C92_00C0DFF689EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template <class T>
class Vector3  
{
public:
	typedef Vector3 <T> Type;
	typedef T * pType;
protected:
	static float _sin(float f) { return sinf(f); }
	static float _cos(float f) { return cosf(f); }
	static double _sin(double f) { return sin(f); }
	static double _cos(double f) { return cos(f); }
public:
	T x,y,z;
	Vector3(){x=y=z=0; };
	Vector3 (T v) { x=y=z=v; };
	Vector3 (const Type & v) { x=v.x;y=v.y;z=v.z; };
	Vector3 (T vx,T vy,T vz) { x=vx,y=vy,z=vz; };
	~Vector3() {};
	
	const Type& operator = (const Type& v) { x=v.x;y=v.y;z=v.z; return *this; }
	const Type& operator = (T f) { x=y=z=f; return *this; }
	const Type  operator - () const { return Type(-x,-y,-z); };
	const Type& operator +=(const Type& v) {x+=v.x;y+=v.y;z+=v.z;return *this;};
	const Type& operator -=(const Type& v) {x-=v.x;y-=v.y;z-=v.z;return *this;};
	const Type& operator *=(const Type& v) {x*=v.x;y*=v.y;z*=v.z;return *this;};
	const Type& operator *=(T a) {x*=a;y*=a;z*=a;return *this;};
	const Type& operator /=(T a) {x/=a;y/=a;z/=a;return *this;};;
	
	friend Type operator + (const Type& u,const Type& v)
	{return Type(u.x+v.x,u.y+v.y,u.z+v.z);};
	friend Type operator - (const Type& u,const Type& v)
	{return Type(u.x-v.x,u.y-v.y,u.z-v.z);};
	friend Type operator * (const Type& u,const Type& v)
	{return Type(u.x*v.x,u.y*v.y,u.z*v.z);};
	friend Type operator / (const Type& u,const Type& v)
	{return Type(u.x/v.x,u.y/v.y,u.z/v.z);};
	friend Type operator * (const Type& u,const T a)
	{return Type(u.x*a,u.y*a,u.z*a);};
	friend Type operator / (const Type& u,const T a)
	{return Type(u.x/a,u.y/a,u.z/a);};
	friend T operator & (const Type& u,const Type& v)
	{ return u.x*v.x+u.y*v.y+u.z*v.z;};
	friend Type operator ^ (const Type& u,const Type& v)
	{return Type(u.y*v.z-u.z*v.y,u.z*v.x-u.x*v.z,u.x*v.y-u.y*v.x);};

	friend bool operator == (const Type& u,const Type& v)
	{return (u.x == v.x && u.y==v.y && u.z==v.z);};

	friend bool operator != (const Type& u,const Type& v)
	{return (u.x != v.x || u.y!=v.y || u.z!=v.z);};

	T operator ! ()  { return (T)sqrt(x*x+y*y+z*z);};
	T& operator [](int n)             { return *(&x+n); };

	friend static void TurnVector(const T& a,const Type& e,Type& t)
	{
		if(abs(e.x) == 1.0) TurnVectorX(a,e,t); else
		if(abs(e.y) == 1.0) TurnVectorA(a,e,t); else
		if(abs(e.z) == 1.0) TurnVectorA(a,e,t); else
					TurnVectorA(a,e,t);
	}

	friend static void TurnVectorA(const T& a,const Type& e,Type& t)
	{
		Type v;
		T Cos=_cos(a),Sin=_sin(a);
		T m[3][3]= 
		{
			{
				e.x*e.x+Cos*(1-e.x*e.x),
					e.x*(1-Cos)*e.y+e.z*Sin,
					e.x*(1-Cos)*e.z-e.y*Sin
			},
			{
				e.x*(1-Cos)*e.y-e.z*Sin,
					e.y*e.y+Cos*(1-e.y*e.y),
					e.y*e.z*(1-Cos)+e.x*Sin
			},
				{
					e.x*(1-Cos)*e.z+e.y*Sin,
						e.y*e.z*(1-Cos)-e.x*Sin,
						e.z*e.z+Cos*(1-e.z*e.z)
				}
		};

		////////////////////// X'=M*X ////////////////////////

		v.x=(m[0][0]*t.x+m[0][1]*t.y+m[0][2]*t.z),
			v.y=(m[1][0]*t.x+m[1][1]*t.y+m[1][2]*t.z),
			v.z=(m[2][0]*t.x+m[2][1]*t.y+m[2][2]*t.z);
		t=v;

	}

	friend static void TurnVectorX(const T& a,const Type& e,Type& t)
	{
		Type v;
		T Cos=_cos(a),Sin=_sin(a);
		v.x=((e.x*e.x+Cos*(1-e.x*e.x))*t.x),
			v.y=(Cos*t.y+e.x*Sin*t.z),
			v.z=((-e.x*Sin)*t.y+Cos*t.z);
		t=v;
	}

	friend static void TurnVectorY(const T& a,const Type& e,Type& t)
	{
		Type v;

		T Cos=_cos(a),Sin=_sin(a);

		v.x=(Cos*t.x-e.y*Sin*t.z),
			v.y=((e.y*e.y+Cos*(1-e.y*e.y))*t.y),
			v.z=(e.y*Sin*t.x+Cos*t.z);
		t=v;
	}
	
	friend static void TurnVectorZ(const T& a,const Type& e,Type& t)
	{
		Type v;

		T Cos=_cos(a),Sin=_sin(a);

		T m[3][3]= 
		{
			{
				Cos,
				e.z*Sin,
					0
			},
			{
				-e.z*Sin,
				Cos,
				0
			},
				{
					0,
					0,
					e.z*e.z+Cos*(1-e.z*e.z)
				}
		};

		////////////////////// X'=M*X ////////////////////////

		v.x=(Cos*t.x+e.z*Sin*t.y),
			v.y=(-e.z*Sin*t.x+Cos*t.y),
			v.z=((e.z*e.z+Cos*(1-e.z*e.z))*t.z);
		t=v;
	}
};




typedef Vector3 <float> Vector3f;
typedef Vector3 <double> Vector3d;
typedef Vector3 <int> Vector3i;





template <class T>
class Vector2
 {
public:
	typedef Vector2 <T> Type;
	typedef T * pType;
 protected:
	static float _sin(float f) { return sinf(f); }
	static float _cos(float f) { return cosf(f); }
	static double _sin(double f) { return sin(f); }
	static double _cos(double f) { return cos(f); }
public:
   T x,y;
   Vector2(){x=y=0; };
   Vector2 (T v) { x=y=v; };
   Vector2 (const Vector2& v) { x=v.x;y=v.y; };
   Vector2 (T vx,T vy) { x=vx,y=vy; };
   
   Type& operator = (const Type& v) { x=v.x;y=v.y; return *this; }
   Type& operator = (T f) { x=y=f; return *this; }
   Type  operator - () const { return Type(-x,-y); };

	const Type& operator +=(const Type& v) {x+=v.x; y+=v.y; return (*this);};
	const Type& operator -=(const Type& v) {x-=v.x; y-=v.y; return (*this);};
	const Type& operator *=(const Type& v) {x*=v.x; y*=v.y; return (*this);};
	const Type& operator *=(T a) {x*=a; y*=a; return *this;};
	const Type& operator /=(T a) {x/=a;y/=a; return *this;};;

	friend Type operator + (const Type& u,const Type& v)
	{return Type(u.x+v.x,u.y+v.y);};
	friend Type operator - (const Type& u,const Type& v)
	{return Type(u.x-v.x,u.y-v.y);};
	friend Type operator * (const Type& u,const Type& v)
	{return Type(u.x*v.x,u.y*v.y);};
	friend Type operator / (const Type& u,const Type& v)
	{return Type(u.x/v.x,u.y/v.y);};
	friend Type operator * (const Type& u,const T a)
	{return Type(u.x*a,u.y*a);};
	friend Type operator / (const Type& u,const T a)
	{return Type(u.x/a,u.y/a);};

   friend T operator & (const Type& u,const Type& v)
                           { return u.x*v.x+u.y*v.y;};

   friend bool operator == (const Type& u,const Type& v)
   {return (u.x == v.x && u.y==v.y);};

   friend bool operator != (const Type& u,const Type& v)
   {return (u.x != v.x || u.y!=v.y);};

   T operator ! ()  { return (T)sqrt(x*x+y*y);};

   T& operator [](int n)             { return *(&x+n); };
   int operator < (T v)    { return x<v && y<v ; };
   int operator > (T v)    { return x>v && y>v ; };

   int operator < (const Type& v) { return (y<v.y || (y==v.y && x<v.x))?1:0; };
   int operator > (const Type& v) { return (y>v.y || (y==v.y && x>v.x))?1:0; };

   	Type Turn(const T& a)
	{
		T _sina = _sin(a), _cosa = _cos(a);
		return Type(_cosa*x-_sina*y,_sina*x+_cosa*y);
	}

 };

typedef Vector2 <float> Vector2f;
typedef Vector2 <double> Vector2d;
typedef Vector2 <int> Vector2i;





#endif // !defined(AFX_VECTORT_H__15A180A0_FD4A_11D4_8C92_00C0DFF689EF__INCLUDED_)
