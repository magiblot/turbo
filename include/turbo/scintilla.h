#ifndef TURBO_SCINTILLA_H
#define TURBO_SCINTILLA_H

// Define the standard order in which to include header files
// All platform headers should be included before Scintilla headers
// and each of these groups are then divided into directory groups.

// C standard library
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

// C++ wrappers of C standard library
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <climits>

// C++ standard library
#include <stdexcept>
#include <new>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <forward_list>
#include <algorithm>
#include <iterator>
#include <functional>
#include <memory>
#include <numeric>
#include <chrono>
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>

// Scintilla headers

// Non-platform-specific headers

// include
#include "scintilla/include/Platform.h"
#include "scintilla/include/Sci_Position.h"
#include "scintilla/include/ILoader.h"
#include "scintilla/include/ILexer.h"
#include "scintilla/include/Scintilla.h"
#include "scintilla/include/ScintillaWidget.h"
#include "scintilla/include/SciLexer.h"

// lexlib
#include "scintilla/lexlib/StringCopy.h"
#include "scintilla/lexlib/PropSetSimple.h"
#include "scintilla/lexlib/WordList.h"
#include "scintilla/lexlib/LexAccessor.h"
#include "scintilla/lexlib/Accessor.h"
#include "scintilla/lexlib/StyleContext.h"
#include "scintilla/lexlib/CharacterSet.h"
#include "scintilla/lexlib/CharacterCategory.h"
#include "scintilla/lexlib/LexerModule.h"
#include "scintilla/lexlib/CatalogueModules.h"
#include "scintilla/lexlib/OptionSet.h"
#include "scintilla/lexlib/SparseState.h"
#include "scintilla/lexlib/SubStyles.h"
#include "scintilla/lexlib/DefaultLexer.h"
#include "scintilla/lexlib/LexerBase.h"
#include "scintilla/lexlib/LexerSimple.h"
#include "scintilla/lexlib/LexerNoExceptions.h"

// src
#include "scintilla/src/Catalogue.h"
#include "scintilla/src/Position.h"
#include "scintilla/src/IntegerRectangle.h"
#include "scintilla/src/UniqueString.h"
#include "scintilla/src/SplitVector.h"
#include "scintilla/src/Partitioning.h"
#include "scintilla/src/RunStyles.h"
#include "scintilla/src/SparseVector.h"
#include "scintilla/src/ContractionState.h"
#include "scintilla/src/CellBuffer.h"
#include "scintilla/src/PerLine.h"
#include "scintilla/src/CallTip.h"
#include "scintilla/src/KeyMap.h"
#include "scintilla/src/Indicator.h"
#include "scintilla/src/XPM.h"
#include "scintilla/src/LineMarker.h"
#include "scintilla/src/Style.h"
#include "scintilla/src/ViewStyle.h"
#include "scintilla/src/CharClassify.h"
#include "scintilla/src/Decoration.h"
#include "scintilla/src/CaseFolder.h"
#include "scintilla/src/Document.h"
#include "scintilla/src/RESearch.h"
#include "scintilla/src/CaseConvert.h"
#include "scintilla/src/UniConversion.h"
#include "scintilla/src/DBCS.h"
#include "scintilla/src/Selection.h"
#include "scintilla/src/PositionCache.h"
#include "scintilla/src/FontQuality.h"
#include "scintilla/src/EditModel.h"
#include "scintilla/src/MarginView.h"
#include "scintilla/src/EditView.h"
#include "scintilla/src/Editor.h"
#include "scintilla/src/ElapsedPeriod.h"
#include "scintilla/src/AutoComplete.h"
#include "scintilla/src/ScintillaBase.h"
#include "scintilla/src/ExternalLexer.h"

#endif // TURBO_SCINTILLA_H
