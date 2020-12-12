#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext.h"
#include "vecdraw.h"

#define SVG_DS 2

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

SVGNodeType SVGParseNodeType(const char* tag)
{
    if (strcmp(tag, "?xml"))
        return XMLDOC;
    else if (strcmp(tag, "svg"))
        return SVGROOT;
    else if (strcmp(tag, "defs"))
        return SVG_DEFS;
    else if (strcmp(tag, "use"))
        return SVG_USE;
    else if (strcmp(tag, "linearGradient")) {
        assert(!"Not implemented.");
        return SVG_LINEAR_GRADIENT;
    }
    else if (strcmp(tag, "radialGradient")) {
        assert(!"Not implemented.");
        return SVG_RADIAL_GRADIENT;
    }
    else if (strcmp(tag, "g"))
        return SVG_G;
    else if (strcmp(tag, "style")) {
        assert(!"Not implemented.");
        return SVG_STYLE;
    }
    else if (strcmp(tag, "path"))
        return SVG_PATH;
    else if (strcmp(tag, "rect"))
        return SVG_RECT;
    else if (strcmp(tag, "circle"))
        return SVG_CIRCLE;
    else if (strcmp(tag, "ellipse"))
        return SVG_ELLIPSE;
    else
        return XMLTAG;
}

typedef struct _SVGPathGroup {
    const char*           id;
    struct _SVGGroupPath* children;
    struct _SVGGroup*     next;
} SVGPathGroup;

// typedef struct {
//     double x, y;
//     double w, h;
// } SVGRect;

// typedef struct {
//     double cx, cy;
//     double r;
// } SVGCircle;

// typedef struct {
//     double cx, cy;
//     double r;
// } SVGCircle;

typedef struct {
    const char* key;
    const char* val;
} XMLAttribute;

int XMLAttributeComp(const void* p1, const void* p2)
{
    return strcmp(((const XMLAttribute*)p1)->key, ((const XMLAttribute*)p2)->key);
}

typedef struct _XMLNode {
    SVGNodeType      type;
    const char*      tagName;
    RBTree*          attributes;
    void*            value;
    struct _XMLNode* children;
    struct _XMLNode* next;
} XMLNode;

// returns 1 if tag has body
int XMLParseAttributes(char* buf, int* j, XMLNode* node)
{
    int i = 0;
    while (1) {
        while (buf[i] == ' ')
            i++;
        if (buf[i] == '/' || buf[i] == '>') break;
        XMLAttribute* attrib = (XMLAttribute*)malloc(sizeof(XMLAttribute));
        attrib->key          = buf + i;
        while (isalpha(buf[i]) || buf[i] == ':')
            i++;
        buf[i++] = '\0';
        while (buf[i] != ' ' || buf[i] != '=')
            i++;
        assert(buf[i++] == '\"');
        attrib->val = buf + i;
        while (buf[i] != '\"')
            i++;
        buf[i++] = '\0';
        RBTreeInsert(node->attributes, attrib);
    }
    if (strncmp(buf + i, "/>", 2) == 0) {
        *j = i + 2;
        return 0;
    }
    *j += i + 1;
    return 1;
}

int XMLParseEndTag(char* buf, int* j)
{
    int i = 0;
    while (buf[i] == ' ')
        i++;
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

typedef enum {
    SVG_TRANSFORM_ROTATE,
    SVG_TRANSFORM_SCALE,
    SVG_TRANSFORM_TRANSLATE,
    SVG_TRANSFORM_SKEWX,
    SVG_TRANSFORM_SKEWY,
    SVG_TRANSFORM_MATRIX
} SVGTransformType;

TransformMat SVGRotate1Mat(double deg) { return rotateOrginMat(-toRad(deg)); }

TransformMat SVGRotate3Mat(double deg, double cx, double cy) { return rotateMat(-toRad(deg), {cx, cy}); }

TransformMat SVGScale1Mat(double s) { return scaleMat({s, s}); }

TransformMat SVGScale2Mat(double sx, double sy) { return scaleMat({sx, sy}); }

TransformMat SVGTranslateMat(double dx, double dy) { return translateMat({dx, dy}); }

TransformMat SVGSkewXMat(double degx) { return skewMat(-toRad(degx), 0); }
TransformMat SVGSkewYMat(double degy) { return skewMat(0, -toRad(degy)); }

XMLAttribute* XMLFindAttribute(XMLNode* xmlnode, const char* key)
{
    XMLAttribute key  = {.key = key};
    RBNode*      attr = RBTreeFind(xmlnode->attributes, &key);
    if (attr == RBNull)
        return NULL;
    else
        return RBPointer(attr, XMLAttribute);
}

TransformMat SVGParseTransform(XMLNode* node)
{
    XMLAttribute* attr = XMLFindAttribute(node, "transform");
    if (attr == NULL) return identity();
    const char* str = attr->val;
    int         i   = 0;
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
        while (str[i] == ' ' || str[i] == ',')
            i++;
        args[k++] = strtod(str + i, &endp);
        i         = endp - str;
        while (str[i] == ' ' || str[i] == ',')
            i++;
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
            TransformMat mat;
            mat.mat[0][0] = args[0];
            mat.mat[1][0] = args[1];
            mat.mat[0][1] = args[2];
            mat.mat[1][1] = args[3];
            mat.mat[0][2] = args[4];
            mat.mat[1][2] = args[5];
            return mat;
        }
    }
    assert(!"wtf");
    return identity();
}

XMLNode* XMLParseNode(char* buf, int* j)
{
    int i = 0;
    while (buf[i] == ' ')
        i++;
    assert(buf[i++] == '<');
    while (buf[i] == ' ')
        i++;
    XMLNode* node    = (XMLNode*)malloc(sizeof(XMLNode));
    node->attributes = createRBTree(XMLAttributeComp);
    node->value = node->children = node->next = NULL;
    node->tagName                             = buf + i;
    while (isalpha(buf[i]))
        i++;
    buf[i++]   = '\0';
    node->type = SVGParseNodeType(node->tagName);
    if (XMLParseAttributes(buf + i, &i, node)) {
        XMLNode* next_node = node;
        while (!XMLParseEndTag(buf + i, &i)) {
            next_node->next = XMLParseNode(buf + i, &i);
            next_node       = next_node->next;
        }
    }
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
    RBTreeFree(node->attributes);
    if (node->value) free(node->value);
    free(node);
}

SVGPathGroup* SVGParse(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buf = (char*)malloc(fileSize);
    fread(buf, fileSize, 1, file);
    fclose(file);

    SVGPathGroup* g = (SVGPathGroup*)malloc(sizeof(SVGPathGroup));
    g->children     = NULL;
    g->children     = NULL;
    int      i      = 0;
    XMLNode* root   = XMLParseNode(buf, &i);
    assert(stricmp(root->tagName, "svg"));
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
    pointVectorPush(points, p);
    return p;
}
Point SVGLineTo(PointVector* points, Point p2, int rel)
{
    Point p1 = pointVectorBack(points);
    if (rel) p2 = add(p1, p2);
    pointVectorPush(points, p2);
    return p2;
}
Point SVGHLineTo(PointVector* points, double x2, int rel)
{
    Point p1 = pointVectorBack(points);
    Point p2 = {x2, p1.y};
    if (rel) p2.x += p1.x;
    pointVectorPush(points, p2);
    return p2;
}
Point SVGVLineTo(PointVector* points, double y2, int rel)
{
    Point p1 = pointVectorBack(points);
    Point p2 = {p1.x, y2};
    if (rel) p2.y += p1.y;
    pointVectorPush(points, p2);
    return p2;
}
void SVGCubicBezier(PointVector* points, Point p1, Point c1, Point c2, Point p2)
{
    double S  = norm(sub(c1, p1)) + norm(sub(c2, c1)) + norm(sub(p2, c2));
    double N  = (S / SVG_DS);
    double dt = 1 / N;
    Point  p;
    for (double t = 0; t < 1.0; t += dt) {
        p = mul(p1, (1 - t) * (1 - t) * (1 - t));
        p = add(p, mul(c1, 3 * (1 - t) * (1 - t) * t));
        p = add(p, mul(c2, 3 * (1 - t) * t * t));
        p = add(p, mul(p2, t * t * t));
        pointVectorPush(points, p);
    }
    pointVectorPush(points, p2);
}

void SVGQuadraticBezier(PointVector* points, Point p1, Point c, Point p2)
{
    double S  = norm(sub(c, p1)) + norm(sub(p2, c));
    double N  = (S / SVG_DS);
    double dt = 1 / N;
    Point  p;
    for (double t = 0; t < 1.0; t += dt) {
        p = mul(p1, (1 - t) * (1 - t));
        p = add(p, mul(c, 2 * (1 - t) * t));
        p = add(p, mul(p2, t * t));
        pointVectorPush(points, p);
    }
    pointVectorPush(points, p2);
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
    pointVectorPush(points, p1);
    for (double t = t1 + dt; t < t2; t += dt) {
        p.x = r.x * cos(t);
        p.y = r.y * sin(t);
        p   = add(rotate(p, rad, {0, 0}), c);
        pointVectorPush(points, p);
    }
    pointVectorPush(points, p2);
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
    Fill          fill = {.fill = 0};
    XMLAttribute* attr;
    if ((attr = XMLFindAttribute(pathNode, "fill")) != NULL) {
        if (strcmp(attr->val, "none") != 0) {
            fill.fill  = 1;
            fill.color = SVGParseColor(attr->val);
            if ((attr = XMLFindAttribute(pathNode, "fill-opacity")) != NULL) fill.opacity = strtod(attr->val, NULL);
        }
    }
    return fill;
}

Stroke SVGParseStroke(XMLNode* pathNode)
{
    Stroke        stroke = {.width = 0};
    XMLAttribute* attr;
    if ((attr = XMLFindAttribute(pathNode, "stroke")) != NULL) {
        if (strcmp(attr->val, "none") != 0) {
            stroke.color = SVGParseColor(attr->val);
            if ((attr = XMLFindAttribute(pathNode, "stroke-width")) != NULL) stroke.width = strtod(attr->val, NULL);
            if ((attr = XMLFindAttribute(pathNode, "stroke-opacity")) != NULL) stroke.opacity = strtod(attr->val, NULL);
        }
    }
    return stroke;
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
    int          k, n_args;
    double       args[8];
    char*        endp;
    Point        lp;
    while (cmd_str[i]) {
        while (cmd_str[i] && cmd_str[i] == ' ' || cmd_str[i] == ',')
            i++;
        if (!cmd_str[i]) break;
        if (isalpha(cmd_str[i])) {
            rel  = islower(cmd_str[i]);
            lcmd = toupper(cmd_str[i]);
            i++;
        }
        if (lcmd == 'Z') break;
        k = 0;
        while (cmd_str[i]) {
            while (cmd_str[i] && (cmd_str[i] == ' ' || cmd_str[i] == ','))
                i++;
            if (!cmd_str[i] || isalpha(cmd_str[i])) break;
            args[k++] = strtod(cmd_str + i, &endp);
            i         = endp - cmd_str;
        }
        n_args = k;
        switch (lcmd) {
            case 'M':
                assert(n_args == 2);
                lp = SVGMoveTo(points, {args[0], args[1]});
                break;
            case 'L':
                assert(n_args == 2);
                lp = SVGLineTo(points, {args[0], args[1]}, rel);
                break;
            case 'H':
                assert(n_args == 1);
                lp = SVGHLineTo(points, args[0], rel);
                break;
            case 'V':
                assert(n_args == 1);
                lp = SVGVLineTo(points, args[0], rel);
                break;
            case 'C':
                assert(n_args == 6);
                lp = SVGCubicBezierTo(points, {args[0], args[1]}, {args[2], args[3]}, {args[4], args[5]}, rel);
                break;
            case 'S':
                assert(n_args == 4);
                lp = SVGCubicBezierTo(points, lp, {args[0], args[1]}, {args[2], args[3]}, rel);
                break;
            case 'Q':
                assert(n_args == 4);
                lp = SVGQuadraticBezierTo(points, {args[0], args[1]}, {args[2], args[3]}, rel);
                break;
            case 'T':
                assert(n_args == 2);
                lp = SVGQuadraticBezierTo(points, lp, {args[0], args[1]}, rel);
                break;
            case 'A':
                assert(n_args == 7);
                lp = SVGArcTo(
                    points, args[0], args[1], args[2], args[3] >= 0.5, args[4] >= 0.5, {args[5], args[6]}, rel);
                break;
        }
    }
    Path* path = createPath(points->p, points->n, SVGParseFill(pathNode), SVGParseStroke(pathNode), closed);
    free(points);
    return path;
}