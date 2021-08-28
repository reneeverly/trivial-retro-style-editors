/*
 * Program: demo
 * Project: reneeverly/trivial-retro-style-editors
 * License: Apache 2.0
 * Author: Renee Waverly Sonntag
 *
 * Description:
 *
 *      This is a template program used to test the include header functionality.
 */

#include <string>

#include "../../include/terminal/terminal.h"
#include "../../include/terminal/tui.h"
#include "../../include/terminal/keyboard.h"

using namespace std;

terminal rt;
tui ui(&rt);

int main(void) {
   rt.clear();

   ui.drawFunctionLabels("F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8=exit");

   rt.moveCursor(0, 0);

   // Character input loop
   int c;
   while(true) {
      c = getch();

      if (c && c != LITERAL_KEY_ESCAPE) {
         // do nothing
         cout << (size_t) c << endl;
      } else {
         int resultant = resolveEscapeSequence();

         if (resultant == KEY_F8) {
            rt.resetTerminal();
            exit(0);
         } else if (resultant == KEY_HOME) {
            cout << "Home!" << endl;
         } else if (resultant == KEY_END) {
            cout << "End!" << endl;
         } else if (resultant == KEY_PGUP) {
            cout << "Page Up!" << endl;
         } else if (resultant == KEY_PGDN) {
            cout << "Page Down!" << endl;
         }
      }
   }
}
         
