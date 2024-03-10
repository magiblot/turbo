#ifndef TURBO_CMDS_H
#define TURBO_CMDS_H

enum : ushort
{
    cmOpenRecent = 100,
    cmEditorNext,
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
};

#endif // TURBO_CMDS_H
