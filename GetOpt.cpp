//--------------------------------------------------------------------
// Free GetOpt 1.0
//
// Copyright 2000 by Christopher J. Madsen
//
// Process command line arguments
//
// Free GetOpt is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Free GetOpt is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// As a special exception, if you link Free GetOpt with other files to
// produce an executable, this does not by itself cause the resulting
// executable to be covered by the GNU General Public License.  Your
// use of that executable is in no way restricted on account of linking
// the Free GetOpt code into it.  However, if you link a modified
// version of Free GetOpt to your executable and distribute the
// executable, you must make your modifications to Free GetOpt publicly
// available as machine-readable source code.
//
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
//
// This exception applies only to the code released under the name
// Free GetOpt.  If you copy code from other programs into a copy of
// Free GetOpt, as the General Public License permits, the exception
// does not apply to the code that you add in this way.  To avoid
// misleading anyone as to the status of such modified files, you must
// delete this exception notice from them.
//
// If you write modifications of your own for Free GetOpt, it is your
// choice whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
//--------------------------------------------------------------------

#ifndef GETOPT_NO_STDIO
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "GetOpt.hpp"

static const char longOptionStart[] = "--";

//====================================================================
// Standard argument callback functions:
//
// An argument callback function is called when GetOpt finds the
// option that specified it.  The usual behavior is to validate the
// argument and copy it to the location specified by option->data.
// However, the function can do anything it wants.  It should return
// true if it found an argument, or false if it did not.
//
// To report an error, the callback should set getopt->error to true,
// or call getopt->reportError, which does that automatically.  It
// should then return false.
//
// Input:
//   getopt:
//     The GetOpt object which is calling the function
//   option:
//     The GetOpt::Option which we are processing
//   asEntered:
//     The option as the user entered it
//   connected:
//     The way the argument (if any) was connected to the option:
//       nextArg:     The next command-line argument (or no argument)
//       withEquals:  Connected to the option by an equals sign
//       adjacent:    Adjacent to the option (single-char option only)
//   argument:
//     The argument to the option (if any)
//     May be NULL, which means there was no argument.
//       This is because some callbacks may not care about the
//       argument, but may want to do something just because the
//       option was found.  connected will be nextArg in this case.
//     If connected is withEquals, then argument[-1] is the equals sign.
//   usedChars:
//     Most callback functions can ignore this parameter.  Use it only
//     if you want to implement the following behavior.  If usedChars
//     is not NULL, then we are processing an argument that was
//     adjacent to a single-character option.  If the function would
//     like to use only some of the characters in argument and allow
//     the rest to be processed as more single-character options, it
//     should set *usedChars to the number of characters used.
//     *usedChars is always initialized to -1, which means that the
//     entire argument was used.
//
// Return Value:
//   true:   The argument was processed
//   false:  The argument was not used, or an error occurred
//
// Note:
//   The standard callbacks insist that an optional argument must be
//   connected to the option (ie, they return false if connected is
//   nextArg and the argument was not required).  You can change this
//   behavior by using your own callbacks instead of the standard ones.
//
//--------------------------------------------------------------------
// Process a floating-point argument:
//
// option->data must point to a double.

bool GetOpt::isFloat(GetOpt* getopt, const Option* option,
                     const char* asEntered,
                     Connection connected, const char* argument,
                     int* usedChars)
{
  if (!argument ||
      ((connected == nextArg) && !(option->flag & GetOpt::needArg)))
    return false; // No argument or non-connected optional argument

  char*  end;
  *reinterpret_cast<double*>(option->data) = strtod(argument, &end);

  if (*end) {
    getopt->reportError(asEntered, " requires a numeric argument");
    return false;
  }

  return true;
} // end GetOpt::isFloat

//--------------------------------------------------------------------
// Process an integer argument:
//
// option->data must point to a long.

bool GetOpt::isLong(GetOpt* getopt, const Option* option,
                   const char* asEntered,
                   Connection connected, const char* argument,
                   int* usedChars)
{
  if (!argument ||
      ((connected == nextArg) && !(option->flag & GetOpt::needArg)))
    return false; // No argument or non-connected optional argument

  char*  end;
  *reinterpret_cast<long*>(option->data) = strtol(argument, &end, 0);

  if (*end) {
    getopt->reportError(asEntered, " requires an integer argument");
    return false;
  }

  return true;
} // end GetOpt::isLong

//--------------------------------------------------------------------
// Process a string argument:
//
// option->data must point to a const char*.

bool GetOpt::isString(GetOpt* getopt, const Option* option,
                   const char* asEntered,
                   Connection connected, const char* argument,
                   int* usedChars)
{
  if (!argument ||
      ((connected == nextArg) && !(option->flag & GetOpt::needArg)))
    return false; // No argument or non-connected optional argument

  if (option->data)
    *reinterpret_cast<const char**>(option->data) = argument;

  return true;
} // end GetOpt::isString

//====================================================================
// Class GetOpt:
//
// Member Variables:
//   error:
//     true if an error has occurred during option processing
//     false if everything is ok
//   errorOutput:
//     A function which is called to display errors
//     Set to GetOpt::printError by the GetOpt constructor.
//     If NULL, errors are reported only by setting error to true.
//   optionStart:
//     A string containing the characters that indicate options
//     Set to "-" by the GetOpt constructor.
//     Must contain '-' if you expect long options to work.
//     This applies only to single-character options; "--" is always
//     the long option indicator.
//     Programs that want to accept DOS-style options should set this
//     to "-/".  Note that this string is not disposed of by the
//     GetOpt object.  It must continue to exist as long as the GetOpt
//     object does.  (Normally, you would set it to point to a string
//     literal.)
//
// Protected Member Variables:
//   optionList:
//     The array of GetOpt::Option objects passed to the constructor
//   argc, argv:
//     The parameters passed to main (or similar)
//   argi:
//     The index in argv of the argument currently being processed
//   chari:
//     If non-zero, the index in argv[argi] of the option character
//     currently being processed (for single-character option bundles).
//   normalOnly:
//     True means that all arguments yet to be processed are not options.
//   returningAll:
//     Points to the GetOpt::Option that corresponds to normal
//     non-option arguments, or NULL if GetOpt is to process only
//     options.
//   shortOptionBuf:
//     Used when processing single-character option bundles
//
//--------------------------------------------------------------------
// Constructor:
//
// Input:
//   aList:
//     An array of GetOpt::Option objects that define the options to
//     look for.  This array is not copied, and must continue to exist
//     as long as the GetOpt object does.

GetOpt::GetOpt(const Option* aList)
: error(false),
#ifdef GETOPT_NO_STDIO
  errorOutput(NULL),               // No stdio, can't print errors
#else
  errorOutput(GetOpt::printError), // Print error messages to stderr
#endif
  optionStart("-"),
  optionList(aList),
  argc(0),
  argi(0), chari(0),
  argv(NULL),
  normalOnly(false)
{
  checkReturnAll();
} // end GetOpt::GetOpt

//--------------------------------------------------------------------
// Standard callback function for printing error messages:
//
// You should generally not call this function directly.  Use
// GetOpt::reportError to report errors, which calls the errorOutput
// function.
//
// The GetOpt constructor sets errorOutput to GetOpt::printError.
//
// Input:
//   option:   The option the user typed
//   message:  The error message to display

#ifndef GETOPT_NO_STDIO
void GetOpt::printError(const char* option, const char* message)
{
  fputs(option, stderr);
  fputs(message, stderr);
  putc('\n', stderr);
} // end GetOpt::printError
#endif // not GETOPT_NO_STDIO

//--------------------------------------------------------------------
// Prepare to process a command line:
//
// This function also goes through the option list and sets all found
// entries to notFound.
//
// Input:
//   theArgc:
//     The number of elements in theArgv
//   theArgv:
//     The program name & command line arguments.
//     theArgv[0] (the program name) is not used and may be NULL.
//     This array is not copied, and must exist as long as the GetOpt
//     object is in use.

void GetOpt::init(int theArgc, const char** theArgv)
{
  argc = theArgc;
  argv = theArgv;
  argi = chari = 0;
  error = normalOnly = false;

  const Option* op = optionList;

  while (op->shortName || op->longName) {
    if (op->found)
      *(op->found) = notFound;
    ++op;
  }
} // end GetOpt::init

//--------------------------------------------------------------------
// Set the returningAll member variable:

void GetOpt::checkReturnAll()
{
  const Option* op = optionList;

  while (op->shortName || op->longName) {
    if (op->longName && !*(op->longName)) {
      returningAll = op;
      return;
    }
    ++op;
  }

  returningAll = NULL;
} // end GetOpt::checkReturnAll

//--------------------------------------------------------------------
// Determine what Option a long option refers to:
//
// Looks first for an exact match, then for an approximate one.
//
// Input:
//   option:
//     The option the user typed (without the leading "--", but with
//     any trailing argument attached by an '=')
//
// Returns:
//   A pointer to the corresponding GetOpt::Option
//   NULL if no option matched
//   If more than one option might match, calls reportError and then
//   returns NULL.

const GetOpt::Option* GetOpt::findLongOption(const char* option)
{
  const Option* op = optionList;
  const Option*  possibleMatch = NULL;
  bool  ambiguous = false;

  while (op->shortName || op->longName) {
    if (op->longName) {
      bool partial = false;
      const char* u = option;
      const char* o = op->longName;
      for (;;) {
        if (!*u || *u == '=') { // Reached end of user entry
          if (*o || partial) {
            if (possibleMatch) ambiguous = true; // 2 possible matches
            possibleMatch = op;
            break;              // Found possible match, keep going
          } else return op;     // Exact match!
        } else if (!*o) {
          break;                // Not a match
        } else if (*u == *o) {
          ++u; ++o;
        } else if (*u == '-') {
          partial = true;
          while (*(++o))
            if (*o == '-') break;
        } else
          break;                // Not a match
      } // end forever
    } // end if option has a longName
    ++op;
  } // end while more options

  // We didn't find an exact match, what about a possible one?
  if (possibleMatch) {
    if (ambiguous) { // More than one possible match found
      reportError(argv[argi], " is ambiguous");
      return NULL;
    }
    return possibleMatch;       // Found just one possible match
  } // end if possibleMatch

  return NULL;                  // Found no matches at all
} // end GetOpt::findLongOption

//--------------------------------------------------------------------
// Determine what Option a short option refers to:
//
// Input:
//   option:  The option to look for
//
// Returns:
//   A pointer to the corresponding GetOpt::Option
//   NULL if no option matched

const GetOpt::Option* GetOpt::findShortOption(char option) const
{
  const Option* op = optionList;

  while (op->shortName || op->longName) {
    if (op->shortName == option)
      return op;
    ++op;
  }

  return NULL;
} // end GetOpt::findShortOption

//--------------------------------------------------------------------
// Find the next argument to process:
//
// If returningAll is not NULL, this returns all arguments in order.
// Otherwise, the options (and their arguments) are moved before the
// non-option arguments, but this function still returns all arguments.
//
// Output:
//   option:  Points to the option (after any option start characters)
//   type:    The type of option found
//     optArg:    Normal argument (not an option)
//     optLong:   A long option
//     optShort:  A single-character option
//   posArg:  The index of a possible argument for this option
//            0 means use argi+1
//
// Returns:
//   true:   Found an option or normal argument to be returned
//   false:  No more arguments (option & type are undefined in this case)

bool GetOpt::nextOption(const char*& option, Type& type, int& posArg)
{
  posArg = 0;

  if (chari) {
    if (argv[argi][++chari]) {
      option = argv[argi] + chari;
      type = optShort;
      return true;
    }
    chari = 0; // We've reached the end of a short option bundle
  } // end if processing a short option bundle

  if (++argi >= argc) return false; // No more options

  const char*  arg = argv[argi];

  if (!normalOnly) {
    if (*arg && strchr(optionStart, *arg)) {
     foundOptionStart:
      if (!strncmp(longOptionStart, arg, sizeof(longOptionStart)-1)) {
        option = arg+2;
        type = optLong;
        return true;
      }
      if (arg[1]) {
        chari  = 1;
        option = arg+1;
        type   = optShort;
        return true;
      }
    } // end if arg begins with option start character

    if (!returningAll) { // Look for another option argument
      for (int i = argi+1; i < argc; ++i) {
        if (argv[i][0] && strchr(optionStart, argv[i][0])) {
          // We found another option, move it before the other args:
          posArg = i + 1;
          arg = argv[i];
          while (--i >= argi)
            argv[i+1] = argv[i];
          argv[argi] = arg;
          goto foundOptionStart;
        } // end if we found another option argument
      } // end for remaining arguments
      normalOnly = true; // There are no more option arguments
    } // end if not returning all arguments in order
  } // end if still looking for options

  option = arg;
  type   = optArg;
  return true;
} // end GetOpt::nextOption

//--------------------------------------------------------------------
// Return the next argument to process:
//
// If returningAll is not NULL, this returns all arguments in order.
// Otherwise, it returns only the options, which are moved before the
// non-option arguments, and returns false when it runs out of options.
//
// Output:
//   option:     Points to the GetOpt::Option selected by the user
//   asEntered:  The actual text the user typed
//
// Returns:
//   true:   Found an option or normal argument to be returned
//   false:  No more arguments match the option list

bool GetOpt::nextOption(const Option*& option, const char*& asEntered)
{
  const char* arg;
  Type        type;
  int         posArg;

 nextArg:
  if (!nextOption(arg, type, posArg)) return false;

  if (type == optLong && !*arg) {
    normalOnly = true;
    goto nextArg;
  } // end if "--" by itself (no more options)

  if (type == optShort) {
    shortOptionBuf[0] = argv[argi][0];
    shortOptionBuf[1] = *arg;
    shortOptionBuf[2] = 0;
    asEntered = shortOptionBuf;
    option = findShortOption(*arg);
  } else {
    asEntered = argv[argi];
    if (type == optArg)
      option = returningAll;
    else
      option = findLongOption(arg);
  }

  if (!option) {
    if ((type != optArg) && !error)
      reportError(asEntered, " is not a recognized option");
    return false;
  }

  if (option->found && *option->found &&
      !(option->flag & GetOpt::repeatable)) {
    reportError(asEntered, " cannot be repeated");
    return false;
  }

  Found  found = noArg;

  if (option->function) {
    int   usedChars = -1;
    int*  mayUseChars = NULL;
    Connection  connect = nextArg;

    if (!posArg) posArg = argi + 1;

    if (type != optArg) {
      if ((type == optShort) && argv[argi][chari+1]) {
        mayUseChars = &usedChars;
        arg = argv[argi] + chari + 1;
        if (*arg == '=') {
          ++arg;
          connect = withEquals;
        } else
          connect = adjacent;
      } else if ((type == optLong) && (arg = strchr(arg, '='))) {
        ++arg;                    // Skip over equals
        connect = withEquals;
      } else if (posArg < argc)
        arg = argv[posArg];
      else if (option->flag & GetOpt::needArg) {
        reportError(asEntered, " requires an argument");
        return false;
      } else
        arg = NULL;
    } // end if option (not normal argument)

    if ((*(option->function))(this, option, asEntered, connect, arg,
                              mayUseChars)) {
      found = withArg;
      if (usedChars >= 0)
        chari += usedChars;
      else {
        chari = 0;
        if ((type != optArg) && (connect == nextArg) && arg) {
          ++argi;
          // If we moved the option, we need to move the argument:
          while (--posArg >= argi)
            argv[posArg+1] = argv[posArg];
          argv[argi] = arg;
        } // end if used next argument
      } // end else didn't use just some of the characters in a bundle
    } // end if found option
    else if ((option->flag & GetOpt::needArg) && !error) {
      reportError(asEntered, " requires an argument");
      return false;
    }
  } // end if option has function to call

  if (option->found)
    *(option->found) = found;

  return true;
} // end GetOpt::nextOption

//--------------------------------------------------------------------
// Report (and possibly print) an error:
//
// This sets error to true and then calls the errorOutput function (if
// that is not NULL).  The errorOutput function (normally printError)
// is expected to print both its arguments (see printError).
//
// Your callback functios should call reportError to report any errors
// they encounter.  If printing error messages to stderr is not
// appropriate for your application, then set errorOutput to a
// suitable function, or to NULL to suppress error messages
// altogether.
//
// Input:
//   option:   The option that caused a problem
//   message:  The problem that was encountered

void GetOpt::reportError(const char* option, const char* message)
{
  error = true;
  if (errorOutput)
    (*errorOutput)(option, message);
} // end GetOpt::reportError

//--------------------------------------------------------------------
// Process a command line:
//
// This is the standard entry point for GetOpt.  Most programs will
// just call the constructor to set up the option list and then call
// process, relying on callbacks to store the results.
//
// Input:
//   theArgc:
//     The number of elements in theArgv
//   theArgv:
//     The program name & command line arguments.
//     theArgv[0] (the program name) is not used and may be NULL.
//     This array is not copied, and must exist as long as the GetOpt
//     object is in use.
//
// Returns:
//   The index (into theArgv) of the first argument that was not
//   processed by GetOpt.  If this is >= theArgc, then all arguments
//   were processed.  (This is the same value that would be returned
//   by currentArg().)

int GetOpt::process(int theArgc, const char** theArgv)
{
  const Option* option;
  const char* asEntered;

  init(theArgc, theArgv);
  while (nextOption(option, asEntered))
    ;

  return argi;
} // end GetOpt::process
