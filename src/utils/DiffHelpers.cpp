#include "DiffHelpers.h"


namespace DiffHelpers {

// Helper to trim lines so whitespace differences don't cause false mismatches
static QString normalize(const QString& s) {
    return s.trimmed();
}

QVector<DiffHunk> computeDiff(const QStringList &oldLines, const QStringList &newLines) {
    int N = oldLines.size();
    int M = newLines.size();

    // Compute Longest Common Subsequence (LCS) Matrix
    // L[i][j] stores the length of the LCS between oldLines[0...i-1] and newLines[0...j-1]
    std::vector<std::vector<int>> L(N + 1, std::vector<int>(M + 1));

    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= M; ++j) {
            if (i == 0 || j == 0) {
                L[i][j] = 0;
            } else if (normalize(oldLines[i - 1]) == normalize(newLines[j - 1])) {
                // Lines match (ignoring whitespace), extend LCS length
                L[i][j] = L[i - 1][j - 1] + 1;
            } else {
                // Lines don't match, take max LCS from previous states
                L[i][j] = std::max(L[i - 1][j], L[i][j - 1]);
            }
        }
    }

    // Backtrack through the matrix to generate the diff hunks
    QVector<DiffHunk> diffs;
    int i = N, j = M;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && normalize(oldLines[i - 1]) == normalize(newLines[j - 1])) {
            // Part of LCS: Line is unchanged
            diffs.prepend({NoChange, oldLines[i - 1]});
            i--; j--;
        }
        else if (j > 0 && (i == 0 || L[i][j - 1] >= L[i - 1][j])) {
            // Moved left in matrix: Line came from newLines (Insertion)
            diffs.prepend({Inserted, newLines[j - 1]});
            j--;
        }
        else {
            // Moved up in matrix: Line came from oldLines (Deletion)
            diffs.prepend({Deleted, oldLines[i - 1]});
            i--;
        }
    }

    return diffs;
}

}