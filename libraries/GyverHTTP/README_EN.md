This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gyverhttp
Very simple and light http server and semiasinchronous http client
- Fast sending and receiving files
- Convenient minimalistic API

## compatibility
Compatible with all arduino platforms (used arduino functions)

## Content
- [use] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id="usage"> </a>

## Usage
### Streamsender
A quick data sender to Print, supports work with files and Progmem.Reads to the buffer and sends with blocks, which is many times faster than ordinary sending
`` `CPP
Streamsender (File & File);
Streamsender (Consta Uint8_t* Buf, Size_t Len, Bool PGM = 0);

// Data size
Size_t Length ();

// set the size of the sending unit
VOID Setblocksize (Size_T BSIZE);

// Print in print
Size_t Printto (Print & P);
`` `

### StreamReader
Quick reader of data from Stream of a certain length.Buffering and writes in blocks in the consumer, which is many times faster than usual reading
`` `CPP
StreamReader (Stream* Stream = Nullptr, Size_t Len = 0);

// Install a timaut
VOID settimeout (size_t tout);

// Set the block size
VOID Setblocksize (Size_T BSIZE);

// read to the buffer, will return True with success
Bool Readbytes (uint8_t* buf);

// Bring out to Write (uint8_t*, size_t)
TEMPLATE <TYPENAME T>
Bool Writeto (T&P);

Size_t Printto (Print & P);

// total amount of incoming data
Size_t Length ();

// Correctic Rider
Operator Bool ();

Stream* Stream;
`` `

## client
`` `CPP
Size_t Write (Uint8_t Data);
SIZE_T WRITE (COST UINT8_T* BUFFER, SIZE_T SIZE);

// ============================

// install a new host and port
VOID Sethost (Const Char* Host, Uint16_T Port);

// install a new host and port
VOID Sethost (Constress & IP, Uint16_T Port);

// Install a new client for communication
VOID setclient (: Client & Client);

// Install the server response time, silent.2000 ms
VOID settimeout (uint16_t tout);

// answers processor, requires a tick () call to loop ()
VOID Onresponse (Responsecallback CB);

// ============================

// Connect
Bool Connect ();

// Send a request
Bool Request (Constation Su :: Text & Path, Cost Su :: Text & Method, const SU :: Text & Headers, Constation Su :: Text & Payload);

// Send a request
Bool Request (Const SU :: Text & Path, Cost Su :: Text & Method = "Get", const SU :: Text & Headers = Su :: Text (), Cont Uint8_t* Payload, Size_t Length = 0);

// Start sending.Then you need to manually Print
Bool BeginSend ();

// Client is waiting for an answer
Bool ISWaiting ();

// there is a response from the server (asynkhronno)
Bool Available ();

// wait and read the response of the server (according to AVAILABLE if LONG Poll)
Response getresponspon (Headerscollector* Collector = Nullptr);

// ticker, call in LOOP to work with collbe
VOID Tick ();

// Stop the client
VOID Stop ();

// Skip the answer, remove the waiting flag, stop if the Connection Close
VOID Flush ();
`` `

### Client :: Response
`` `CPP
// Content type
SU :: Text Type ();

// The body of the answer
StreamReader & Body ();

// The answer exists
Operator Bool ();
`` `

### Server
`` `CPP
ServeR (uint16_t port);

// Launch
VOID Begin ();

// Call in Loop
VOID Tick (Headerscollector* Collector = Nullptr);

// Connect the request handler
VOID Onrequest (RequestCallback Callback);

// Send the client.Can be called several times in a row
VOID SEND (COST SU :: Text & Text, Uint16_t Code = 200, Su :: TEXT TYPE = SU :: Text ());

// Send the client the code.Should be the only answer
VOID SEND (UINT16_T CODE);

// Send the file
VOID SENDFILE (File & File, Su :: Text Type = Su :: Text (), Bool Cache = False, Bool Gzip = False);

// Send a file from the buffer
VOID SENDFILE (COST UINT8_T* BUF, SIZE_T LEN, SU :: Text Type = Su :: Text (), Bool Cache = FALSE, BOL GZIP = FALSE);

// Send a file from Progmem
VOID SENDFILE_P (COST UINT8_T* BUF, SIZE_T LEN, SU :: Text Type = SU :: Text (), Bool Cache = False, Bool Gzip = FALSE);

// mark the request as executed
Void Handle ();

// Use Cors Harders (silent inclusive)
VOID usecors (Bool Use);

// Get MIME File type along its path
const __flashstringhelper* getmime (const SU :: text & Path);
`` `

### SERVERBASE :: Request
`` `CPP
// Request method
COST SU :: Text & Method ();

// Full Url
COST SU :: Text & url ();

// Path (without parameters)
SU :: Text Path ();

// get the value of the parameter by the key
SU :: Text Param (Const SU :: Text & Key);

// Get the body of the request.Can be displayed in Print
StreamReader & Body ();
`` `

<a id="versions"> </a>

## versions
- V1.0

<a id="install"> </a>
## Installation
- The library can be found by the name ** gyverhttp ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download the library] (https://github.com/gyverlibs/gyverhttp/archive/refs/heads/main.zip) .Zip archive for manual installation:
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