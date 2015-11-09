/**
 * screen.h - Utility functions related to the screen.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#pragma once

#include <vector>
#include <unordered_map>
#include "utfstring.h"

/**
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen
{

public:

    /**
     * Draw/Refresh the display.
     */
    void refresh_display();

    /**
     * Setup the screen.
     */
    void setup();

    /**
     * Return the width of the screen.
     */
    static int width();

    /**
     * Return the height of the screen.
     */
    static int height();

    /**
     * Clear the main display area, leaving the status-area alone.
     */
    static void clear_main();

    /**
     * Clear the status-line of the screen.
     */
    static void clear_status();

    /**
     * Display a line of text on the screen, interpreting ${colour:red bold}
     * type codes.
     */
    void display_styled_line(int screenLine, std::string line,
                                             const std::string &default_colour);

    /**
     * Some simple remapping of keyboard input.
     */
    static const char *get_key_name( gunichar c, bool isKeyCode);

    /**
     * Choose a single item from a small selection, graphically.
     *
     * (This is used to resolve ambiguity in TAB-completion.)
     */
    static std::string choose_string( std::vector<std::string> choices );

    /**
     * Get all possible completions for the given token.
     */
    static std::vector<std::string> get_completions( std::string token );

    /**
     * Read a line of input via the status-line.
     */
    static UTFString get_line();

    /**
     * Get access to this singleton instance.
     */
    static CScreen *Instance();

private:

    static CScreen *pinstance;

    /**
     * Constructor.  NOP.
     */
    CScreen();

    /**
     * Destructor.  NOP.
     */
    ~CScreen();

    /**
     * The segment of the screen the highlighted row is within.
     */
    enum vectorPosition
    {
        TOP,
        MIDDLE,
        BOTTOM,
        NONE
    };

    /**
     * Per-mode drawing primitives.
     */
    void drawMaildir();
    void drawIndex();
    void drawMessage();
    void drawText();

    /**
     * Lookup the curses attribute for the given string.
     */
    int lookup_curses_attribute( std::string *name, int default_val = A_STANDOUT );

    /**
     * Colour-maps.
     */
    std::unordered_map<std::string, int> m_colours;

};
