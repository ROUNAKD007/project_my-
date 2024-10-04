// just same thing as the python version, but in C++
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

std::mt19937 Random::rng;

/**
 * Print instructions for using the program.
 */
void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

int main() {
  Random::seed(time(NULL));
  string userInput;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  vector<string> loadedDictionary;
  ifstream dictionaryFile("dictionary.txt");
  string dictionaryWord;
  while (getline(dictionaryFile, dictionaryWord)) {
    loadedDictionary.push_back(dictionaryWord);
  }
  dictionaryFile.close();

  vector<string> quadgramList;
  vector<int> quadgramCounts;
  ifstream quadgramInput("english_quadgrams.txt");
  string quadgramLine;
  while (getline(quadgramInput, quadgramLine)) {
    size_t commaLocation = quadgramLine.find(',');
    string quadgram = quadgramLine.substr(0, commaLocation);
    int countValue = stoi(quadgramLine.substr(commaLocation + 1));
    quadgramList.push_back(quadgram);
    quadgramCounts.push_back(countValue);
  }
  quadgramInput.close();

  QuadgramScorer englishScorer(quadgramList, quadgramCounts);

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    getline(cin, userInput);
    cout << endl;

    if (userInput == "C" || userInput == "c") {
      runCaesarEncrypt();
    } else if (userInput == "D" || userInput == "d") {
      runCaesarDecrypt(loadedDictionary);
    } else if (userInput == "A" || userInput == "a") {
      applyRandSubstCipherCommand();
    } else if (userInput == "E" || userInput == "e") {
      computeEnglishnessCommand(englishScorer);
    } else if (userInput == "S" || userInput == "s") {
      decryptSubstCipherCommand(englishScorer);
    } else if (userInput == "R" || userInput == "r") {
      string seedStr;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seedStr);
      Random::seed(stoi(seedStr));
    }

    cout << endl;

  } while (!(userInput == "x" || userInput == "X") && !cin.eof());

  return 0;
}

// Group related functions using VSCode "foldable" feature
#pragma region CaesarEnc

char caesarShift(char letter, int shift) {
  // Assuming letter is uppercase
  int position = ALPHABET.find(letter);
  int newPosition = ((position + shift) % 26 + 26) % 26;
  return ALPHABET[newPosition];
}

string caesarShiftText(const string& inputText, int shift) {
  string shiftedResult;
  for (char letter : inputText) {
    if (isalpha(letter)) {
      letter = toupper(letter);
      shiftedResult += caesarShift(letter, shift);
    } else if (isspace(letter)) {
      shiftedResult += letter;
    }
  }
  return shiftedResult;
}

void runCaesarEncrypt() {
  cout << "Enter the text to Caesar encrypt: ";
  string plainText;
  getline(cin, plainText);

  cout << "Enter the number of characters to rotate by: ";
  string shiftAmountStr;
  getline(cin, shiftAmountStr);
  int shiftAmount = stoi(shiftAmountStr);

  string encryptedText = caesarShiftText(plainText, shiftAmount);
  cout << encryptedText << endl;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void caesarShiftWords(vector<string>& wordList, int shiftValue) {
  for (string& word : wordList) {
    for (char& letter : word) {
      if (isalpha(letter)) {
        int position = ALPHABET.find(letter);
        int newPosition = (position + shiftValue) % 26;
        if (newPosition < 0) {
          newPosition += 26;
        }
        letter = ALPHABET[newPosition];
      }
    }
  }
}

string cleanText(const string& rawInput) {
  string cleanedText;
  for (char letter : rawInput) {
    if (isalpha(letter)) {
      cleanedText += toupper(letter);
    }
  }
  return cleanedText;
}

vector<string> splitTextBySpaces(const string& rawInput) {
  vector<string> words;
  istringstream wordStream(rawInput);
  string word;
  while (wordStream >> word) {
    words.push_back(word);
  }
  return words;
}

string joinWords(const vector<string>& words) {
  string joinedText;
  for (size_t i = 0; i < words.size(); ++i) {
    joinedText += words[i];
    if (i != words.size() - 1) {
      joinedText += ' ';
    }
  }
  return joinedText;
}

int countValidWords(const vector<string>& wordList,
                    const vector<string>& dictionary) {
  int count = 0;
  for (const string& word : wordList) {
    if (find(dictionary.begin(), dictionary.end(), word) != dictionary.end()) {
      ++count;
    }
  }
  return count;
}

void runCaesarDecrypt(const vector<string>& dictionary) {
  cout << "Enter the text to Caesar decrypt: ";
  string cipherText;
  getline(cin, cipherText);

  vector<string> originalWords = splitTextBySpaces(cipherText);

  for (string& word : originalWords) {
    word = cleanText(word);
  }

  bool decryptionFound = false;

  for (int shiftValue = 0; shiftValue < 26; ++shiftValue) {
    vector<string> shiftedWords = originalWords;
    caesarShiftWords(shiftedWords, shiftValue);
    int matchCount = countValidWords(shiftedWords, dictionary);

    if (matchCount > shiftedWords.size() / 2) {
      string decryptedMessage = joinWords(shiftedWords);
      cout << decryptedMessage << endl;
      decryptionFound = true;
    }
  }

  if (!decryptionFound) {
    cout << "No good decryptions found" << endl;
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstitutionCipher(const vector<char>& cipherKey,
                               const string& inputText) {
  string cipheredText;
  for (char letter : inputText) {
    if (isalpha(letter)) {
      char upperLetter = toupper(letter);
      int letterIndex = ALPHABET.find(upperLetter);
      cipheredText += cipherKey[letterIndex];
    } else {
      cipheredText += letter;
    }
  }
  return cipheredText;
}

void applyRandSubstCipherCommand() {
  cout << "Enter the text to substitution-cipher encrypt: ";
  string plainText;
  getline(cin, plainText);

  vector<char> cipherKey = genRandomSubstCipher();
  string encryptedText = applySubstitutionCipher(cipherKey, plainText);

  cout << encryptedText << endl;
}

#pragma endregion SubstEnc

#pragma region SubstDec

double computeQuadgramScore(const QuadgramScorer& scorer,
                            const string& inputText) {
  double totalScore = 0.0;
  string uppercaseText;
  for (char letter : inputText) {
    if (isalpha(letter)) {
      uppercaseText += toupper(letter);
    }
  }
  for (size_t i = 0; i + 3 < uppercaseText.size(); ++i) {
    string quadgram = uppercaseText.substr(i, 4);
    totalScore += scorer.getScore(quadgram);
  }
  return totalScore;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  cout << "Enter a string to score: ";
  string inputText;
  getline(cin, inputText);

  string cleanedText;
  for (char letter : inputText) {
    if (isalpha(letter)) {
      cleanedText += toupper(letter);
    }
  }

  double score = computeQuadgramScore(scorer, cleanedText);

  cout << score << endl;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& cipherText) {
  vector<char> bestKey;
  double bestScore = -INFINITY;

  string uppercaseCipherText;
  for (char letter : cipherText) {
    if (isalpha(letter)) {
      uppercaseCipherText += toupper(letter);
    }
  }

  for (int i = 0; i < 20; ++i) {
    vector<char> currentKey = genRandomSubstCipher();
    int noImprovement = 0;
    double currentScore = -INFINITY;

    while (noImprovement < 1500) {
      vector<char> newKey = currentKey;
      int idx1 = Random::randInt(25);
      int idx2 = Random::randInt(25);
      while (idx2 == idx1) {
        idx2 = Random::randInt(25);
      }
      swap(newKey[idx1], newKey[idx2]);

      string decryptedAttempt =
          applySubstitutionCipher(newKey, uppercaseCipherText);
      double newScore = computeQuadgramScore(scorer, decryptedAttempt);

      if (newScore > currentScore) {
        currentKey = newKey;
        currentScore = newScore;
        noImprovement = 0;
      } else {
        ++noImprovement;
      }
    }

    if (currentScore > bestScore) {
      bestScore = currentScore;
      bestKey = currentKey;
    }
  }
  return bestKey;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  cout << "Enter text to substitution-cipher decrypt: ";
  string cipherText;
  getline(cin, cipherText);

  vector<char> bestKey = decryptSubstCipher(scorer, cipherText);

  string decryptedText = applySubstitutionCipher(bestKey, cipherText);

  cout << decryptedText << endl;
}

#pragma endregion SubstDec
