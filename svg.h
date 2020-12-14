#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vecdraw.h"

#define SVG_DS 5

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

typedef struct {
    const char* key;
    const char* val;
} XMLAttribute;

typedef struct _XMLNode {
    const char*      id;
    SVGNodeType      type;
    const char*      tagName;
    RBTree*          attributes;
    void*            value;
    struct _XMLNode* parent;
    struct _XMLNode* children;
    struct _XMLNode* next;
} XMLNode;

typedef struct {
    double x, y, w, h;
} ViewBox;

int           XMLAttributeComp(const void* p1, const void* p2);
int           XMLNodeComp(const void* p1, const void* p2);
int           XMLParseAttributes(char* buf, int* j, XMLNode* node);
int           XMLParseEndTag(char* buf, int* j);
XMLAttribute* XMLFindAttribute(XMLNode* xmlnode, const char* key);
XMLNode*      XMLParseNode(XMLNode* parent, RBTree* idd, char* buf, int* j);
void          XMLFreeNode(XMLNode* node);

TransformMat SVGParseTransform(const char* str);
Color        SVGParseColor(const char* str);
Fill         SVGParseFill(XMLNode* pathNode);
Stroke       SVGParseStroke(XMLNode* pathNode);
Point        SVGMoveTo(PointVector* points, Point p);
Point        SVGLineTo(PointVector* points, Point p2, int rel);
Point        SVGHLineTo(PointVector* points, double x2, int rel);
Point        SVGVLineTo(PointVector* points, double y2, int rel);
void         SVGCubicBezier(PointVector* points, Point p1, Point c1, Point c2, Point p2);
void         SVGQuadraticBezier(PointVector* points, Point p1, Point c, Point p2);
Point        SVGCubicBezierTo(PointVector* points, Point c1, Point c2, Point p2, int rel);
Point        SVGQuadraticBezierTo(PointVector* points, Point c, Point p2, int rel);
double       SVGEllipseParameter(Point c, Point r, double rad, Point p);
void         SVGEllipseArc(PointVector* points, Point c, Point r, double rad, Point p1, Point p2, int laf);
void         SVGArc(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p1, Point p2);
Point        SVGArcTo(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p2, int rel);
Path*        SVGParsePath(XMLNode* pathNode);
Path*        SVGParseRect(XMLNode* node);
Path*        SVGParseCircle(XMLNode* node);
Path*        SVGParseEllipse(XMLNode* node);
void         SVGAddPoint(PointVector* points, Point p);
ViewBox      SVGParseViewBox(XMLNode* svgRoot);

typedef struct _SVGPathGroup {
    const char*           id;
    int                   isGroup, hidden;
    struct _SVGPathGroup* parent;
    void*                 child;
    struct _SVGPathGroup* next;
} SVGPathGroup;

SVGPathGroup* SVGParse(const char* filePath);
XMLNode*      SVGGetUseRef(XMLNode* node, RBTree* idd);
SVGPathGroup* SVGParseGroup(SVGPathGroup* parent, RBTree* idd, XMLNode* node, TransformMat mat);

void printfew(const char* buf, int n)
{
    for (int i = 0; i < n; i++)
        putchar(buf[i]);
    printf("\n");
}

int matchAny(char c, const char* match)
{
    int m = 0;
    for (int j = 0; match[j]; j++)
        if (m = match[j] == c) break;
    return m;
}

void skipSpaces(const char* buf, int* j, const char* ad)
{
    int i = 0;
    do {
        switch (buf[i]) {
            case ' ':
            case '\t':
            case '\n': continue;
        }
        if (!matchAny(buf[i], ad)) break;
    } while (buf[++i]);
    *j += i;
}

SVGNodeType SVGParseNodeType(const char* tag)
{
    if (strcmp(tag, "?xml") == 0)
        return XMLDOC;
    else if (strcmp(tag, "svg") == 0)
        return SVGROOT;
    else if (strcmp(tag, "defs") == 0)
        return SVG_DEFS;
    else if (strcmp(tag, "use") == 0)
        return SVG_USE;
    else if (strcmp(tag, "linearGradient") == 0) {
        assert(!"Not implemented.");
        return SVG_LINEAR_GRADIENT;
    }
    else if (strcmp(tag, "radialGradient") == 0) {
        assert(!"Not implemented.");
        return SVG_RADIAL_GRADIENT;
    }
    else if (strcmp(tag, "g") == 0)
        return SVG_G;
    else if (strcmp(tag, "style") == 0) {
        assert(!"Not implemented.");
        return SVG_STYLE;
    }
    else if (strcmp(tag, "path") == 0)
        return SVG_PATH;
    else if (strcmp(tag, "rect") == 0)
        return SVG_RECT;
    else if (strcmp(tag, "circle") == 0)
        return SVG_CIRCLE;
    else if (strcmp(tag, "ellipse") == 0)
        return SVG_ELLIPSE;
    else
        return XMLTAG;
}

int XMLAttributeComp(const void* p1, const void* p2)
{
    return strcmp(((const XMLAttribute*)p1)->key, ((const XMLAttribute*)p2)->key);
}

// returns 1 if tag has body
int XMLParseAttributes(char* buf, int* j, XMLNode* node)
{
    int i = 0;
    while (1) {
        skipSpaces(buf + i, &i, "");
        if (buf[i] == '/' || buf[i] == '>') break;
        XMLAttribute* attrib = (XMLAttribute*)malloc(sizeof(XMLAttribute));
        attrib->key          = buf + i;
        while (isalpha(buf[i]) || matchAny(buf[i], ":-"))
            i++;
        // printfew(buf + i, 10);
        assert(matchAny(buf[i], " \t\n="));
        buf[i++] = '\0';
        assert(buf[i++] == '\"');
        attrib->val = buf + i;
        while (buf[i] != '\"')
            i++;
        assert(buf[i] == '\"');
        buf[i++] = '\0';
        RBTreeInsert(node->attributes, attrib);
    }
    if (strncmp(buf + i, "/>", 2) == 0) {
        *j += i + 2;
        return 0;
    }
    *j += i + 1;
    return 1;
}

int XMLParseEndTag(char* buf, int* j)
{
    int i = 0;
    skipSpaces(buf + i, &i, "");
    if (strncmp(buf + i, "</", 2) != 0) {
        *j += i;
        return 0;
    }
    i += 2;
    while (buf[i] != '>')
        i++;
    *j += i + 1;
    return 1;
}

TransformMat SVGRotate1Mat(double deg) { return rotateOrginMat(-toRad(deg)); }

TransformMat SVGRotate3Mat(double deg, double cx, double cy) { return rotateMat(-toRad(deg), {cx, cy}); }

TransformMat SVGScale1Mat(double s) { return scaleMat({s, s}); }

TransformMat SVGScale2Mat(double sx, double sy) { return scaleMat({sx, sy}); }

TransformMat SVGTranslateMat(double dx, double dy) { return translateMat({dx, dy}); }

TransformMat SVGSkewXMat(double degx) { return skewMat(-toRad(degx), 0); }
TransformMat SVGSkewYMat(double degy) { return skewMat(0, -toRad(degy)); }

XMLAttribute* XMLFindAttribute(XMLNode* xmlnode, const char* key)
{
    XMLAttribute key_node = {.key = key};
    RBNode*      attr     = RBTreeFind(xmlnode->attributes, &key_node);
    if (attr == RBNull)
        return NULL;
    else
        return RBPointer(attr, XMLAttribute);
}

TransformMat SVGParseTransform(const char* str)
{
    int i = 0;
    while (str[i] == ' ')
        i++;
    double           args[6];
    int              n_args;
    SVGTransformType type;
    if (strncmp(str, "rotate", 6) == 0)
        type = SVG_TRANSFORM_ROTATE, i += 6;
    else if (strncmp(str, "scale", 5) == 0)
        type = SVG_TRANSFORM_SCALE, i += 5;
    else if (strncmp(str, "translate", 9) == 0)
        type = SVG_TRANSFORM_TRANSLATE, i += 9;
    else if (strncmp(str, "skewX", 5) == 0)
        type = SVG_TRANSFORM_SKEWX, i += 5;
    else if (strncmp(str, "skewY", 5) == 0)
        type = SVG_TRANSFORM_SKEWY, i += 5;
    else if (strncmp(str, "matrix", 6) == 0)
        type = SVG_TRANSFORM_MATRIX, i += 6;
    else
        assert(!"Unknown transformation");
    assert(str[i++] == '(');
    char* endp;
    int   k = 0;
    while (str[i] != ')') {
        skipSpaces(str + i, &i, ",");
        args[k++] = strtod(str + i, &endp);
        i         = endp - str;
        skipSpaces(str + i, &i, ",");
    }
    n_args = k;
    switch (type) {
        case SVG_TRANSFORM_ROTATE:
            assert(n_args == 1 || n_args == 3);
            if (n_args == 1)
                return SVGRotate1Mat(args[0]);
            else
                return SVGRotate3Mat(args[0], args[1], args[2]);
        case SVG_TRANSFORM_SCALE:
            assert(n_args == 1 || n_args == 2);
            if (n_args == 1)
                return SVGScale1Mat(args[0]);
            else
                return SVGScale2Mat(args[0], args[1]);
        case SVG_TRANSFORM_TRANSLATE: assert(n_args == 2); return SVGTranslateMat(args[0], args[1]);
        case SVG_TRANSFORM_SKEWX: assert(n_args == 1); return SVGSkewXMat(args[0]);
        case SVG_TRANSFORM_SKEWY: assert(n_args == 1); return SVGSkewYMat(args[0]);
        case SVG_TRANSFORM_MATRIX: {
            assert(n_args == 6);
            TransformMat mat = identity();
            mat.mat[0][0]    = args[0];
            mat.mat[1][0]    = args[1];
            mat.mat[0][1]    = args[2];
            mat.mat[1][1]    = args[3];
            mat.mat[0][2]    = args[4];
            mat.mat[1][2]    = args[5];
            return mat;
        }
    }
    assert(!"wtf");
    return identity();
}

int XMLNodeComp(const void* p1, const void* p2)
{
    const XMLNode *n1 = (const XMLNode*)p1, *n2 = (const XMLNode*)p2;
    return strcmp(n1->id, n2->id);
}

XMLNode* XMLParseNode(XMLNode* parent, RBTree* idd, char* buf, int* j)
{
    int i = 0;
    skipSpaces(buf + i, &i, "");
    assert(buf[i++] == '<');
    skipSpaces(buf + i, &i, "");
    XMLNode* node    = (XMLNode*)malloc(sizeof(XMLNode));
    node->attributes = createRBTree(XMLAttributeComp);
    node->parent     = parent;
    node->value = node->children = node->next = NULL;
    node->tagName                             = buf + i;
    while (isalpha(buf[i]))
        i++;
    assert(matchAny(buf[i], " \t\n=/>"));
    int no_attr = buf[i] == '>';
    buf[i++]    = '\0';
    node->type  = SVGParseNodeType(node->tagName);
    if (no_attr || (XMLParseAttributes(buf + i, &i, node) && !no_attr)) {
        XMLNode *child = NULL, *next_child;
        while (!XMLParseEndTag(buf + i, &i)) {
            next_child = XMLParseNode(node, idd, buf + i, &i);
            if (child)
                child->next = next_child;
            else
                node->children = next_child;
            child = next_child;
        }
    }
    XMLAttribute* idattr = XMLFindAttribute(node, "id");
    if (idattr) {
        node->id = idattr->val;
        RBTreeInsert(idd, node);
    }
    else
        node->id = "";
    switch (node->type) {
        case SVG_PATH: node->value = SVGParsePath(node); break;
        case SVG_RECT: node->value = SVGParseRect(node); break;
        case SVG_CIRCLE: node->value = SVGParseCircle(node); break;
        case SVG_ELLIPSE: node->value = SVGParseEllipse(node); break;
    }
    *j += i;
    return node;
}

void XMLFreeNode(XMLNode* node)
{
    XMLNode *y, *z = node->children;
    while (z) {
        y = z;
        z = z->next;
        XMLFreeNode(y);
    }
    RBTreeFree(node->attributes, 1);
    if (node->value) free(node->value);
    free(node);
}

SVGPathGroup* SVGParse(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    assert(file && "File doesn't exit");
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buf = (char*)malloc(fileSize);
    fread(buf, fileSize, 1, file);
    fclose(file);
    int      i    = 0;
    RBTree*  idd  = createRBTree(XMLNodeComp);
    XMLNode* root = XMLParseNode(NULL, idd, buf, &i);
    assert(strcmp(root->tagName, "svg") == 0);
    SVGPathGroup* ret = SVGParseGroup(NULL, idd, root, identity());
    XMLFreeNode(root);
    RBTreeFree(idd, 0);
    free(buf);
    return ret;
}

int hexToNum(char hex) { return isdigit(hex) ? hex - '0' : tolower(hex) - 'a' + 10; }

Color SVGParseColor(const char* str)
{
    unsigned char r = 0, g = 0, b = 0;
    if (!strcmp(str, "aqua"))
        r = 0, g = 255, b = 255;
    else if (!strcmp(str, "black"))
        r = 0, g = 0, b = 0;
    else if (!strcmp(str, "blue"))
        r = 0, g = 0, b = 255;
    else if (!strcmp(str, "fuchsia"))
        r = 255, g = 0, b = 255;
    else if (!strcmp(str, "gray"))
        r = 128, g = 128, b = 128;
    else if (!strcmp(str, "green"))
        r = 0, g = 128, b = 0;
    else if (!strcmp(str, "lime"))
        r = 0, g = 255, b = 0;
    else if (!strcmp(str, "maroon"))
        r = 128, g = 0, b = 0;
    else if (!strcmp(str, "navy"))
        r = 0, g = 0, b = 128;
    else if (!strcmp(str, "olive"))
        r = 128, g = 128, b = 0;
    else if (!strcmp(str, "purple"))
        r = 128, g = 0, b = 128;
    else if (!strcmp(str, "red"))
        r = 255, g = 0, b = 0;
    else if (!strcmp(str, "silver"))
        r = 192, g = 192, b = 192;
    else if (!strcmp(str, "teal"))
        r = 0, g = 128, b = 128;
    else if (!strcmp(str, "white"))
        r = 255, g = 255, b = 255;
    else if (!strcmp(str, "yellow"))
        r = 255, g = 255, b = 0;
    else {
        assert(str[0] == '#' && "Only hex implemented");
        assert(strlen(str) >= 7);
        r = hexToNum(str[1]) * 16 + hexToNum(str[2]);
        g = hexToNum(str[3]) * 16 + hexToNum(str[4]);
        b = hexToNum(str[5]) * 16 + hexToNum(str[6]);
    }
    Color c = {.r = r, .g = g, .b = b};
    return c;
}

Point SVGMoveTo(PointVector* points, Point p)
{
    SVGAddPoint(points, p);
    return p;
}
Point SVGLineTo(PointVector* points, Point p2, int rel)
{
    Point p1 = pointVectorBack(points);
    if (rel) p2 = add(p1, p2);
    SVGAddPoint(points, p2);
    return p2;
}
Point SVGHLineTo(PointVector* points, double x2, int rel)
{
    Point p1 = pointVectorBack(points);
    Point p2 = {x2, p1.y};
    if (rel) p2.x += p1.x;
    SVGAddPoint(points, p2);
    return p2;
}
Point SVGVLineTo(PointVector* points, double y2, int rel)
{
    Point p1 = pointVectorBack(points);
    Point p2 = {p1.x, y2};
    if (rel) p2.y += p1.y;
    SVGAddPoint(points, p2);
    return p2;
}
void SVGCubicBezier(PointVector* points, Point p1, Point c1, Point c2, Point p2)
{
    double S  = norm(sub(c1, p1)) + norm(sub(c2, c1)) + norm(sub(p2, c2));
    double N  = (S / SVG_DS);
    double dt = 1 / N;
    Point  p;
    for (double t = dt; t < 1.0; t += dt) {
        p = mul(p1, (1 - t) * (1 - t) * (1 - t));
        p = add(p, mul(c1, 3 * (1 - t) * (1 - t) * t));
        p = add(p, mul(c2, 3 * (1 - t) * t * t));
        p = add(p, mul(p2, t * t * t));
        SVGAddPoint(points, p);
    }
    SVGAddPoint(points, p2);
}

void SVGQuadraticBezier(PointVector* points, Point p1, Point c, Point p2)
{
    double S  = norm(sub(c, p1)) + norm(sub(p2, c));
    double N  = (S / SVG_DS);
    double dt = 1 / N;
    Point  p;
    for (double t = dt; t < 1.0; t += dt) {
        p = mul(p1, (1 - t) * (1 - t));
        p = add(p, mul(c, 2 * (1 - t) * t));
        p = add(p, mul(p2, t * t));
        SVGAddPoint(points, p);
    }
    SVGAddPoint(points, p2);
}

Point SVGCubicBezierTo(PointVector* points, Point c1, Point c2, Point p2, int rel)
{
    Point p1 = pointVectorBack(points);
    if (rel) {
        c1 = add(p1, c1);
        c2 = add(p1, c2);
        p2 = add(p1, p2);
    }
    SVGCubicBezier(points, p1, c1, c2, p2);
    return sub(p2, sub(c2, p2));
}
Point SVGQuadraticBezierTo(PointVector* points, Point c, Point p2, int rel)
{
    Point p1 = pointVectorBack(points);
    if (rel) {
        c  = add(p1, c);
        p2 = add(p1, p2);
    }
    SVGQuadraticBezier(points, p1, c, p2);
    return sub(p2, sub(c, p2));
}
double SVGEllipseParameter(Point c, Point r, double rad, Point p)
{
    double x = ((p.y - c.y) * cos(rad) - (p.x - c.x) * sin(rad)) / r.y;
    if (x > 1.0) x = 1.0;
    if (x < -1.0) x = -1.0;
    double t[2];
    t[0] = asin(x), t[1] = PI - t[0];
    t[0] = fmod(t[0] + 2 * PI, 2 * PI);
    t[1] = fmod(t[1] + 2 * PI, 2 * PI);
    Point p_;
    for (int i = 0; i < 2; i++) {
        p_.x = r.x * cos(t[i]);
        p_.y = r.y * sin(t[i]);
        p_   = add(rotate(p_, rad, {0, 0}), c);
        if (norm(sub(p, p_)) < 1e-5) return t[i];
    }
    assert(!"Point not on ellipse");
    return 0.0;
}
void SVGEllipseArc(PointVector* points, Point c, Point r, double rad, Point p1, Point p2, int laf)
{
    double dt = SVG_DS / min(r.x, r.y);
    double t1 = SVGEllipseParameter(c, r, rad, p1), t2 = SVGEllipseParameter(c, r, rad, p2), t1_, t2_;
    t1_ = min(t1, t2), t2_ = max(t1, t2);
    laf = 2 * laf - 1;
    if (laf * (t2_ - t1_) < laf * PI)
        t1 = t2_, t2 = t1_ + 2 * PI;
    else
        t1 = t1_, t2 = t2_;
    Point p;
    for (double t = t1 + dt; t < t2; t += dt) {
        p.x = r.x * cos(t);
        p.y = r.y * sin(t);
        p   = add(rotate(p, rad, {0, 0}), c);
        SVGAddPoint(points, p);
    }
    SVGAddPoint(points, p2);
}
void SVGArc(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p1, Point p2)
{
    static int ff  = 1;
    double     rad = -deg * PI / 180;
    Point      p1_ = rotate(p1, -rad, {0, 0});
    Point      p2_ = rotate(p2, -rad, {0, 0});
    Point      p0  = sub(p2_, p1_);
    double     a = rx, b = ry;
    double     x0 = p0.x;
    double     y0 = p0.y;
    double     m, c;
    int        f = 1;
    do {
        m = -y0 * a * a / (x0 * b * b);
        c = (x0 * x0 * b * b + y0 * y0 * a * a) / (2 * x0 * b * b);
        if (f)
            f = 0;
        else {
            a += 0.1;
            b += (b / a) * 0.1;
        }
    } while ((b * b * m * m + a * a) < c * c);
    Interval sol = solveQuadratic(b * b * m * m + a * a, 2 * m * c * b * b, b * b * (c * c - a * a));
    Point    c1  = {m * sol.l + c, sol.l};
    Point    c2  = {m * sol.r + c, sol.r};
    c1           = rotate(add(c1, p1_), rad, {0, 0});
    c2           = rotate(add(c2, p1_), rad, {0, 0});
    if ((sf && signedArea(p1, p2, c1) >= 0) || (!sf && signedArea(p1, p2, c1) < 0))
        SVGEllipseArc(points, c1, {a, b}, rad, p1, p2, laf);
    else
        SVGEllipseArc(points, c2, {a, b}, rad, p1, p2, laf);
}

Point SVGArcTo(PointVector* points, double rx, double ry, double deg, int laf, int sf, Point p2, int rel)
{
    Point p1 = pointVectorBack(points);
    if (rel) p2 = add(p1, p2);
    SVGArc(points, rx, ry, deg, laf, sf, p1, p2);
    return p2;
}

Fill SVGParseFill(XMLNode* pathNode)
{
    Fill fill;
    fill.fill    = 0;
    fill.opacity = 1;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(pathNode, "fill")) {
        if (strcmp(attr->val, "none") != 0) {
            fill.fill  = 1;
            fill.color = SVGParseColor(attr->val);
            if (attr = XMLFindAttribute(pathNode, "fill-opacity")) fill.opacity = strtod(attr->val, NULL);
        }
    }
    return fill;
}

Stroke SVGParseStroke(XMLNode* pathNode)
{
    Stroke stroke;
    stroke.width   = 0;
    stroke.opacity = 1;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(pathNode, "stroke")) {
        if (strcmp(attr->val, "none") != 0) {
            stroke.color = SVGParseColor(attr->val);
            if (attr = XMLFindAttribute(pathNode, "stroke-width")) stroke.width = strtod(attr->val, NULL);
            if (attr = XMLFindAttribute(pathNode, "stroke-opacity")) stroke.opacity = strtod(attr->val, NULL);
        }
    }
    return stroke;
}

int parseNArgs(const char* cmd_str, int* j, double args[], int n)
{
    int   i = 0;
    int   k = 0;
    char* endp;
    while (cmd_str[i] && k < n) {
        skipSpaces(cmd_str + i, &i, ",");
        if (!cmd_str[i] || isalpha(cmd_str[i])) break;
        args[k++] = strtod(cmd_str + i, &endp);
        i         = endp - cmd_str;
    }
    *j += i;
    return k;
}

Path* SVGParsePath(XMLNode* pathNode)
{
    XMLAttribute* attr = XMLFindAttribute(pathNode, "d");
    assert(attr);
    PointVector* points  = createPointVector();
    int          closed  = 1;
    const char*  cmd_str = attr->val;
    char         lcmd    = 'L';
    int          rel     = 0;
    int          i       = 0;
    double       args[8];
    char*        endp;
    Point        lp;
    while (cmd_str[i]) {
        skipSpaces(cmd_str + i, &i, ",");
        if (!cmd_str[i]) break;
        if (isalpha(cmd_str[i])) {
            rel  = islower(cmd_str[i]);
            lcmd = toupper(cmd_str[i]);
            i++;
        }
        if (lcmd == 'Z') {
            closed = 1;
            break;
        }
        switch (lcmd) {
            case 'M':
                assert(parseNArgs(cmd_str + i, &i, args, 2) == 2);
                lp = SVGMoveTo(points, {args[0], args[1]});
                break;
            case 'L':
                assert(parseNArgs(cmd_str + i, &i, args, 2) == 2);
                lp = SVGLineTo(points, {args[0], args[1]}, rel);
                break;
            case 'H':
                assert(parseNArgs(cmd_str + i, &i, args, 1) == 1);
                lp = SVGHLineTo(points, args[0], rel);
                break;
            case 'V':
                assert(parseNArgs(cmd_str + i, &i, args, 1) == 1);
                lp = SVGVLineTo(points, args[0], rel);
                break;
            case 'C':
                assert(parseNArgs(cmd_str + i, &i, args, 6) == 6);
                lp = SVGCubicBezierTo(points, {args[0], args[1]}, {args[2], args[3]}, {args[4], args[5]}, rel);
                break;
            case 'S':
                assert(parseNArgs(cmd_str + i, &i, args, 4) == 4);
                lp = SVGCubicBezierTo(points, lp, {args[0], args[1]}, {args[2], args[3]}, rel);
                break;
            case 'Q':
                assert(parseNArgs(cmd_str + i, &i, args, 4) == 4);
                lp = SVGQuadraticBezierTo(points, {args[0], args[1]}, {args[2], args[3]}, rel);
                break;
            case 'T':
                assert(parseNArgs(cmd_str + i, &i, args, 4) == 2);
                lp = SVGQuadraticBezierTo(points, lp, {args[0], args[1]}, rel);
                break;
            case 'A':
                assert(parseNArgs(cmd_str + i, &i, args, 7) == 7);
                lp = SVGArcTo(
                    points, args[0], args[1], args[2], args[3] >= 0.5, args[4] >= 0.5, {args[5], args[6]}, rel);
                break;
        }
    }
    Path* path = createPath(points->p, points->n, SVGParseFill(pathNode), SVGParseStroke(pathNode), closed);
    if (attr = XMLFindAttribute(pathNode, "transform")) applyLocalTransform(SVGParseTransform(attr->val), path);
    free(points);
    return path;
}

Path* SVGParseRect(XMLNode* node)
{
    assert(node->type == SVG_RECT);
    double        x = 0, y = 0, w = 0, h = 0;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(node, "x")) x = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "y")) y = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "width")) w = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "height")) h = strtod(attr->val, NULL);
    Point* p   = (Point*)malloc(sizeof(Point) * 4);
    p[0]       = {x, y};
    p[1]       = {x + w, y};
    p[2]       = {x + w, y + h};
    p[3]       = {x, y + h};
    Path* path = createPath(p, 4, SVGParseFill(node), SVGParseStroke(node), 1);
    if (attr = XMLFindAttribute(node, "transform")) applyLocalTransform(SVGParseTransform(attr->val), path);
    return path;
}

Path* SVGParseCircle(XMLNode* node)
{
    assert(node->type == SVG_CIRCLE);
    double        cx = 0, cy = 0, r = 0;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(node, "r")) r = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "cx")) cx = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "cy")) cy = strtod(attr->val, NULL);
    int    N  = 2 * PI * r / SVG_DS;
    double dt = 2 * PI / N;
    Point* p  = (Point*)malloc(sizeof(Point) * N);
    double t  = 0.0;
    for (int i = 0; i < N; t += dt, i++) {
        p[i].x = r * cos(t) + cx;
        p[i].y = r * sin(t) + cy;
    }
    Path* path = createPath(p, N, SVGParseFill(node), SVGParseStroke(node), 1);
    if (attr = XMLFindAttribute(node, "transform")) applyLocalTransform(SVGParseTransform(attr->val), path);
    return path;
}

Path* SVGParseEllipse(XMLNode* node)
{
    assert(node->type == SVG_ELLIPSE);
    double        cx = 0, cy = 0, rx = 0, ry = 0;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(node, "rx")) rx = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "ry")) ry = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "cx")) cx = strtod(attr->val, NULL);
    if (attr = XMLFindAttribute(node, "cy")) cy = strtod(attr->val, NULL);
    int    N  = 2 * PI * max(rx, ry) / SVG_DS;
    double dt = 2 * PI / N;
    Point* p  = (Point*)malloc(sizeof(Point) * N);
    double t  = 0.0;
    for (int i = 0; i < N; t += dt, i++) {
        p[i].x = rx * cos(t) + cx;
        p[i].y = ry * sin(t) + cy;
    }
    Path* path = createPath(p, N, SVGParseFill(node), SVGParseStroke(node), 1);
    if (attr = XMLFindAttribute(node, "transform")) applyLocalTransform(SVGParseTransform(attr->val), path);
    return path;
}

XMLNode* SVGGetUseRef(XMLNode* node, RBTree* idd)
{
    assert(node->type == SVG_USE);
    XMLAttribute* href = XMLFindAttribute(node, "xlink:href");
    assert(href);
    assert(href->val[0] == '#');
    XMLNode key = {.id = href->val + 1};
    RBNode* n   = RBTreeFind(idd, &key);
    assert(n != RBNull);
    if (n != RBNull)
        return RBPointer(n, XMLNode);
    else
        return NULL;
}

SVGPathGroup* SVGParseGroup(SVGPathGroup* parent, RBTree* idd, XMLNode* node, TransformMat mat)
{
    assert(node->type != SVG_USE);
    SVGPathGroup* g = (SVGPathGroup*)malloc(sizeof(SVGPathGroup));
    g->id           = strdup(node->id);
    g->parent       = parent;
    g->child = g->next = NULL;
    g->isGroup         = node->type == SVG_G || node->type == SVGROOT; // treat root as a group
    g->hidden          = 0;
    XMLAttribute* attr;
    if (attr = XMLFindAttribute(node, "transform")) mat = matMul(mat, SVGParseTransform(attr->val));
    if (g->isGroup) {
        SVGPathGroup *child = NULL, *next_child;
        for (XMLNode *x = node->children, *y; x; x = x->next) {
            switch (x->type) {
                case SVG_G:
                case SVG_PATH:
                case SVG_RECT:
                case SVG_CIRCLE:
                case SVG_ELLIPSE: y = x; break;
                case SVG_USE:
                    y = SVGGetUseRef(x, idd);
                    if (y == NULL) continue;
                    break;
                default: continue;
            }
            next_child = SVGParseGroup(g, idd, y, mat);
            if (child)
                child->next = next_child;
            else
                g->child = next_child;
            child = next_child;
        }
    }
    else {
        switch (node->type) {
            case SVG_PATH:
            case SVG_RECT:
            case SVG_CIRCLE:
            case SVG_ELLIPSE: {
                Path* path = (Path*)node->value;
                path       = duplicatePath(path);
                applyLocalTransform(mat, path);
                g->child = path;
            } break;
        }
    }
    return g;
}

void renderSVGPathGroup(SVGPathGroup* g, TransformMat mat)
{
    if (!g->hidden) {
        if (g->isGroup) {
            for (SVGPathGroup* c = (SVGPathGroup*)g->child; c; c = c->next)
                renderSVGPathGroup(c, mat);
        }
        else if (g->child) {
            renderPath((Path*)g->child, mat);
        }
    }
}

void SVGAddPoint(PointVector* points, Point p)
{
    if (points->n == 0 || norm(sub(pointVectorBack(points), p)) > (SVG_DS / 10.0)) pointVectorPush(points, p);
}

ViewBox SVGParseViewBox(XMLNode* svgRoot)
{
    XMLAttribute* attr = XMLFindAttribute(svgRoot, "viewBox");
    int           i    = 0;
    double        args[4];
    assert(parseNArgs(attr->val, &i, args, 4) == 4);
    ViewBox vb;
    vb.x = args[0];
    vb.y = args[1];
    vb.w = args[2];
    vb.h = args[3];
    return vb;
}