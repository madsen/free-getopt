//--------------------------------------------------------------------
// $Id: GetOpt.cpp,v 0.1 2000/12/11 02:54:30 Madsen Exp $
//--------------------------------------------------------------------
//
//   Free GetOpt
//   Copyright 2000 by Christopher J. Madsen
//
//   Process command line arguments
//
//--------------------------------------------------------------------

#include "GetOpt.hpp"
#include <string.h>

static const char longOptionStart[] = "--";

GetOpt::GetOpt(const Option* aList)
: optionList(aList),
  argc(0),
  argi(0), chari(0),
  argv(NULL),
  normalOnly(false),
  optionStart("-")
{
  checkReturnAll();
} // end GetOpt::GetOpt

void GetOpt::init(int theArgc, const char** theArgv)
{
  argc = theArgc;
  argv = theArgv;
  argi = chari = 0;
  normalOnly = false;
} // end GetOpt::init

void GetOpt::checkReturnAll()
{
  const Option* op = optionList;

  while (op->shortName || op->longName)
    ++op;

  if (op->argument)
    returningAll = op;
  else
    returningAll = NULL;
} // end GetOpt::checkReturnAll

//--------------------------------------------------------------------
const GetOpt::Option* GetOpt::findLongOption(const char* option) const
{
  const Option* op = optionList;
  // FIXME add abbreviations and arguments

  while (op->shortName || op->longName || op->argument) {
    if (!stricmp(op->longName, option))
      return op;                // Found exact match
    ++op;
  }

  return NULL;
} // end GetOpt::findLongOption

//--------------------------------------------------------------------
const GetOpt::Option* GetOpt::findShortOption(char option) const
{
  const Option* op = optionList;

  while (op->shortName || op->longName || op->argument) {
    if (op->shortName == option)
      return op;
    ++op;
  }

  return NULL;
} // end GetOpt::findShortOption

//--------------------------------------------------------------------
bool GetOpt::nextOption(const char*& option, Type& type)
{
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

    if (!returningAll) {
      for (int i = argi+1; i < argc; ++i) {
        if (argv[i][0] && strchr(optionStart, argv[i][0])) {
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
bool GetOpt::nextOption(const Option*& option, const char*& asEntered)
{
  const char* arg;
  Type        type;

 nextArg:
  if (!nextOption(arg, type)) return false;

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

  if (!option) return false;

  if (option->argument) {
    int   usedChars = 0;
    int*  mayUseChars = NULL;
    Connection  connect = nextArg;

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
      } else if (argi+1 < argc)
        arg = argv[argi+1];
      else
        arg = NULL;
    }

    if ((*(option->argument))(this, option, asEntered, connect, arg,
                              mayUseChars)) {
      if (usedChars)
        chari += usedChars;
      else {
        chari = 0;
        if ((type != optArg) && (connect == nextArg))
          ++argi;
      }
    } // end if found option
  } // end if option has function to call

  return true;
} // end GetOpt::nextOption

int GetOpt::process(int theArgc, const char** theArgv)
{
  const Option* option;
  const char* asEntered;

  init(theArgc, theArgv);
  while (nextOption(option, asEntered))
    ;

  return argi;
} // end GetOpt::process

//--------------------------------------------------------------------
bool GetOpt::isFlag(GetOpt* getopt, const Option* option,
                      const char*, Connection, const char*, int*)
{
  *reinterpret_cast<bool*>(option->data) = option->flag;
  return false;
} // end GetOpt::isFlag

