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

// basics
#include <vector>
#include <sstream>

// file io
#include <fstream>

using namespace std;

// flags for the file edit / screen update loop
#define UPDATE_NONE 0
#define UPDATE_ALL 1
#define UPDATE_LINE 2 // unimplemented
#define UPDATE_SUBLINE 3

#define SUGGEST_NONE 4

terminal rt;
tui ui(&rt);

// Function prototypes
void drawFunctionLabels();
void updateDisplay(const size_t &startLine, const size_t &startCursor, const vector<string> &file);
void updateLine(const size_t &screen_lines_from_top, const size_t &virtualCursorLine, const size_t &virtualCursorChar, vector<string> &file);

int main(void) {
   rt.clear();
   rt.moveCursor(rt.lines - 1, 0);
   drawFunctionLabels();
   rt.moveCursor(0, 0);

   vector<string> file;

   file.emplace(file.begin(), "");

   size_t startLine = 0;
   size_t startCursor = 0; // for use when line length exceeds terminal width, will be a multiple of the screen width
   size_t virtualCursorLine = 0;
   size_t virtualCursorChar = 0;

   int updateType;

   // File editing loop
   int c;
   while(true) {
      c = getch();
      
      // by default, assume the whole screen has to be updated
      updateType = UPDATE_ALL;

      if (c && c != LITERAL_KEY_ESCAPE) {
         if ((c == 0x08) || (c == 0x7f)) {
            // backspace key

            if ((virtualCursorChar == 0) && (virtualCursorLine == 0)) {
               // at the start of the very first line, do nothing
               updateType = UPDATE_NONE;
            } else if ((virtualCursorChar == 0) && (virtualCursorLine != 0)) {
               // at start of a line which is not the first line, append this line to the previous line
               virtualCursorChar = file.at(virtualCursorLine - 1).length(); // special case to get preceeding line length
               file.at(virtualCursorLine - 1).append(file.at(virtualCursorLine));
               file.erase(file.begin() + virtualCursorLine);
               virtualCursorLine--;
            } else if (file.at(virtualCursorLine).length() > 0) {
               // within a line, just delete the caracter preceeding it
               file.at(virtualCursorLine).erase(file.at(virtualCursorLine).begin() + virtualCursorChar - 1);
               virtualCursorChar--;

               // if this cursor is within the last subline of the line, then we can do a subline update.
               // (Since this takes place AFTER the decrement, the 3 char gap between rt.cols takes care of possible wrap problems with a subline update.)
               if (((virtualCursorChar % rt.cols) > 3) && ((virtualCursorChar % rt.cols) < (rt.cols - 3)) && ((file.at(virtualCursorLine).length() / rt.cols) == (virtualCursorChar / rt.cols))) updateType = UPDATE_SUBLINE;
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

            // if the cursor is within the last subline of the line, then we can do a subline update.
            // (Since this takes place BEFORE increment, the 3 char gap between rt.cols takes care of possible wrap problems with a subline update.)
            if (((virtualCursorChar % rt.cols) > 3) && ((virtualCursorChar % rt.cols) < (rt.cols - 3)) && (file.at(virtualCursorLine).length() / rt.cols) == (virtualCursorChar / rt.cols)) updateType = UPDATE_SUBLINE;

            // we added a character, so increment the cursor position
            virtualCursorChar++;
         }
      } else {
         int resultant = resolveEscapeSequence();

         if (resultant == KEY_LEFT) {
            // Decrement the virtual cursor character position if possible
            if (virtualCursorChar > 0) {
               virtualCursorChar--;
            }

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_RIGHT) {
            // Increment the virtual cursor character position if possible
            // can exceed length by 1 for append position
            if (virtualCursorChar < file.at(virtualCursorLine).length()) {
               virtualCursorChar++;
            }

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_UP) {
            // decrement the virtual line position if possible
            if (virtualCursorLine > 0) {
               virtualCursorLine--;
               virtualCursorChar = 0;
            }

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_DOWN) {
            // increment the virtual cursor character position if possible
            if (virtualCursorLine < (file.size() - 1)) {
               virtualCursorLine++;
               virtualCursorChar = 0;
            }

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_HOME) {
            virtualCursorChar = 0;

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_END) {
            virtualCursorChar = file.at(virtualCursorLine).length();

            // Nothing has changed, so suggest no display update (scroll routine will have final say)
            updateType = SUGGEST_NONE;
         } else if (resultant == KEY_F8) {
            // exit
            rt.resetTerminal();
            exit(0);
         } else if (resultant == KEY_F3) {
            // save file

            // prompt for file name
            rt.moveCursor(rt.lines - 2, 0);
            cout << setw(rt.cols) << left << "Save to: ";
            rt.moveCursor(rt.lines - 2, 9);

            // loop for file name
            string filename = "";
            while (true) {
               c = getch();
               
               if (c && c != LITERAL_KEY_ESCAPE) {
                  if ((c == 0x08) || (c == 0x7f)) {
                     if (filename.length() > 0) {
                        filename.pop_back();
                        cout << (char)0x08 << ' ' << (char)0x08;
                     }
                  } else if ((c == 10) || (c == 13)) {
                     // open the file and write to it!
                     std::ofstream outfile;
                     outfile.open(filename, ios_base::trunc);
                     for (size_t i = 0; i < file.size(); i++) {
                        outfile << file.at(i) << endl;
                     }
                     outfile.close();
                     break;
                  } else {
                     filename.push_back(c);
                     cout << (char)c;
                  }
               } else {
                  resultant = resolveEscapeSequence();
                  if (resultant == KEY_F8) {
                     // cancel the save
                     break;
                  }
               }
            }

            // We overlapped a line in the file
            updateType = UPDATE_ALL;
         }
      }

      // Ensure that the virtualCursorLine is within range of startLine

      // top bound
      if (virtualCursorLine < startLine) {
         startLine = virtualCursorLine;

         // need to scroll
         if (updateType == SUGGEST_NONE) updateType = UPDATE_ALL;
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

            // need to scroll
            if (updateType == SUGGEST_NONE) updateType = UPDATE_ALL;
            break;
         }

         // exit before it wraps around if startLine is 0
         if (i == 0) break;
      }

      // accept update suggestion if they got through the filter
      if (updateType == SUGGEST_NONE) updateType = UPDATE_NONE;

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
      
      // determine what, if anything, needs to be updated
      if (updateType == UPDATE_ALL) {
         // redisplay the file with changes
         updateDisplay(startLine, startCursor, file);
      } else if (updateType == UPDATE_SUBLINE) {
         // update just the line of the virtual cursor
         updateLine(screen_lines_from_top, virtualCursorLine, virtualCursorChar, file);
      } else if (updateType == UPDATE_NONE) {
         // update nothing
      }

      // place the cursor at the proper location
      rt.moveCursor(screen_lines_from_top + (virtualCursorChar / rt.cols), virtualCursorChar % rt.cols);
   }
}

/**
 * @function updateDisplay
 * As a side effect, destroys cursor location.
 * @param {size_t} startLine - the line of the file to start from
 * @param {size_t} startCursor - the character of the line of the file to start from
 * @param {vector<string>} file - the file to display
 */
void updateDisplay(const size_t &startLine, const size_t &startCursor, const vector<string> &file) {
   // ostringstream was causing a memory leak.  Reverted to cout and no bliting.
   size_t curScreenLine = 0;
   size_t curFileLine = 0;
   size_t timesOnLine = startCursor / rt.cols;
   rt.hideCursor();
   rt.moveCursor(0,0);

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

   // handle screen area after the file ends
   string blankline (rt.cols, ' ');
   for (; curScreenLine < (rt.lines - 1); curScreenLine++) {
      cout << blankline;
   }
   
   rt.showCursor();

}

/**
 * @function updateLine
 * As a side effect, destroys cursor location.
 * @param {size_t} screen_lines_from_top - the line on the screen to update
 * @param {size_t} virtualCursorLine - the line of the file to use as reference
 * @param {size_t} virtualCursorChar - the character of the line that the cursor is at
 * @param {vector<string>} file - the file to display
 */
void updateLine(const size_t &screen_lines_from_top, const size_t &virtualCursorLine, const size_t &virtualCursorChar, vector<string> &file) {
   // move to the start of the line
   rt.moveCursor(screen_lines_from_top + (virtualCursorChar / rt.cols), 0);
   
   cout << setw(rt.cols) << left << file.at(virtualCursorLine).substr((virtualCursorChar / rt.cols) * rt.cols, rt.cols);
}

void drawFunctionLabels() {
   //ui.drawFunctionLabels("F1=Find", "F2=Load", "F3=Save", "", "F5=Copy", "F6=Cut", "F7=Select", "F8=Exit");
   ui.drawFunctionLabels("", "", "F3=Save", "", "", "", "", "F8=Exit");
}

