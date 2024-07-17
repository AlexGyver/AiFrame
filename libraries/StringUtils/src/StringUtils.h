#pragma once

#include "utils/PrintString.h"
#include "utils/String.h"
#include "utils/Text.h"
#include "utils/TextList.h"
#include "utils/TextParser.h"
#include "utils/Value.h"
#include "utils/convert/b64.h"
#include "utils/convert/convert.h"
#include "utils/convert/qwerty.h"
#include "utils/convert/unicode.h"
#include "utils/convert/url.h"
#include "utils/hash.h"
#include "utils/list.h"
#include "utils/parser.h"
#include "utils/splitter.h"

using su::Splitter;
using su::SplitterT;
using su::StringExt;
using su::StringStatic;
using su::Text;
using su::TextList;
using su::TextListT;
using su::TextParser;
using su::Value;

using su::SH;
using su::SH32;
using su::SL;

// legacy
namespace sutil = su;
namespace su {
using AnyText = Text;
using AnyValue = Value;
using AnyTextList = TextList;
}  // namespace su