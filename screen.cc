/**
 * screen.cc - Utility functions related to the screen size.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "global.h"
#include "screen.h"


/**
 * Constructor.  NOP.
 */
CScreen::CScreen()
{
}


/**
 * Destructor.  NOP.
 */
CScreen::~CScreen()
{
}


/**
 * This function will draw the appropriate screen
 * depending upon our current mode.
 */
void CScreen::refresh_display()
{
  /**
   * Get the current mode.
   */
  CGlobal *g = CGlobal::Instance();
  std::string * s = g->get_mode();

  if ( strcmp( s->c_str(), "maildir" ) == 0 )
    drawMaildir();
  else if ( strcmp( s->c_str(), "index") == 0 )
    drawIndex();
  else if ( strcmp( s->c_str(), "message" ) == 0 )
    drawMessage();
  else
    {
      clear();
      move( 3, 3 );
      printw( "UNKNOWN MODE: '%s'", s->c_str() );
    }
}


/**
 * Draw a list of folders.
 */
void CScreen::drawMaildir()
{
  //  clear();
  move(3, 3);
  printw( "Drawing maildir here ..");
}

void CScreen::drawIndex()
{
  move(3, 3);
  printw( "Drawing INDEX here ..");
}
void CScreen::drawMessage()
{
  move(3, 3);
  printw( "Drawing MESSAGE here ..");
}


/**
 * Setup the curses/screen.
 */
void CScreen::Init()
{
  /**
   * Setup ncurses.
   */
  initscr();

  /**
   * Make sure we have colours.
   */
    if (!has_colors() || (start_color() != OK)) {
	endwin();
	std::
	    cerr << "We don't have the required colour support available."
	    << std::endl;
	exit(1);
    }

    keypad(stdscr, TRUE);
    crmode();
    noecho();
    curs_set(0);
    timeout(1000);
}


/**
 * Return the width of the screen.
 */
int CScreen::width()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_col);
}


/**
 * Return the height of the screen.
 */
int CScreen::height()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_row);
}
