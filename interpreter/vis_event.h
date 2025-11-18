#pragma once
#include <QString>

struct VisEvent {
    enum Type { Call, Return, Compare, Assign, Swap, VectorSet, Highlight } type;
    QString name;    // function name or info
    int i=-1, j=-1;  // indices (for vector ops)
    long long value=0; // values when relevant
};
