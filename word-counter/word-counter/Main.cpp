#include <iostream>
#include <future>
#include <fstream>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <chrono>

using namespace std;

const bool DEBUG = false;

typedef unordered_map<string, size_t> Words;
typedef map<string, size_t> SortedWords;

bool PARALEL = true;

// odstran� jin� ne� alfanumerick� znaky a spojovn�ky
inline bool removeCharacter(char& c)
{
	if ((c <= 'z' && c >= 'a') || (c == '-'))
	{
		return false;
	}
	
	return true;
}

// posuzuje, zda je dan� string slovo
inline bool isWord(string& s)
{
	if (s.size() > 2)
	{
		return true;
	}
	
	if (s.size() == 1 && !removeCharacter(s.at(0)))
	{
		return true;
	}

	return false;
}

Words countWordsInFile(const string& fileName)
{
	// otev�e soubor
	ifstream file(fileName);
	
	// soubor nelze otev��t -> v�jimka
	if (!file)
	{
		throw "Unable to open file: " + fileName;
	}

	// mapa jednotliv�ch slov v souboru
	Words loadFromFile;

	// slovo do mapy bu� p�idej, nebo zvedni jeho �etnost o 1
	for (string word; file >> word;)
	{
		// odstran� jin� ne� alfanumerick� znaky a spojovn�ky
		word.erase(remove_if(word.begin(), word.end(), removeCharacter), word.end());
		
		// ignoruje sekvence, kter� nejsou slova
		if (!isWord(word))
		{
			continue;
		}
		
		// p�evede slovo na mal� p�smena
		transform(word.begin(), word.end(), word.begin(), ::tolower);

		++loadFromFile[word];
	}

	return loadFromFile;
}

void wordCounter(int& argc, char *argv[])
{
	SortedWords words;
	chrono::time_point<chrono::system_clock> startTime, endTime;

	if (PARALEL)
	{
		startTime = chrono::system_clock::now();
		
		// std::async vrac� n�vratov� data z funkce v prom�nn� typu future
		// vektor pro futures, kde budou kon�it n�vratov� hodnoty
		vector<future<Words>> futures;

		// rezervuje rovnou velikost vektoru pro v�echny soubory -> nemus� doch�zet k inkrement�ln� realokaci
		futures.reserve(argc - 2);

		for (int i = 2; i < argc; ++i)
		{
			futures.push_back(async(countWordsInFile, argv[i]));
		}

		// slou�� v�sledky jednotliv�ch soubor�
		for (auto& i : futures)
		{
			Words results;

			try
			{
				results = i.get();
			}
			catch (exception &err)
			{
				cerr << "ERROR: " << err.what() << '\n';
			}

			for (const auto& j : results)
			{
				words[j.first] += j.second;
			}
		}

		endTime = chrono::system_clock::now();
	}
	else
	{
		startTime = chrono::system_clock::now();

		vector<Words> futures;

		// rezervuje rovnou velikost vektoru pro v�echny soubory -> nemus� doch�zet k inkrement�ln� realokaci
		futures.reserve(argc - 2);

		for (int i = 2; i < argc; ++i)
		{
			futures.push_back(countWordsInFile(argv[i]));
		}

		// slou�� v�sledky jednotliv�ch soubor�
		for (auto& i : futures)
		{
			for (const auto& j : i)
			{
				words[j.first] += j.second;
			}
		}

		endTime = chrono::system_clock::now();
	}

	chrono::duration<double> elapsedTime = endTime - startTime;

	// v�pis v�sledk�
	cout << "Word\t\tNo. of occurrences" << endl;
	cout << "----------------------------------" << endl;
	
	for (const auto& i : words)
	{
		cout << i.first << "\t\t" << i.second << endl;
	}

	cout << endl;
	cout << "Elapsed time:\t" << elapsedTime.count() << endl << endl;
}

// Vyp�e n�pov�du k programu.
void help()
{
	cout << "WORD COUNTER - APLICATION MANUAL" << endl << endl;

	cout << "DESCRIPTION" << endl;
	cout << "\tProgram Word Counter serves to count the occurrences of unique words in given text files." << endl;
	cout << "\tResults can be printed on the screen, or writen into text file." << endl << endl;
	
	cout << "SYNOPSIS" << endl;
	cout << "\tword-counter [ -p | -s ] [ file ]..." << endl << endl;

	cout << "OPTIONS" << endl;
	cout << "\t-help\tdisplay application manual" << endl;
	cout << "\t-s\tprocess files with single-thread algorithm" << endl;
	cout << "\t-p\tprocess files with multi-thread algorithm" << endl << endl;

	cout << "NOTES" << endl;
	cout << "\tIf multi-thread algorithm choosen, every file will be processed in its own thread." << endl;
	cout << "\tFor single given file, algorithm will always run in single thread." << endl << endl;

	cout << "AUTHORS" << endl;
	cout << "\tVit Valecka <valecvit@fjfi.cvut.cz>" << endl << endl;
}

int main(int argc, char *argv[])
{	
	// kontrola argument�
	if ((string)argv[1] == "help" || (string)argv[1] == "-help" || (string)argv[1] == "HELP" || (string)argv[1] == "-HELP" || (string)argv[1] == "/help" || (string)argv[1] == "/HELP" || (string)argv[1] == "/?")
	{
		help();
	}
	else if ((string)argv[1] == "s" || (string)argv[1] == "S" || (string)argv[1] == "-s" || (string)argv[1] == "-S" || (string)argv[1] == "/s" || (string)argv[1] == "/S")
	{
		PARALEL = false;
		wordCounter(argc, argv);
	}
	else if ((string)argv[1] == "p" || (string)argv[1] == "P" || (string)argv[1] == "-p" || (string)argv[1] == "-P" || (string)argv[1] == "/p" || (string)argv[1] == "/P")
	{
		PARALEL = true;
		wordCounter(argc, argv);
	}
	else
	{
		cout << "Parameter mishmash. Program will exit." << endl << endl;
	}

	if (DEBUG)
	{
		system("PAUSE");
	}
}