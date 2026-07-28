#ifndef TURBO_CMDS_H
#define TURBO_CMDS_H

// Commands 0..255 support being disabled while the rest don't. However,
// command ranges 0..99 and 256..999 are reserved by Turbo Vision, so custom
// commands must be defined in the ranges 100..255 and 1000..65535.

enum : ushort
{
    // Commands that can be disabled.
    cmEditorNext = 100,
    cmEditorPrev,
    cmToggleWrap,
    cmToggleLineNums,
    cmSearchPrev,
    cmToggleIndent,
    cmTreeNext,
    cmTreePrev,
    cmCloseEditor,
    cmRename,
    cmSelUppercase,
    cmSelLowercase,
    cmSelCapitalize,
    cmToggleComment,
    cmGoToLine,
    cmReplaceOne,
    cmReplaceAll,

    cmUseLanguage = 200,
    cmUseLanguageMax = 400      // Support for selecting 200 languages. The command for selecting language ABC is (cmUseLanguage + turbo::Language::ABC)
};

// Commands that cannot be deactivated.

enum : ushort
{
    cmToggleTree = 1000,
    cmStateChanged,
    cmFindFindBox,
    cmFindGoToLineBox,
    cmCloseView ,
    cmSearchIncr,
    cmFindReplaceBox,
    cmClearReplace,
    cmAbout,
    cmFindHelpWindow,
};

#endif // TURBO_CMDS_H
