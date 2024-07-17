This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gyver Template Library
A set of template tools
- smart signs
- Dynamic arrays
- Buffers

## compatibility
Compatible with all arduino platforms (used arduino functions)

## Content
- [use] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id="usage"> </a>

## Usage
## gtl :: Array_x
Dynamic array.It has three behavior models when transferring from one object to another:
- `Array_copy` - creates an independent copy (like Arduino String)
- `Array_Shared` - creates a dependent copy, changes in one array are reflected on the other
- `Array_uniq` - completely goes to the new object

`` `CPP
T* buf ();
size_t size ();
Operator t*();
Explicit Operator Bool ();

// change the size in the number of elements t
Bool Resize (Size_T Newsize);

// Delete the buffer
VOID Reset ();
`` `

## gtl :: stack_x
The dynamic array, stores data linearly, allows you to add and remove it.Has several options:
- `stack_ext` - External buffer
- `stack_static` - internal static buffer
- `stack_copy` - a dynamic buffer with semantics` Array_copy`
- `stack_shared` - a dynamic buffer with semantics` Array_Shared`
- `stack_uniq` - a dynamic buffer with semantics` Array_uniq`

`` `CPP
// Add to the end
Bool Push (const t & val);

// Add to the end
Bool Operator+= (Consta T & Val);

// get from the end and delete
T Pop ();

// read from the end without deleting
T & Peek ();

// Add to the beginning
Bool Shift (const t & val);

// get from the beginning and delete
T Unshift ();

// read from the beginning without deleting
T & unpek ();

// Delete the element.Negative - from the end
Bool Remove (int IDX);

// insert the element to the index (the Length () index is allowed)
Bool Insert (Intx, Const T&D);

// add another array to the end
Bool Concat (const stock_ext & st);

// add another array to the end
Bool Operator+= (Const Stack_ext & ST);

// Fill in the value (on capacity)
VOID Fill (const t & val);

// initialize, call designers (on capacity)
VOID Init ();

// Clean (set length 0)
Void Clear ();

// amount of elements
Size_t Length ();

// Set the number of elements
VOID Setlength (Size_T Newlen);

// there is a place to add
Bool Canadd ();

// capacity, elements
Size_t Capacy ();

// The position of the element (-1 if not found)
InteXOF (COST T&LAL);

// contains an element
Bool Includes (const t & val);

// Delete in value (true if there is no element)
Bool Removebyval (Constation T & Val);

// Get an element under the index.Negative - from the end
T&Get (int IDX);

// Get an element under the index.Negative - from the end
T & operator [] (int IDX);

// access to the buffer
inline t* buf ();

// buffer exists
Bool Valid ();

// buffer exists
Explicit Operator Bool ();

// for dynamic
// reserve memory in the number of elements
Bool Reserve (Size_T CAP);

// Delete the buffer
VOID Reset ();
`` `

## gtl :: fifo_x
FIFO buffer
- `fifo_ext` - external buffer
- `fifo_static` - internal static buffer

`` `CPP
// connect the buffer
VOID SetBuffer (T* Buf, Uint8_t Capacy);

// entry in a buffetriver.Will return True with a successful recording
Bool Write (const t & val);

// buffer full
Bool ISFull ();

// buffer is empty
Inline Bool ISEMPTY ();

// Reading from the buffer
T Read ();

// returns extreme value without removal from the buffer
T peek ();

// The number of unread elements
uint8_t available ();

// buffer size
uint8_t size ();

// Clean
Void Clear ();

T* buffer;
`` `

<a id="versions"> </a>

## versions
- V1.0

<a id="install"> </a>
## Installation
- The library can be found by the name ** gtl ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download library] (https://github.com/gyverlibs/gtl/archive/refs/heads/main.zip). Zip archive for manual installation:
- unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
- unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
- unpack and put in *documents/arduino/libraries/ *
- (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%BD%D0%BE%BE%BE%BED0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id="feedback"> </a>

## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code