///////////////////////////////////////////////////////////////////////////////
// html2xhtml.cpp
///////////////////////////////////////////////////////////////////////////////
// A program to help convert old style html files to new style xhtml.  Written
// using c style structure (not a class) since it would be silly to build a 
// class structure for this simple of a task.
//
// Author: Ben Anderson
// Date: April 05, 2005
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <ostream>
#include <istream>
#include <fstream>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <set>
using namespace std;

map<string, map<string, int> > g_words;

class Arguments
{
public:
  bool singleFile;
  string outFile;
  Arguments(bool singleFile, string outFile) { 
    this->singleFile = singleFile; 
  };
};

void PrintSyntax();
void PrintDebug();
void ProcessStream(istream& in, const string& out);

// Print out how to use this program from the command line
void PrintSyntax()
{
  cout 
    << "usage: spasmifytext [OPTION]... [FILE]..." << endl
    << "spasmifytext is a utility to convert textfiles to multi-dimensional \n"
    << "data points in enchilada data format.  Each dimension corresponds to\n"
    << "a different word.  The words are listed in the description of the \n"  
    << "collection.  A new set of dimensions is calculated each time you run\n"
    << "spasmifytext based upon the words in the input files."
    << endl << endl
    << "  -s, --single-file           output to single-file format\n" 
    << "  --output-file=FILENAME      the name of the datafile to output to.\n"
    << "                                if no file is specified, output\n"
    << "                                will be sent to a.edmf/edsf\n"
    << endl
    << "All options with arguments require them." << endl
    << "Report bugs to <andersbe@gmail.com>." << endl
    << endl << endl;
}

void PrintDebug(set<string> filenames, Arguments args)
{
  cout << "Arguments: " << endl
       << "\targs.singleFile = " << args.singleFile
       << "\targs.outFile = " << args.outFile << endl;

  cout << "filenames:" << endl;
  set<string>::iterator fIter;
  for (fIter = filenames.begin(); fIter != filenames.end(); fIter++)
    {
      cout << "    Input file:   " << *fIter << endl;
    }
  cout << "Output file:  " << args.outFile << endl;
}

/**
 * Takes an open input stream and an open output stream and processes
 * the input stream, ouputting the results to the output stream.
 */
void ProcessStream(istream& in, const string& name)
{
  string temp;
  map<string, map<string, int> >::iterator iter1;
  map<string, int>::iterator iter2;
  while(in >> temp)
  {
  	iter1 = g_words.find(temp);
  	if (iter1 != g_words.end())
    {
        iter2 = iter1->second.find(name);
        if (iter2 != iter1->second.end())
	    {
	      iter2->second++;
	    }
        else
	    {
	      iter1->second.insert(make_pair(name,1));
	    }
	}
    else 
    {
        map<string, int> tempMap;
        tempMap.insert(make_pair(name,1));
        g_words.insert(make_pair(temp,tempMap));
    }
  } 
}

int main(int argc, char* argv[])
{

  set<string> filenames;
  string outfile;

  bool help = false, unrecognized = false;
  

  Arguments args(false, "");

  for (int i = 1; i < argc; i++)
    {
      // it's an option
      if (argv[i][0] == '-')
	{
	  // it's a long-form option
	  if (argv[i][1] == '-')
	    {
	      if (strcmp("--single-file",argv[i]) == 0)
		{
		  args.singleFile = true;
		}
	      else if (strcmp("--help",argv[i]) == 0)
		{
		  help = true;
		}
	      else if (strncmp("--output-file=",argv[i],
			       strlen("--output-file=")) == 0)
		{
		  args.outFile = argv[i];
		  args.outFile.erase(0,strlen("--output-file="));

		  if (args.outFile.length() == 0)
		    unrecognized = true;
		}
	      else
		{
		  unrecognized = true;
		}
	    }
	  else
	    {
	      int arg = 1;
	      while (argv[i][arg] != '\0')
		{
		  switch (argv[i][arg])
		    {
		    case 's':
		      args.singleFile = true;
		      break;
		    case 'h':
		      help = true;
		      break;
		    default:
		      unrecognized = true;
		    }
		  arg++;
		}
	    }
	}
      else
	{
	  filenames.insert(string(argv[i]));
	}
    }

  if (filenames.size() == 0)
    unrecognized = true;

  // A command was malformed, or they asked for help, just print usage and 
  // don't do anything else
  if (unrecognized || help)
    {
      if (unrecognized)
	{
	  cout << "One of your arguments was not recognized.  The correct "
	       << "syntax is:" << endl << endl;
	}

      PrintSyntax();

      exit(0);

    }
    
  if (filenames.size() == 0)
    {
      ProcessStream(cin, "STDIN");
    }
  else
    {
      set<string>::iterator fIter;
      for (fIter = filenames.begin(); fIter != filenames.end(); fIter++)
	{
	  ifstream fin(fIter->c_str());

	  ProcessStream(fin, *fIter);
	  fin.close();
	}
    }

  
  ofstream* out;
  if (args.outFile != "")
    {
      out = new ofstream(args.outFile.c_str());
      *out << args.outFile << "^^^^^^^^" << endl;
    }
  
  else 
    {
      if (args.singleFile)
	{
	  out = new ofstream("a.edsf");
	  *out << "a.edsf" << endl << "^^^^^^^^" << endl;
	}
      else
	{
	  out = new ofstream("a.edmf");
	  *out << "a.edsf" << endl << "^^^^^^^^" << endl;
	}
    }
  *out << "Text data" << endl <<  "^^^^^^^^" <<  endl;
  ofstream* main = out;

  map<string, map<string, int> >::iterator outerIt;
  int dimension = 0; 
  for (outerIt = g_words.begin(); outerIt != g_words.end(); outerIt++)
    {
      *out << "\t" << dimension << "\t" << outerIt->first << endl;
      dimension++;
    }
    
  if (!args.singleFile)
    {
      *out << "^^^^^^^^" << endl;
    }

  set<string>::iterator fIter;
  for (fIter = filenames.begin(); fIter != filenames.end(); fIter++)
    {
      if (args.singleFile)
	{
	  *out << "^^^^^^^^" << endl;
	  *out << *fIter << endl;
	  *out << "^^^^^^^^" << endl;
	}
      else
	{
	  *out << *fIter << ".spasms" << endl;
	  out = new ofstream((*fIter + ".spasms").c_str());
	}
      dimension = 0;
      for (outerIt = g_words.begin(); outerIt != g_words.end(); outerIt++)
	{
	  map<string, int>::iterator innerIt;
	  innerIt = outerIt->second.find(*fIter);
	  if (innerIt != outerIt->second.end())
	    {
	      *out << dimension << "\t" << innerIt->second << endl;
	    }
	  dimension++;
	}
      if (!args.singleFile)
	{
	  out->close();
	  delete out;
	  out = main;
	}
    }
  out->close();
  delete out;
  
  PrintDebug(filenames, args);
}
