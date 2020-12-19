#include "svg.h"

typedef enum {
    UI_DIV,
    UI_VECTORIMAGE,
    UI_BUTTON,
    UI_EDITTEXT,
    UI_LABEL,
    UI_TABLE,
    UI_TABLEROW,
    UI_TABLEDATA
} UIElementType;
typedef enum { FONT_SMALL, FONT_MEDIUM, FONT_LARGE } FontSize;
typedef enum { ALIGN_LEFT, ALIGN_HCENTER, ALIGN_RIGHT } HAlignment;
typedef enum { ALIGN_TOP, ALIGN_VCENTER, ALIGN_BOTTOM } VAlignment;
typedef enum { TL, TC, TR, CL, CC, CR, BL, BC, BR, ABS } ElementAlignment;
typedef enum { FLOW_COL, FLOW_ROW } FlowDirection;

typedef struct {
    double x, y;
    double w, h;
} BoundingBox;

void adjustBB(BoundingBox cont, ElementAlignment p, BoundingBox* b)
{
    switch (p) {
        case TL: b->x = cont.x, b->y = cont.y + cont.h - b->h; break;
        case TC: b->x = cont.x + cont.w / 2 - b->w / 2, b->y = cont.y + cont.h - b->h; break;
        case TR: b->x = cont.x + cont.w - b->w, b->y = cont.y + cont.h - b->h; break;
        case CL: b->x = cont.x, b->y = cont.y + cont.h / 2 - b->h / 2; break;
        case CC: b->x = cont.x + cont.w / 2 - b->w / 2, b->y = cont.y + cont.h / 2 - b->h / 2; break;
        case CR: b->x = cont.x + cont.w - b->w, b->y = cont.y + cont.h / 2 - b->h / 2; break;
        case BL: b->x = cont.x, b->y = cont.y; break;
        case BC: b->x = cont.x + cont.w / 2 - b->w / 2, b->y = cont.y; break;
        case BR: b->x = cont.x + cont.w - b->w, b->y = cont.y; break;
    }
}

BoundingBox cutBB(BoundingBox* c, ElementAlignment p, FlowDirection d, double w, double h)
{
    BoundingBox r;
    int         col = d == FLOW_COL;
    r.w = w, r.h = h;
    r.x = c->x, r.y = c->y;
    switch (p) {
        case TL:
            r.x = c->x;
            r.y = c->y + c->h - h;
            if (col)
                c->h -= h;
            else
                c->x += w;
            break;
        case TC:
        case CC:
            r.y = c->y + c->h - h;
            if (col) {
                r.x = c->x + c->w / 2 - w / 2;
                c->h -= h;
            }
            else {
                r.x = c->x;
                c->x += w;
            }
            break;
        case TR:
            r.x = c->x + c->w - w;
            r.y = c->y + c->h - h;
            if (col)
                c->h -= h;
            else
                c->w -= w;
            break;
        case CL:
            r.x = c->x;
            if (col) {
                r.y = c->y + c->h - h;
                c->h -= h;
            }
            else {
                r.y = c->y + c->h / 2 - h / 2;
                c->x += w;
            }
            break;
        case CR:
            r.x = c->x + c->w - w;
            if (col) {
                r.y = c->y + c->h - h;
                c->h -= h;
            }
            else {
                r.y = c->y + c->h / 2 - h / 2;
                c->w -= w;
            }
            break;
        case BL:
            r.x = c->x;
            r.y = c->y;
            if (col)
                c->y += h;
            else
                c->x += w;
            break;
        case BC:
            r.y = c->y;
            if (col) {
                r.x = c->x + c->w / 2 - w / 2;
                c->h -= h;
            }
            else {
                r.x = c->x;
                c->x += w;
            }
            break;
        case BR:
            r.x = c->x + c->w - w;
            r.y = c->y;
            if (col)
                c->y += h;
            else
                c->w -= w;
            break;
    }
    return r;
}

typedef union {
    int    iData;
    double fData;
    void*  pData;
} Data;

typedef struct {
    HAlignment h;
    VAlignment v;
} Alignment;

typedef struct {
    double t, b, l, r;
} Spacing;

typedef struct _UIStyle {
    Fill          fill;
    Stroke        stroke;
    Fill          textFill;
    FontSize      fontSize;
    Alignment     alignment;
    FlowDirection flowDirection;
    Spacing       padding, margin;
    int           absolute;
    double        width, height;
    double        actualWidth, actualHeight;
    double        innerWidth, innerHeight;
    double        outerWidth, outerHeight;
    BoundingBox   bb;
    void (*onMouse)(struct _UIStyle*, int enter);
    void (*onClick)(struct _UIStyle*, int button, int state);
} UIStyle;

typedef struct _Element {
    struct _Element *parent, *children;
    struct _Element *next, *prev; // circular
    UIElementType    type;
    UIStyle          style;
    Data             data;
} Element;
typedef struct {
    Element base;
    char*   buffer;
    int     pos;
    int     capacity;
} Label;
typedef struct {
    Element      base;
    SVGObject*   svg;
    double       w, h;
    TransformMat transform;
    double       opacity;
} VectorImage;
typedef struct {
    Element      base;
    VectorImage* background;
} Div;
typedef struct _Button {
    Element      base;
    Label*       label;
    VectorImage* icon;
    void (*onClick)(struct _Button* element, int button, int state);
} Button;
typedef struct _EditText {
    Element base;
    Label*  text;
    void (*onKey)(struct _EditText* element, unsigned char key);
} EditText;

typedef struct {
    Element base;
    Label*  text;
} TableData;

typedef struct {
    Element base;
} TableRow;

typedef struct {
    Element base;
    int     n_rows, max_cols;
    int     col_capacity;
    double* widths;
} Table;

typedef struct {
    Div* rootDiv;
} Layout;

BoundingBox innerBB(BoundingBox bb, Spacing margin, Spacing padding)
{
    BoundingBox ret = {.x = bb.x + margin.l + padding.l,
                       .y = bb.y + margin.b + padding.b,
                       .w = bb.w - (margin.l + margin.r + padding.l + padding.r),
                       .h = bb.h - (margin.t + margin.b + padding.t + padding.b)};
    return ret;
}

UIStyle defaultStyle()
{
    UIStyle ret;
    ret.fill          = {.fill = 0, .color = {255, 255, 255}, .opacity = 1};
    ret.stroke        = {.width = 0, .color = {0}, .opacity = 1};
    ret.textFill      = {.fill = 1, .color = {0}, .opacity = 1};
    ret.fontSize      = FONT_MEDIUM;
    ret.alignment     = {.h = ALIGN_LEFT, .v = ALIGN_TOP};
    ret.flowDirection = FLOW_COL;
    ret.absolute      = 0;
    ret.padding = ret.margin = {0, 0, 0, 0};
    ret.bb                   = {0, 0, 0, 0};
    ret.width = ret.height = 0;
    ret.outerWidth = ret.outerHeight = 0;
    ret.onClick                      = NULL;
    ret.onMouse                      = NULL;
    return ret;
}

void addChild(Element* parent, Element* child)
{
    child->parent = parent;
    if (parent->children) {
        child->next       = parent->children;
        child->prev       = parent->children->prev;
        child->next->prev = child->prev->next = child;
    }
    else {
        parent->children = child;
        child->next = child->prev = child;
    }
}

void addAfter(Element* before, Element* after)
{
    assert(before);
    after->parent     = before->parent;
    after->prev       = before;
    after->next       = before->next;
    after->next->prev = after->prev->next = after;
}
void freeElement(Element* element)
{
    switch (element->type) {
        case UI_LABEL: free(((Label*)element)->buffer); break;
    }
    free(element);
}

void clearElements(Element* element)
{
    Element *x = element->children, *y;
    while (x) {
        y = x;
        x = y->next;
        clearElements(y);
        if (x == element->children) break;
    }
    // element is actually a pointer ot the derived ui type
    freeElement(element);
}
void removeElement(Element* element)
{
    assert(element);
    if (element == element->parent->children) {
        if (element->next == element)
            element->parent->children = NULL;
        else
            element->parent->children = element->next;
    }
    element->prev->next = element->next;
    element->next->prev = element->prev;
    clearElements(element);
}

void clearLayout(Layout* layout)
{
    clearElements((Element*)layout->rootDiv);
    free(layout);
}

void addToLayout(Layout* layout, Element* child) { addChild((Element*)layout->rootDiv, child); }

void initElement(Element* elm, UIElementType type)
{
    elm->parent = elm->children = elm->next = elm->prev = NULL;
    elm->type                                           = type;
    elm->style                                          = defaultStyle();
}

VectorImage* createVectorImage(SVGObject* svg)
{
    VectorImage* image = (VectorImage*)malloc(sizeof(VectorImage));
    image->svg         = svg;
    image->transform   = identity();
    image->w = image->h = -1;
    image->opacity      = 1;
    initElement((Element*)image, UI_VECTORIMAGE);
    image->base.style.width = image->base.style.height = -1;
    image->base.style.absolute                         = 1;
    return image;
}

void resizeVectorImage(VectorImage* vi, double w, double h)
{
    SVGFitToScreen(vi->svg, w, h);
    vi->transform = vi->svg->localTransform;
    vi->w = w, vi->h = h;
}
int vectorImageIsFit(VectorImage* img)
{
    return !(fabs(img->w - img->base.style.actualWidth) > 0.1) || !(fabs(img->h - img->base.style.actualHeight) > 0.1);
}

Label* createLabel(const char* text)
{
    Label* label = (Label*)malloc(sizeof(Label));
    initElement((Element*)label, UI_LABEL);
    label->base.style.width = label->base.style.height = 0;
    label->capacity                                    = 16;
    int len                                            = strlen(text);
    while (label->capacity < len)
        label->capacity *= 2;
    label->buffer = (char*)malloc(label->capacity + 1);
    strcpy(label->buffer, text);
    label->pos = len;
    return label;
}
EditText* createEditText()
{
    EditText* editText = (EditText*)malloc(sizeof(EditText));
    initElement((Element*)editText, UI_EDITTEXT);
    editText->text  = createLabel("");
    editText->onKey = NULL;
    addChild((Element*)editText, (Element*)editText->text);
    return editText;
}
void updateText(Label* label, const char* text)
{
    int len        = strlen(text);
    int n_capacity = label->capacity;
    while (label->capacity < len)
        n_capacity *= 2;
    while (label->capacity / 2 > len) {
        n_capacity /= 2;
        if (n_capacity <= 16) break;
    }
    if (n_capacity != label->capacity) {
        label->capacity = n_capacity;
        label->buffer   = (char*)malloc(label->capacity + 1);
    }
    strcpy(label->buffer, text);
    label->pos = len;
}

void addChar(Label* label, char ch)
{
    if (label->pos >= label->capacity) {
        label->capacity *= 2;
        char* t = (char*)malloc(label->capacity + 1);
        memcpy(t, label->buffer, label->pos);
        free(label->buffer);
        label->buffer = t;
    }
    label->buffer[label->pos]   = ch;
    label->buffer[++label->pos] = '\0';
}
void subChar(Label* label)
{
    if (label->pos == 0) return;
    if (label->pos <= label->capacity / 2) {
        label->capacity /= 2;
        char* t = (char*)malloc(label->capacity + 1);
        memcpy(t, label->buffer, label->pos);
        free(label->buffer);
        label->buffer = t;
    }
    label->buffer[--label->pos] = '\0';
}

void fitLabel(Label* label)
{
    double w = 0;
    double h = 1;
    for (int i = 0; label->buffer[i]; i++) {
        if (!isalnum(label->buffer[i]))
            w += 0.6;
        else if (isupper(label->buffer[i]) && (label->buffer[i] != 'I') || isdigit(label->buffer[i]))
            w += 0.95;
        else
            w += 0.65;
    }
    switch (label->base.style.fontSize) {
        case FONT_LARGE:
            w *= 15;
            h *= 22;
            break;
        case FONT_MEDIUM:
            w *= 13.5;
            h *= 18;
            break;
        case FONT_SMALL:
            w *= 8;
            h *= 10;
            break;
    }
    label->base.style.actualWidth  = w;
    label->base.style.actualHeight = h;
}

Button* createButton(const char* text, SVGObject* icon)
{
    Button* button = (Button*)malloc(sizeof(Button));
    button->label  = NULL;
    button->icon   = NULL;
    initElement((Element*)button, UI_BUTTON);
    if (text) {
        button->label = createLabel(text);
        addChild((Element*)button, (Element*)button->label);
    };
    if (icon) {
        button->icon                         = createVectorImage(icon);
        button->icon->base.style.absolute    = 0;
        button->icon->base.style.alignment.h = ALIGN_RIGHT;
        addChild((Element*)button, (Element*)button->icon);
    }
    button->onClick = NULL;
    return button;
}

Div* createDiv()
{
    Div* div = (Div*)malloc(sizeof(Div));
    initElement((Element*)div, UI_DIV);
    div->background = NULL;
    return div;
}

Table* createTable()
{
    Table* table = (Table*)malloc(sizeof(Table));
    initElement((Element*)table, UI_TABLE);
    table->base.style.flowDirection = FLOW_COL;
    table->n_rows = 0, table->max_cols = 0;
    table->col_capacity = 8;
    table->widths       = (double*)malloc(sizeof(double) * table->col_capacity);
    memset(table->widths, 0, sizeof(double) * table->col_capacity);
    return table;
}

TableRow* addTableRow(Table* table)
{
    TableRow* tr = (TableRow*)malloc(sizeof(TableRow));
    initElement((Element*)tr, UI_TABLEROW);
    tr->base.style.flowDirection = FLOW_ROW;
    addChild((Element*)table, (Element*)tr);
    return tr;
}

TableData* addTableData(TableRow* row, const char* text)
{
    TableData* td = (TableData*)malloc(sizeof(TableData));
    initElement((Element*)td, UI_TABLEDATA);
    td->text = createLabel(text);
    addChild((Element*)td, (Element*)td->text);
    addChild((Element*)row, (Element*)td);
    return td;
}

Layout* createLayout(double x, double y, double w, double h)
{

    Layout* layout                     = (Layout*)malloc(sizeof(Layout));
    layout->rootDiv                    = createDiv();
    layout->rootDiv->base.style.width  = w;
    layout->rootDiv->base.style.height = h;
    layout->rootDiv->base.style.bb     = {.x = x, .y = y, .w = w, .h = h};
    return layout;
}

void renderRect(Element* element)
{
    UIStyle* style = &element->style;
    if (style->fill.fill) {
        iSetColorEx(style->fill.color.r, style->fill.color.g, style->fill.color.b, style->fill.opacity);
        iFilledRectangle(
            style->bb.x + style->margin.l, style->bb.y + style->margin.b, style->actualWidth, style->actualHeight);
    }
    if (style->stroke.width > 0.1) {
        iSetColorEx(style->stroke.color.r, style->stroke.color.g, style->stroke.color.b, style->stroke.opacity);
        iRectangleEx({style->bb.x + style->margin.l, style->bb.y + style->margin.b},
                     {style->actualWidth, style->actualHeight},
                     style->stroke.width);
    }
}

void renderVectorImage(VectorImage* image)
{
    Point        p      = {image->base.style.bb.x + image->base.style.margin.l,
               image->base.style.bb.y + image->base.style.margin.b};
    TransformMat mat    = matMul(translateMat(p), image->transform);
    image->svg->opacity = image->opacity;
    renderSVGObject(image->svg, mat);
}

void renderLabel(Label* label)
{
    double x = label->base.style.bb.x + label->base.style.margin.l;
    double y = label->base.style.bb.y + label->base.style.margin.b;
    iSetColorEx(label->base.style.textFill.color.r,
                label->base.style.textFill.color.g,
                label->base.style.textFill.color.b,
                label->base.style.textFill.opacity);
    switch (label->base.style.fontSize) {
        case FONT_LARGE: iText(x, y, label->buffer, GLUT_BITMAP_TIMES_ROMAN_24); break;
        case FONT_MEDIUM: iText(x, y, label->buffer, GLUT_BITMAP_HELVETICA_18); break;
        case FONT_SMALL: iText(x, y, label->buffer, GLUT_BITMAP_HELVETICA_12); break;
    }
}

void adjustTable(Element* element)
{
    Table*   table     = (Table*)element;
    int      n_rows    = 0;
    int      max_cols  = 0;
    double   max_width = 0;
    double   w = 0, h = 0;
    Element* tr = element->children;
    while (tr) {
        max_width           = max(max_width, tr->style.outerWidth);
        double   max_height = 0;
        int      n_cols     = 0;
        Element* td         = tr->children;
        while (td) {
            max_height = max(max_height, td->style.outerHeight);
            n_cols++;
            td = td->next;
            if (td == tr->children) break;
        }
        td = tr->children;
        while (td) {
            td->style.outerHeight = max_height;
            td                    = td->next;
            if (td == tr->children) break;
        }
        h += max_height;
        max_cols = max(max_cols, n_cols);
        n_rows++;
        tr = tr->next;
        if (tr == element->children) break;
    }
    if (max_cols > table->col_capacity) {
        table->col_capacity *= 2;
        table->widths = (double*)malloc(sizeof(double) * table->col_capacity);
    }
    if (max_cols < table->col_capacity / 2) {
        table->col_capacity /= 2;
        table->widths = (double*)malloc(sizeof(double) * table->col_capacity);
    }
    memset(table->widths, 0, sizeof(double) * table->col_capacity);
    tr = element->children;
    while (tr) {
        int      col = 0;
        Element* td  = tr->children;
        while (td) {
            table->widths[col] = max(table->widths[col], td->style.outerWidth);
            col++;
            td = td->next;
            if (td == tr->children) break;
        }
        tr = tr->next;
        if (tr == element->children) break;
    }
    for (int i = 0; i < max_cols; i++)
        w += table->widths[i];
    tr = element->children;
    while (tr) {
        tr->style.outerWidth = w;
        int      col         = 0;
        Element* td          = tr->children;
        while (td) {
            td->style.outerWidth = table->widths[col];
            col++;
            td = td->next;
            if (td == tr->children) break;
        }
        tr = tr->next;
        if (tr == element->children) break;
    }
    element->style.outerWidth =
        w + element->style.padding.l + element->style.padding.r + element->style.margin.l + element->style.margin.r;
    element->style.outerHeight =
        h + element->style.padding.t + element->style.padding.b + element->style.margin.t + element->style.margin.b;
}

void calcSizesBU(Element* element, double* cW, double* cH)
{
    double px                   = element->style.padding.l + element->style.padding.r;
    double py                   = element->style.padding.t + element->style.padding.b;
    element->style.actualWidth  = px;
    element->style.actualHeight = py;
    switch (element->type) {
        case UI_LABEL: {
            fitLabel((Label*)element);
        } break;
    }
    if (element->style.width > 0) element->style.actualWidth = element->style.width;
    if (element->style.height > 0) element->style.actualHeight = element->style.height;
    Element* x = element->children;
    while (x) {
        double w, h;
        calcSizesBU(x, &w, &h);
        if (element->style.width == 0) {
            if (element->style.flowDirection == FLOW_COL)
                element->style.actualWidth = max(element->style.actualWidth, w + px);
            else
                element->style.actualWidth += w;
        }
        if (element->style.height == 0) {
            if (element->style.flowDirection == FLOW_COL)
                element->style.actualHeight += h;
            else
                element->style.actualHeight = max(element->style.actualHeight, h + py);
        }
        x = x->next;
        if (x == element->children) break;
    }
    element->style.innerWidth =
        max(element->style.actualWidth - element->style.padding.l - element->style.padding.r, 0);
    element->style.innerHeight =
        max(element->style.actualHeight - element->style.padding.t - element->style.padding.b, 0);
    element->style.outerWidth  = element->style.actualWidth + element->style.margin.l + element->style.margin.r;
    element->style.outerHeight = element->style.actualHeight + element->style.margin.t + element->style.margin.b;
    switch (element->type) {
        case UI_TABLE: adjustTable(element); break;
    }
    *cW = element->style.outerWidth;
    *cH = element->style.outerHeight;
}
void calcSizesTD(Element* element, double pW, double pH)
{
    if (element->style.width == -1) element->style.outerWidth = pW;
    if (element->style.height == -1) element->style.outerHeight = pH;
    element->style.actualWidth  = element->style.outerWidth - element->style.margin.l - element->style.margin.r;
    element->style.actualHeight = element->style.outerHeight - element->style.margin.t - element->style.margin.b;
    element->style.innerWidth =
        max(element->style.actualWidth - element->style.padding.l - element->style.padding.r, 0);
    element->style.innerHeight =
        max(element->style.actualHeight - element->style.padding.t - element->style.padding.b, 0);
    switch (element->type) {
        case UI_VECTORIMAGE: {
            VectorImage* img = (VectorImage*)element;
            if (!vectorImageIsFit(img)) resizeVectorImage(img, element->style.actualWidth, element->style.actualHeight);
        } break;
    }
    Element* x = element->children;
    while (x) {
        calcSizesTD(x, element->style.innerWidth, element->style.innerHeight);
        x = x->next;
        if (x == element->children) break;
    }
}

ElementAlignment getElementAlignment(Element* x)
{
    ElementAlignment p = TL;
    switch (x->style.alignment.v) {
        case ALIGN_TOP:
            switch (x->style.alignment.h) {
                case ALIGN_LEFT: p = TL; break;
                case ALIGN_HCENTER: p = TC; break;
                case ALIGN_RIGHT: p = TR; break;
            }
            break;
        case ALIGN_VCENTER:
            switch (x->style.alignment.h) {
                case ALIGN_LEFT: p = CL; break;
                case ALIGN_HCENTER: p = CC; break;
                case ALIGN_RIGHT: p = CR; break;
            }
            break;
        case ALIGN_BOTTOM:
            switch (x->style.alignment.h) {
                case ALIGN_LEFT: p = BL; break;
                case ALIGN_HCENTER: p = BC; break;
                case ALIGN_RIGHT: p = BR; break;
            }
            break;
    }
    return p;
}

void calcBounds(Element* element)
{
    BoundingBox bb[10] = {0};
    Element*    x      = element->children;
    while (x) {
        if (!x->style.absolute) {
            ElementAlignment p = getElementAlignment(x);
            if (element->style.flowDirection == FLOW_COL) {
                bb[p].w = max(bb[p].w, x->style.outerWidth);
                bb[p].h += x->style.outerHeight;
            }
            else {
                bb[p].w += x->style.outerWidth;
                bb[p].h = max(bb[p].h, x->style.outerHeight);
            }
        }
        x = x->next;
        if (x == element->children) break;
    }
    BoundingBox ibb = innerBB(element->style.bb, element->style.margin, element->style.padding);
    for (int i = 0; i < 9; i++)
        adjustBB(ibb, (ElementAlignment)i, &bb[i]);
    x = element->children;
    while (x) {
        ElementAlignment p = getElementAlignment(x);
        if (!x->style.absolute)
            x->style.bb = cutBB(&bb[p], p, element->style.flowDirection, x->style.outerWidth, x->style.outerHeight);
        else
            x->style.bb = {.x = ibb.x, .y = ibb.y, .w = x->style.outerWidth, .h = x->style.outerHeight};
        calcBounds(x);
        x = x->next;
        if (x == element->children) break;
    }
}

void updateLayout(Layout* layout)
{
    double w, h;
    calcSizesBU((Element*)layout->rootDiv, &w, &h);
    calcSizesTD((Element*)layout->rootDiv, 0, 0);
    calcBounds((Element*)layout->rootDiv);
}
void renderElement(Element* element)
{
    renderRect(element);
    switch (element->type) {
        case UI_LABEL: renderLabel((Label*)element); break;
        case UI_VECTORIMAGE: renderVectorImage((VectorImage*)element); break;
    }
    Element* x = element->children;
    while (x) {
        renderElement(x);
        x = x->next;
        if (x == element->children) break;
    }
}

void renderLayout(Layout* layout)
{
    updateLayout(layout);
    renderElement((Element*)layout->rootDiv);
}

int insideElement(Element* element, int mx, int my)
{
    return (element->style.bb.x + element->style.margin.l <= mx) &&
           (mx <= element->style.bb.x + element->style.bb.w - element->style.margin.r) &&
           (element->style.bb.y + element->style.margin.b <= my) &&
           (my <= element->style.bb.y + element->style.bb.h - element->style.margin.t);
}

void handleElementMouseClick(Element* element, int button, int state, int mx, int my)
{
    if (insideElement(element, mx, my)) {
        Element* x = element->children;
        while (x) {
            handleElementMouseClick(x, button, state, mx, my);
            x = x->next;
            if (x == element->children) break;
        }
        if (element->style.onClick) element->style.onClick(&element->style, button, state);
        switch (element->type) {
            case UI_BUTTON: {
                Button* b = (Button*)element;
                if (b->onClick) b->onClick(b, button, state);
            } break;
        }
    }
}
void handleElementPassiveMouse(Element* element, int mx, int my)
{
    Element* x = element->children;
    while (x) {
        handleElementPassiveMouse(x, mx, my);
        x = x->next;
        if (x == element->children) break;
    }
    if (element->style.onMouse) element->style.onMouse(&element->style, insideElement(element, mx, my));
}

void handleElementKeyInput(Element* element, unsigned char key)
{
    Element* x = element->children;
    while (x) {
        handleElementKeyInput(x, key);
        x = x->next;
        if (x == element->children) break;
    }
    switch (element->type) {
        case UI_EDITTEXT: {
            EditText* editText = (EditText*)element;
            if (editText->onKey) editText->onKey(editText, key);
        } break;
    }
}

void handleMouseClick(Layout* layout, int button, int state, int mx, int my)
{
    handleElementMouseClick((Element*)layout->rootDiv, button, state, mx, my);
}
void handlePassiveMouse(Layout* layout, int mx, int my)
{
    handleElementPassiveMouse((Element*)layout->rootDiv, mx, my);
}

void handleKeyInput(Layout* layout, unsigned char key) { handleElementKeyInput((Element*)layout->rootDiv, key); }

void addBackground(Layout* layout, SVGObject* svg)
{
    layout->rootDiv->background = createVectorImage(svg);
    addChild((Element*)layout->rootDiv, (Element*)layout->rootDiv->background);
}