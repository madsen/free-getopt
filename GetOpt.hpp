//--------------------------------------------------------------------
// $Id: GetOpt.hpp,v 0.1 2000/12/11 02:54:57 Madsen Exp $
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
  enum Type       { optArg, optLong, optShort     };
  typedef bool (ArgFunc)(GetOpt* getopt, const Option* option,
                         const char* asEntered,
                         Connection connected, const char* argument,
                         int* usedChars);
  struct Option
  {
    char         shortName;
    const char*  longName;
    ArgFunc*     argument;
    bool         flag;
    void*        data;
    const char*  description;
  }; // end GetOpt::Option

 protected:
  const Option*  optionList;
  int            argc;
  int            argi, chari;
  const char**   argv;
  bool           normalOnly;
  const char*    optionStart;
  const Option*  returningAll;
  char           shortOptionBuf[3];

 public:
  explicit GetOpt(const Option* aList);
  void  init(int theArgc, const char** theArgv);
  bool  nextOption(const Option*& option, const char*& asEntered);
  int   process(int theArgc, const char** theArgv);

  // Standard callback functions:
  static bool  isFlag(GetOpt* getopt, const Option* option,
                      const char*, Connection, const char*, int*);

 protected:
  void  checkReturnAll();
  const Option*  findShortOption(char option) const;
  const Option*  findLongOption(const char* option) const;
  bool  nextOption(const char*& option, Type& type);
}; // end GetOpt

#endif // INCLUDED_GETOPT_HPP
