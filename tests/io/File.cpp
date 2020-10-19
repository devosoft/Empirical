#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/File.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test File", "[io]")
{
	// Constructor (istream)
	std::istringstream iss("Some words that should go into a file!\nThis is the next line....");
	emp::File fp(iss);
	REQUIRE(fp.GetNumLines() == 2);

	// operator[] and const
	REQUIRE(fp[0] == "Some words that should go into a file!");
	const std::string firstLine = fp[0];
	REQUIRE(std::is_const<decltype(firstLine)>::value);

	// front back
	REQUIRE(firstLine == fp.front());
	REQUIRE(fp[1] == "This is the next line....");
	REQUIRE(fp[1] == fp.back());

	// Append string
	emp::File fp2;
	fp2.Append("Whoa, here's another one!");
	REQUIRE(fp2.front() == "Whoa, here's another one!");
	REQUIRE(fp2.back() == "Whoa, here's another one!");

	// Append vector
	emp::vector<std::string> moreLines;
	moreLines.push_back("Aaaaand another one.");
	moreLines.push_back("And another one.");
	moreLines.push_back("And another one?");
	fp2.Append(moreLines);
	REQUIRE(fp2.GetNumLines() == 4);
	REQUIRE(fp2[0] == fp2.front());
	REQUIRE(fp2[1] == "Aaaaand another one.");
	REQUIRE(fp2[2] == "And another one.");
	REQUIRE(fp2[3] == fp2.back());

	// Append File
	fp.Append(fp2);
	REQUIRE(fp.GetNumLines() == 6);
	REQUIRE(fp.front() == "Some words that should go into a file!");
	REQUIRE(fp.back() == "And another one?");

	// operator+=
	fp += "More lines.";
	REQUIRE(fp.GetNumLines() == 7);
	REQUIRE(fp.back() == "More lines.");

	// operator<<
	fp << "Here's a cool line:";
	REQUIRE(fp.GetNumLines() == 8);
	REQUIRE(fp.back() == "Here's a cool line:");

	// operator>>
	std::string extractedLine;
	fp >> extractedLine;
	REQUIRE(fp.GetNumLines() == 7);
	REQUIRE(extractedLine == "Some words that should go into a file!");
	REQUIRE(fp.front() == "This is the next line....");

	// operator!=
	REQUIRE( (fp2 != fp) );

	// operator==
	fp2 = fp;
	REQUIRE( (fp2 == fp) );

	// Write (ostream)
	std::ostringstream oss;
	fp.Write(oss);
	REQUIRE(oss.str() == "This is the next line....\nWhoa, here's another one!\nAaaaand another one.\nAnd another one.\nAnd another one?\nMore lines.\nHere's a cool line:\n");

	// AsSet
	while(fp2.GetNumLines() > 0){
		fp2 >> extractedLine; // clear fp2
	}
	REQUIRE(fp2.GetNumLines() == 0);
	fp2 << "Line1";
	fp2 << "Line1";
	fp2 << "Line1";
	fp2 << "Line1";
	fp2 << "Line2";
	fp2 << "Line2";
	fp2 << "Line3";
	fp2 << "Line3";
	fp2 << "Line3";
	std::set<std::string> setOfLines = fp2.AsSet();
	REQUIRE(setOfLines.size() == 3);
	REQUIRE(fp2.GetNumLines() == 9);

	// Apply
	fp2.Apply([](std::string& s){s = s.substr(4,1);}); // remove "Line" from each line
	REQUIRE(fp2.front() == "1");
	REQUIRE(fp2.back() == "3");
	REQUIRE(fp2.GetNumLines() == 9);

	// KeepIf
	fp2.KeepIf([](const std::string& s){ return s == "1";}); // only keep "1" strings
	REQUIRE(fp2.GetNumLines() == 4);
	REQUIRE(fp2.back() == "1");

	// RemoveEmpty
	fp2 << "";
	REQUIRE(fp2.GetNumLines() == 5);
	fp2.RemoveEmpty();
	REQUIRE(fp2.GetNumLines() == 4);

	// CompressWhitespace
	emp::File fp3;
	fp3 << "  Here is			a line	\n		with   a   lot of whites p a c e \n\n\n";
	fp3.CompressWhitespace();
	REQUIRE(fp3.front() == "Here is a line with a lot of whites p a c e");

	// RemoveWhitespace
	fp3 << "This line has a few spaces.\nHere\nare\nsome\nnewlines\ntoo!";
	fp3.RemoveWhitespace();
	REQUIRE(fp3.GetNumLines() == 2);
	fp3.RemoveWhitespace(false);
	REQUIRE(fp3.GetNumLines() == 1);

	// RemoveComments
	std::string line;
	fp3 >> line; //clear fp3
	fp3 << "int x = 13;";
	fp3	<< "// x value is 13 to start";
	REQUIRE(fp3.GetNumLines() == 2);
	fp3.RemoveComments("//");
	fp3.RemoveEmpty();
	REQUIRE(fp3.GetNumLines() == 1);
	REQUIRE(fp3.back() == "int x = 13;");

	// Process
	std::function<std::string(std::string&)> fun = [](std::string& s)-> std::string { return (s+"2");};
	emp::vector<std::string> v = fp2.Process(fun);

	// ExtractCol
	fp3 >> line; //clear fp3
	fp3 << "red,1,1990";
	fp3 << "green,2,1995";
	emp::vector<std::string> column = fp3.ExtractCol();
	REQUIRE(column[0] == "red");
	REQUIRE(column[1] == "green");

	// ExtractColAs
	emp::vector<int> int_col = fp3.ExtractColAs<int>();
	REQUIRE(int_col[0] == 1);
	REQUIRE(int_col[1] == 2);
}