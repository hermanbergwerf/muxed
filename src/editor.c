// Copyright (c) 2017, Herman Bergwerf. All rights reserved.
// Use of this source code is governed by a MIT-style license
// that can be found in the LICENSE file.

#include <curses.h>
#include <string.h>

typedef struct {
  WINDOW *window;

  /// File path specified at startup.
  char *filePath;

  /// Array of NULL-terminated text strings containing each line.
  /// Note: this should probably be a linked list.
  char **textBuffer;

  /// Total number of lines.
  int lineCount;

  /// Current scrolling offset.
  int xOffset;
  int yOffset;
} EditorData;

/// Not the most efficent, but for now the easiest approach.
void redrawEditor(EditorData *data) {
  // Get dimensions and erase.
  int w = 0, h = 0;
  getmaxyx(data->window, w, h);  // Note: this is an ugly dirty macro.
  erase();

  // Draw each line.
  for (int i = 0; i < h && i + data->xOffset < data->lineCount; i++) {
    mvaddstr(0, i, data->textBuffer[i]);
  }

  // Refresh screen.
  refresh();
}

/// Insert newline after the given row in the buffer. Returns true on error.
bool insertLine(EditorData *data, int row) {
  if (data->lineCount <= row || row < -1) return true;

  // Reallocate array of line pointers.
  int newLineCount = data->lineCount + 1;
  char **textBuffer = (char **)malloc(sizeof(char *) * newLineCount);

  // Move over all old pointers.
  int offset = 0;
  for (int i = 0; i < newLineCount; i++) {
    if (i == row + 1) {
      char *emptyLine = (char *)malloc(1 * sizeof(char));
      emptyLine[0] = 0;
      textBuffer[i] = emptyLine;
      offset = 1;
    } else {
      textBuffer[i] = data->textBuffer[i + offset];
    }
  }

  // Swap and free.
  free(data->textBuffer);
  data->textBuffer = textBuffer;
  data->lineCount = newLineCount;

  return false;
}

/// Insert the given fragment of length n at the given position in the text
/// buffer in the given editor. Returns true on error.
bool insertText(EditorData *data, int row, int col, char *fragment, int n) {
  if (data->lineCount <= row) return true;

  char *oldLine = data->textBuffer[row];
  int oldLineLength = strlen(oldLine);
  if (oldLineLength < col) return true;

  // Reallocate the given line and copy the old line while inserting fragment.
  char *newLine = (char *)malloc((oldLineLength + n + 1) * sizeof(char));
  strncpy(newLine, oldLine, col);
  strncpy(newLine + col, fragment, n);
  strncpy(newLine + col + n, oldLine + col, oldLineLength - col);

  // Ensure the string is NULL-terminated.
  newLine[oldLineLength + n] = 0;

  // Swap and free.
  data->textBuffer[row] = newLine;
  free(oldLine);

  return false;
}