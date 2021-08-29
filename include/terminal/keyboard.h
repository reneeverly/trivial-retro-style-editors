/*
 * Keyboard Helper Functions
 * Project: reneeverly/trivial-retro-style-editors
 * License: Apache 2.0
 * Author: Renee Waverly Sonnag
 *
 * Description:
 *
 *      I guess there's an "I used to do WIN32 programming" flavor to this header.
 *      Provides getch() because I couldn't find an analogue to it in the posix
 *      or cpp/c headers, and I just wanted a way to input a single character and
 *      see if it matched a control sequence.
 *
 *      Again, this probably could have been solved with ncurses, but as I
 *      explained in rterm.h, I wasn't able to install it at the time.
 *      Maybe I'll reimplement this in ncurses when my Raspberry Pi comes in.
 */

#ifndef RKEYBOARD_H
#define RKEYBOARD_H

#include <string>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

/*
 * Yeah, these generic definitions might conflict with things,
 * but for now it's fine.  resolveEscapeSequence() returns
 * an index which matches any of these.
 */

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_RIGHT 2
#define KEY_LEFT 3
#define KEY_F1 4
#define KEY_F2 5
#define KEY_F3 6
#define KEY_F4 7
#define KEY_F5 8
#define KEY_F6 9
#define KEY_F7 10
#define KEY_F8 11
#define KEY_F9 12 // not present on the TRS-80 Model 100
#define KEY_F10 13 // not present on the TRS-80 Model 100
//#define KEY_RSRVD 14
#define KEY_ENT 15
#define KEY_HOME 16
#define KEY_END 17
#define KEY_PGUP 18
#define KEY_PGDN 19
#define KEY_SHIFT_LEFT 20
#define KEY_SHIFT_RIGHT 21
#define KEY_DELETE 22
//#define KEY 23

#define LITERAL_KEY_ESCAPE 27

/**
 * @function getch
 * Gets a single keypress/character from the input buffer and
 * nothing else (no enter key or other thing required).
 * @returns {int} the character retrieved from the buffer.
 */
int getch() {
   struct termios oldattr, newattr;
   int ch;
   tcgetattr(STDIN_FILENO, &oldattr);
   newattr = oldattr;
   //newattr.c_lflag &= ~(ICANON | ECHO);

   // disable line-reading, echoing, ctrlc & ctrlz & ctrly, ctrlv
   newattr.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
   // disable terminal translation
   newattr.c_oflag &= ~(OPOST);
   // disable cr translation, ctrls & ctrlq
   newattr.c_iflag &= ~(ICRNL | IXON);
   
   tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
   ch = getchar();
   //read(STDIN_FILENO, &ch, 1);
   tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   return ch;
}
/*
int getch() {
   struct termios oldattr, newattr;
   int ch;
   tcgetattr(STDIN_FILENO, &oldattr);
   newattr = oldattr;
   newattr.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
   ch = getchar();
   tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   return ch;
}
*/

/**
 * @function resolveEscapeSequence
 * Detects a control sequence and determines if it matches any that we're interested in.
 *
 * I tried integrating rterm and tput with it,
 * but tput doesn't necessarily return the control sequences
 * that the keyboard will output.
 * (Which, unless I messed up and it actually does, is annoying).
 *
 * @returns {int} the index of the "special key" that the control sequence matches.
 */
int resolveEscapeSequence() {
   int matches = 1;
   string sequence = "\x1B";
   
   // vt100, xterm function key codes: https://invisible-island.net/xterm/xterm-function-keys.html
   string candidates[] = {
      // Format:
      // up     down    left    right
      // f1     f2      f3      f4
      // f5     f6      f7      f8
      // f9     f10     rsrvd   enter
      // home   end     pgup    pgdn
      // shleft shright delete   rsrvd

      // vt100:
      "\x1BOA", "\x1BOB", "\x1BOC", "\x1BOD",
      "\x1BOP", "\x1BOQ", "\x1BOR", "\x1BOS",
      "\x1BOt", "\x1BOu", "\x1BOv", "\x1BOl",
      "\x1BOw", "\x1BOx", "", "\x1BOM",
      "", "", "", "",
      "", "", "", "",

      // rxvt:
      "\x1B[A", "\x1B[B", "\x1B[C", "\x1B[D",
      "\x1B[11~", "\x1B[12~", "\x1B[13~", "\x1B[14~",
      "\x1B[15~", "\x1B[17~", "\x1B[18~", "\x1B[19~",
      "\x1B[20~", "\x1B[21~", "", "\x1BOM",
      "\x1B[7~", "\x1B[8~", "\x1B[5~", "\x1B[6~",
      "\x1B[d", "\x1B[c", "\x1B[3~", "",

      // xterm-new
      "\x1BOA", "\x1BOB", "\x1BOC", "\x1BOD",
      "\x1BOP", "\x1BOQ", "\x1BOR", "\x1BOS",
      "\x1B[15~", "\x1B[17~", "\x1B[18~", "\x1B[19~",
      "\x1B[20~", "\x1B[21~", "", "\x1BOM",
      "\x1BOH", "\x1BOF", "\x1B[5~", "\x1B[6~",
      "\x1B[1;2D", "\x1B[1;2C", "\x1B[3~", "",

      // Edge cases
      "", "", "", "",
      "\x1B[[A", "\x1B[[B", "\x1B[[C", "\x1B[[D", // Raspberry Pi
      "\x1B[[E", "", "", "", // Raspberry Pi
      "", "", "", "",
      "\x1B[H", "\x1B[F", "", "", // MacOS
      "", "", "", ""
   };

   //cout << "Sequence:";

   while (matches > 0) {
      matches = 0;
      
      // Get another character
      sequence += getch();
      //cout << sequence[sequence.length() - 1];
      
      for (size_t i = 0; i < (sizeof(candidates)/sizeof(candidates[0])); i++) {
         if (candidates[i].find(sequence) != string::npos) {
            matches++;
            if (candidates[i].length() == sequence.length()) {
               //cout << endl;
               return i % 24;
            }
         }
      }
   }
   //cout << " abort" << endl;
   return -1;
}

#endif

