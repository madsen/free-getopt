//--------------------------------------------------------------------
// $Id: GetOpt.hpp,v 0.2 2000/12/13 00:43:27 Madsen Exp $
//--------------------------------------------------------------------
//
//   Free GetOpt
//   Copyright 2000 by Christopher J. Madsen
//
//   Process command line arguments
//
//--------------------------------------------------------------------

#ifndef INCLUDED_GETOPT_HPP
#define INCLUDED_GETOPT_HPP

class GetOpt
{
 public:
  struct Option;
  enum Connection { nextArg, withEquals, adjacent };
  enum Found      { notFound, noArg, withArg      };
  enum Type       { optArg, optLong, optShort     };
  typedef bool (ArgFunc)(GetOpt* getopt, const Option* option,
                         const char* asEntered,
                         Connection connected, const char* argument,
                         int* usedChars);
  typedef void (ErrorFunc)(const char* option, const char* message);

  struct Option
  {
    char         shortName;
    const char*  longName;
    Found*       found;
    ArgFunc*     function;
    bool         requireArg;
    void*        data;
  }; // end GetOpt::Option

  bool           error;
  ErrorFunc*     errorOutput;
  const char*    optionStart;

 protected:
  const Option*  optionList;
  int            argc;
  int            argi, chari;
  const char**   argv;
  bool           normalOnly;
  const Option*  returningAll;
  char           shortOptionBuf[3];

 public:
  explicit GetOpt(const Option* aList);
  void  init(int theArgc, const char** theArgv);
  bool  nextOption(const Option*& option, const char*& asEntered);
  int   process(int theArgc, const char** theArgv);
  void  reportError(const char* option, const char* message);

  // Standard callback functions:
  static void  printError(const char* option, const char* message);

 protected:
  void  checkReturnAll();
  const Option*  findShortOption(char option) const;
  const Option*  findLongOption(const char* option) const;
  bool  nextOption(const char*& option, Type& type);
}; // end GetOpt

#endif // INCLUDED_GETOPT_HPP
