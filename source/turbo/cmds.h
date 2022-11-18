#ifndef TURBO_CMDS_H
#define TURBO_CMDS_H

const ushort
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
    cmToggleComment = 114;

// Commands that cannot be deactivated.
const ushort
    cmToggleTree    = 1000,
    cmStateChanged  = 1001;

#endif // TURBO_CMDS_H
