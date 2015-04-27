#ifndef GEOMETRIES
#define GEOMETRIES

typedef struct _RGBColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBColor;

static RGBColor MakeRGBColor(unsigned char red, unsigned char green, unsigned char blue)
{
    RGBColor c;
    c.red = red;
    c.green = green;
    c.blue = blue;
    return c;
}

typedef struct _GlobalPoint
{
    double x;
    double y;
    double z;
    RGBColor color;
} GlobalPoint;

static GlobalPoint MakeGlobalPoint(double x, double y, double z)
{
    GlobalPoint p;
    p.x = x;
    p.y = y;
    p.z = z;
    return p;
}

typedef struct _GlobalSize
{
    double width;
    double height;
    double depth;
} GlobalSize;

static GlobalSize MakeGlobalSize(double width, double height, double depth)
{
    GlobalSize s;
    s.width = width;
    s.height = height;
    s.depth = depth;
    return s;
}

typedef struct _GlobalLine
{
    double a;
    double b;
} GlobalLine;

static GlobalLine MakeGlobalLine(double a, double b)
{
    GlobalLine l;
    l.a = a;
    l.b = b;
    return l;
}

//points must have same height
static GlobalLine computeLineFromPoints(GlobalPoint p1, GlobalPoint p2)
{
    GlobalLine l;
    l.a = (p2.z-p1.z)/(p2.x-p1.x);
    l.b = p1.z-l.a*p1.x;
    return l;
}

//lines must be on same plane
static GlobalPoint computeIntersectionOfLines(GlobalLine l1, GlobalLine l2)
{
    GlobalPoint i; //intersection of the two coplanar lines
    i.x = (l2.b-l1.b)/(l1.a-l2.a);
    i.z = l2.a*i.x+l2.b;
    return i;
}


#endif // GEOMETRIES

