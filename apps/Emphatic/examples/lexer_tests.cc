#include <iostream>
#include <string>

std::string GetString();

int main()
{
  // Comment type one.
  /* Comment type two. */
  [[maybe_unused]] int t1 = 1;  // With a comment following.
  [[maybe_unused]] std::string t2 = "two.";      // A simple string.
  [[maybe_unused]] std::string t3 = "three\n.";  // With an escape character.
  [[maybe_unused]] std::string t4 = "\"four\"";  // In quotes.
  [[maybe_unused]] std::string t5 = "( ";        // Mis-matched paren and whitespace.
  [[maybe_unused]] std::string t6 = GetString(); // Call a non-trivial string function.
}

std::string GetString() {
  return "}";  // This should NOT match the open brace!
}
