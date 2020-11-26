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
#include "Platform.h"

#include "Sci_Position.h"
#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"
#include "ScintillaWidget.h"

#include "SciLexer.h"

// lexlib
#include "StringCopy.h"
#include "PropSetSimple.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "CharacterCategory.h"
#include "LexerModule.h"
#include "CatalogueModules.h"
#include "OptionSet.h"
#include "SparseState.h"
#include "SubStyles.h"
#include "DefaultLexer.h"
#include "LexerBase.h"
#include "LexerSimple.h"
#include "LexerNoExceptions.h"

// src
#include "Catalogue.h"

#include "Position.h"
#include "IntegerRectangle.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "SparseVector.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "PerLine.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "RESearch.h"
#include "CaseConvert.h"
#include "UniConversion.h"
#include "DBCS.h"
#include "Selection.h"
#include "PositionCache.h"
#include "FontQuality.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "ElapsedPeriod.h"

#include "AutoComplete.h"
#include "ScintillaBase.h"

#include "ExternalLexer.h"
