#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext.h"
enum CSSSelectionType { CSS_CLASS, CSS_ID, CSS_TAG };
enum CSSBackgroundType { CSS_FILL, CSS_LINEAR_GRADIENT, CSS_RADIAL_GRADIENT };
enum SVGNodeType {
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
};

typedef struct {
    CSSSelectionType type;
    char*            name;
} CSSSelector;

typedef struct {
    unsigned char r, g, b;
} CSSColor;

typedef struct {
    CSSBackgroundType type;
    void*             value;
} CSSFill;

typedef struct {
    CSSColor color;
    float    width;
} CSSStroke;

typedef struct {
    CSSSelector selector;
    CSSFill     fill;
    CSSStroke   stroke;
} CSSStyle;

typedef struct _XMLNode {
    SVGNodeType      type;
    void*            value;
    char*            cssClass;
    char*            cssId;
    size_t           n_children, max_children;
    struct _XMLNode* children;
} XMLNode;

typedef struct {

} SVGGroup;
typedef struct {

} SVGRect;

typedef struct {

} SVGCircle;

typedef struct {

} SVGPath;

int CSSSelectionComp(CSSSelector* s1, CSSSelector* s2)
{
    if (s1->type != s1->type) return strcmp(s1->name, s2->name);
    return s1->type - s2->type;
}

CSSColor CSSpredefinedColor(const char* str)
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
    CSSColor col;
    col.r = r, col.g = g, col.b = b;
    return col;
}
