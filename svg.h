#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vecdraw.h"

#define SVG_DS 2

typedef enum {
    SVG_TRANSFORM_ROTATE,
    SVG_TRANSFORM_SCALE,
    SVG_TRANSFORM_TRANSLATE,
    SVG_TRANSFORM_SKEWX,
    SVG_TRANSFORM_SKEWY,
    SVG_TRANSFORM_MATRIX
} SVGTransformType;

TransformMat SVGRotate1Mat(double deg);
TransformMat SVGRotate3Mat(double deg, double cx, double cy);
TransformMat SVGScale1Mat(double s);
TransformMat SVGScale2Mat(double sx, double sy);
TransformMat SVGTranslateMat(double dx, double dy);
TransformMat SVGSkewXMat(double degx);
TransformMat SVGSkewYMat(double degy);

typedef enum { CSS_CLASS, CSS_ID, CSS_TAG } CSSSelectionType;
typedef enum { CSS_FILL, CSS_LINEAR_GRADIENT, CSS_RADIAL_GRADIENT } CSSBackgroundType;
typedef enum {
    XMLDOC,
    XMLTAG,
    SVGROOT,
    SVG_DEFS,
    SVG_USE,
    SVG_LINEAR_GRADIENT,
    SVG_RADIAL_GRADIENT,
    SVG_G,
    SVG_STYLE,
    SVG_PATH,
    SVG_RECT,
    SVG_CIRCLE,
    SVG_ELLIPSE
} SVGNodeType;

SVGNodeType SVGParseNodeType(const char* tag);

typedef struct _SVGPathGroup {
    const char*           id;
    struct _SVGGroupPath* children;
    struct _SVGGroup*     next;
} SVGPathGroup;

typedef struct {
    const char* key;
    const char* val;
} XMLAttribute;

typedef struct _XMLNode {
    SVGNodeType      type;
    const char*      tagName;
    RBTree*          attributes;
    void*            value;
    struct _XMLNode* children;
    struct _XMLNode* next;
} XMLNode;

int           XMLAttributeComp(const void* p1, const void* p2);
int           XMLParseAttributes(char* buf, int* j, XMLNode* node);
int           XMLParseEndTag(char* buf, int* j);
XMLAttribute* XMLFindAttribute(XMLNode* xmlnode, const char* key);
XMLNode*      XMLParseNode(char* buf, int* j);
void          XMLFreeNode(XMLNode* node);

SVGPathGroup* SVGParse(const char* filePath);
TransformMat  SVGParseTransform(const char* str);
Color         SVGParseColor(const char* str);
Fill          SVGParseFill(XMLNode* pathNode);
Stroke        SVGParseStroke(XMLNode* pathNode);
Point         SVGMoveTo(PointVector* points, Point p);
Point         SVGLineTo(PointVector* points, Point p2, int rel);
Point         SVGHLineTo(PointVector* points, double x2, int rel);
Point         SVGVLineTo(PointVector* points, double y2, int rel);
void          SVGCubicBezier(PointVector* points, Point p1, Point c1, Point c2, Point p2);
void          SVGQuadraticBezier(PointVector* points, Point p1, Point c, Point p2);
Point         SVGCubicBezierTo(PointVector* points, Point c1, Point c2, Point p2, int rel);
Point         SVGQuadraticBezierTo(PointVector* points, Point c, Point p2, int rel);
double        SVGEllipseParameter(Point c, Point r, double rad, Point p);
void          SVGEllipseArc(PointVector* points, Point c, Point r, double rad, Point p1, Point p2, int laf);
void          SVGArc(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p1, Point p2);
Point         SVGArcTo(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p2, int rel);
Path*         SVGParsePath(XMLNode* pathNode);
Path*         SVGParseRect(XMLNode* node);
Path*         SVGParseCircle(XMLNode* node);
Path*         SVGParseEllipse(XMLNode* node);