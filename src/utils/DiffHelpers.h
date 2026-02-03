#pragma once
#include <QStringList>
#include <QVector>
#include <vector>
#include <algorithm>

namespace DiffHelpers {

// Defines the type of change for a line of code
enum ChangeType {
    NoChange, // Line exists in both original and new
    Inserted, // Line exists only in new
    Deleted   // Line exists only in original
};

// Represents a single line's status in the diff
struct DiffHunk {
    ChangeType type;
    QString line;
};

// Computes the diff between two lists of strings using an LCS algorithm.
// Uses trimmed line comparison to ignore whitespace differences.
QVector<DiffHunk> computeDiff(const QStringList &oldLines, const QStringList &newLines);

}