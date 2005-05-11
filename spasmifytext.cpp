///////////////////////////////////////////////////////////////////////////////
// spasmifytext.cpp
///////////////////////////////////////////////////////////////////////////////
// A program to help convert standard .txt files into multi-dimensional 
// datapoints for analysis (each dimension corresponds to the count for a 
// different word).  Currently does not take advantage of any stemming / 
// punctiuation removal techniques.
//
// The main goal is to create a dimension mapping for every word that occurs 
// in the input files specified at the command line, and maintain count 
// information for the number of times each word occurs in each file.  
// Since this is text the information is stored sparsly since for a given file
// it is likely that it does not contain most words in the universe of input 
// files (most input will likely be of the "*.txt" form).
//
// The program uses a map of maps to generate this information on a single
// pass through the data.  The first entry in the map is a string representing 
// the actual word, and the second information is a map to all filenames using
// that word, and the count for how many times that word occurs in the file.
//
// Author: Ben Anderson
// Date: May 05, 2005
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

// This is a pretty simple program so it's ok to use globals.
// map<word, map<filename, count for word in filename> > 
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

// Prints debug information for the input arguments.
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
* the input stream, adding entries for each new word found in it and 
* incrementing entries for words already found.
*/
void ProcessStream(istream& in, const string& name)
{
	string temp;
	map<string, map<string, int> >::iterator wIter;
	map<string, int>::iterator fIter;
	while(in >> temp)
	{
		// Check for presence of word
		wIter = g_words.find(temp);

		// If it already exists, check to see if we've already found it in this 
		// file/stream
		if (wIter != g_words.end())
		{
			fIter = wIter->second.find(name);
			// If it does, increment the count
			if (fIter != wIter->second.end())
			{
				fIter->second++;
			}
			// Otherwise create it and set it's count to 1
			else
			{
				wIter->second.insert(make_pair(name,1));
			}
		}
		// If it doesn't exist, create it, and add an entry for this file/stream
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
	// Store filenames in a set so we don't process the same file twice, doubling
	// its word counts.
	set<string> filenames;

	// The name for the main Enchilada data format file (suffix will be generated
	// automatically)
	string outfile;


	// Process command line arguments.
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
			else // process short form arguments
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

	// read from standard input if they don't give any.
	if (filenames.size() == 0)
	{
		ProcessStream(cin, "STDIN");
	}
	// otherwise, process the files they input
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

	// work with pointers so we can use the same file to output in either
	// single file or mutli file format
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

	// output the dimension mapping
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

	// output individual datapoints
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

#ifdef DEBUG  
	// Print the arguments for debugging.
	PrintDebug(filenames, args);
#endif
}
