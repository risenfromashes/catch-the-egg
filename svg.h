#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext.h"
#include "vecdraw.h"

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

SVGNodeType parseNodeType(const char* tag)
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
        assert("Not implemented.");
        return SVG_LINEAR_GRADIENT;
    }
    else if (strcmp(tag, "radialGradient")) {
        assert("Not implemented.");
        return SVG_RADIAL_GRADIENT;
    }
    else if (strcmp(tag, "g"))
        return SVG_G;
    else if (strcmp(tag, "style")) {
        assert("Not implemented.");
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

typedef struct {
    CSSSelectionType type;
    const char*      key;
} CSSSelector;

typedef struct {
    // CSSSelector selector;
    Fill   fill;
    Stroke stroke;
} Style;

typedef struct _SVGPathGroup {
    CSSSelector           selector;
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

int CSSSelectionComp(const void* p1, const void* p2)
{
    const CSSSelector *s1 = (const CSSSelector*)p1, *s2 = (const CSSSelector*)p2;
    if (s1->type != s1->type) return strcmp(s1->key, s2->key);
    return s1->type - s2->type;
}

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
int parseAttributes(char* buf, int* j, XMLNode* node)
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

int parseEndTag(char* buf, int* j)
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

TransformMat parseTransform(const char* str)
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
        assert("Unknown transformation");
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
    assert("wtf");
    return identity();
}

XMLAttribute* findAttribute(XMLNode* xmlnode, const char* key)
{
    XMLAttribute key  = {.key = key};
    RBNode*      attr = RBTreeFind(xmlnode->attributes, &key);
    if (attr == RBNull)
        return NULL;
    else
        return RBPointer(attr, XMLAttribute);
}

XMLNode* parseNode(char* buf, int* j)
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
    node->type = parseNodeType(node->tagName);
    if (parseAttributes(buf + i, &i, node)) {
        XMLNode* next_node = node;
        while (!parseEndTag(buf + i, &i)) {
            next_node->next = parseNode(buf + i, &i);
            next_node       = next_node->next;
        }
    }
    return node;
}

SVGPathGroup* parseSVG(const char* filePath)
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
    XMLNode* root   = parseNode(buf, &i);
    assert(stricmp(root->tagName, "svg"));
}

int hexToNum(char hex) { return isdigit(hex) ? hex - '0' : tolower(hex) - 'a' + 10; }

Color parseColor(const char* str)
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

Path* parsePath(XMLNode* pathNode)
{
    XMLAttribute* attr = findAttribute(pathNode, "d");
    assert(attr);
    int         closed = 1;
    const char* cmds   = attr->val;
    enum
    for (int i = 0; cmds[i]; i++) {
        if (cmds[i] == 'z' || cmds[i] == 'Z') {
            closed = 1;
            break;
        }
    }
}