#ifndef TURBO_HELP_H
#define TURBO_HELP_H

class TGroup;

class TurboHelp
{
public:

    static void executeAboutDialog(TGroup &owner) noexcept;
    static void showOrFocusHelpWindow(TGroup &owner) noexcept;
};

#endif // TURBO_HELP_H
