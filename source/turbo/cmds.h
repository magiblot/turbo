#ifndef TURBO_CMDS_H
#define TURBO_CMDS_H

enum : ushort
{
    cmOpenRecent    = 100,
    cmEditorNext    = 101,
    cmEditorPrev    = 102,
    cmToggleWrap    = 103,
    cmToggleLineNums= 104,
    cmSearchPrev    = 105,
    cmToggleIndent  = 106,
    cmTreeNext      = 107,
    cmTreePrev      = 108,
    cmCloseEditor   = 109,
    cmRename        = 110,
    cmSelUppercase  = 111,
    cmSelLowercase  = 112,
    cmSelCapitalize = 113,
    cmToggleComment = 114,
    cmGoToLine      = 115,
};

// Commands that cannot be deactivated.

enum : ushort
{
    cmToggleTree    = 1000,
    cmStateChanged  = 1001,
    cmFindFindBox   = 1002,
    cmFindGoToLineBox   = 1003,
    cmCloseView     = 1004,
    cmSearchIncr    = 1005,
    cmFindReplaceBox= 1006,
    cmReplaceOne    = 1007,
    cmReplaceAll    = 1008,
};

#endif // TURBO_CMDS_H
