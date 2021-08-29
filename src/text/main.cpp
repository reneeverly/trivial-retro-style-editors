/*
 * Program: text
 * Project: reneeverly/trivial-retro-style-editors
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 *
 * Description:
 *
 *      This is a text editor.
 */

#include <string>

#include "../../include/terminal/terminal.h"
#include "../../include/terminal/tui.h"
#include "../../include/terminal/keyboard.h"

// ifnore utf8 for now :(
//#include "../../include/misc/basic_utf8.h"

#include <vector>

using namespace std;

terminal rt;
tui ui(&rt);

int main(void) {
   rt.clear();

   //ui.drawFunctionLabels("F1=Find", "F2=Load", "F3=Save", "", "F5=Copy", "F6=Cut", "F7=Select", "F8=Exit");
   ui.drawFunctionLabels("", "", "", "", "", "", "", "F8=Exit");

   rt.moveCursor(0, 0);

   vector<string> file;

   file.emplace(file.begin(), "");

   size_t startLine = 0;
   size_t startCursor = 0; // for use when line length exceeds terminal width, will be a multiple of the screen width
   size_t virtualCursorLine = 0;
   size_t virtualCursorChar = 0;

   // Character input loop
   int c;
   while(true) {
      c = getch();

      if (c && c != LITERAL_KEY_ESCAPE) {
         if ((c == 0x08) || (c == 0x7f)) {
            // backspace key

            if ((virtualCursorChar == 0) && (virtualCursorLine != 0)) {
               // at start of a line which is not the first line, append this line to the previous line
               virtualCursorChar = file.at(virtualCursorLine - 1).length(); // special case to get preceeding line length
               file.at(virtualCursorLine - 1).append(file.at(virtualCursorLine));
               file.erase(file.begin() + virtualCursorLine);
               virtualCursorLine--;
            } else if (file.at(virtualCursorLine).length() > 0) {
               // within a line, just delete the caracter preceeding it
               file.at(virtualCursorLine).erase(file.at(virtualCursorLine).begin() + virtualCursorChar - 1);
               virtualCursorChar--;
            }
         } else if ((c == 10) || (c == 13)) {
            // enter key

            if (virtualCursorChar == file.at(virtualCursorLine).length()) {
               // cursor is at end of line, simply create a blank new line after it.
               file.emplace(file.begin() + virtualCursorLine + 1, "");
            } else {
               // cursor is within the line, cut characters out of current line and paste them into a new line.
               file.emplace(file.begin() + virtualCursorLine + 1, file.at(virtualCursorLine).substr(virtualCursorChar));
               file.at(virtualCursorLine).erase(virtualCursorChar);
            }
            virtualCursorChar = 0;
            virtualCursorLine++;
         } else {
            // emplace character at current position
            file.at(virtualCursorLine).insert(file.at(virtualCursorLine).begin() + virtualCursorChar, c);
            virtualCursorChar++; 
         }
      } else {
         int resultant = resolveEscapeSequence();

         if (resultant == KEY_LEFT) {
            // Decrement the virtual cursor character position if possible
            if (virtualCursorChar > 0) {
               virtualCursorChar--;
            }
         } else if (resultant == KEY_RIGHT) {
            // Increment the virtual cursor character position if possible
            // can exceed length by 1 for append position
            if (virtualCursorChar < file.at(virtualCursorLine).length()) {
               virtualCursorChar++;
            }
         } else if (resultant == KEY_UP) {
            // decrement the virtual line position if possible
            if (virtualCursorLine > 0) {
               virtualCursorLine--;
               virtualCursorChar = 0;
            }
         } else if (resultant == KEY_DOWN) {
            // increment the virtual cursor character position if possible
            if (virtualCursorLine < (file.size() - 1)) {
               virtualCursorLine++;
               virtualCursorChar = 0;
            }
         } else if (resultant == KEY_HOME) {
            virtualCursorChar = 0;
         } else if (resultant == KEY_END) {
            virtualCursorChar = file.at(virtualCursorLine).length();
         } else if (resultant == KEY_F8) {
            rt.resetTerminal();
            exit(0);
         }
      }

      rt.moveCursor(0,0);

      // Ensure that the virtualCursorLine is within range of startLine

      // top bound
      if (virtualCursorLine < startLine) {
         startLine = virtualCursorLine;
      }

      // bottom bound
      size_t screen_lines_from_top = 0;
      for (size_t i = virtualCursorLine; i >= startLine; i--) {
         if (i == virtualCursorLine) {
            screen_lines_from_top += ((virtualCursorChar != rt.cols) ? 1 : 0) + (virtualCursorChar / (rt.cols));
         } else {
            screen_lines_from_top += ((file.at(i).length() != rt.cols) ? 1 : 0) + (file.at(i).length() / (rt.cols));
         }

         // check for too far
         if ((screen_lines_from_top) >= (rt.lines - 1)) {
            startLine = i;
            startCursor = ((screen_lines_from_top) - (rt.lines - 1)) * rt.cols;
            break;
         }

         // exit before it wraps around if startLine is 0
         if (i == 0) break;
      }

      // adjust for when editing a long line
      if (virtualCursorChar > rt.cols) {
         screen_lines_from_top -= (virtualCursorChar / rt.cols) + 1;
      } else {
         screen_lines_from_top--;
      }

      // this shouldn't be necessary but I'm trying to get it ready to commit (just let it work)
      if (startCursor != 0) {
         screen_lines_from_top -= (startCursor / rt.cols);
      }
      

      size_t curScreenLine = 0;
      size_t curFileLine = 0;
      size_t timesOnLine = startCursor / rt.cols;
      for (; (curScreenLine < (rt.lines - 1)) && ((curFileLine + startLine) < file.size()); curScreenLine++) {
         // print the next line of text
         cout << setw(rt.cols) << left << file.at(startLine + curFileLine).substr(timesOnLine * rt.cols, rt.cols);

         // check if we need to stay on this file line for the next screen line
         if ((file.at(startLine + curFileLine).length() - (timesOnLine * rt.cols)) > rt.cols) {
            timesOnLine++;
         } else {
            // we're done with this file line
            timesOnLine = 0;
            curFileLine++;
         }
      }

      // handle overflow
      for (; curScreenLine < (rt.lines - 1); curScreenLine++) {
         cout << string(rt.cols, ' ');
      }

      rt.moveCursor(screen_lines_from_top + (virtualCursorChar / rt.cols), virtualCursorChar % rt.cols);
   }
}

