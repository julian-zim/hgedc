#ifndef HGCCPP_CSV_PARSER_HPP
#define HGCCPP_CSV_PARSER_HPP

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// parses a CSV file into a flat, row-major table of cells
class CSVParser {

public:
	void parse(const std::string& filename, char sep = ',', char quote = '\"', bool trim = true) {
		numRows_ = 0;
		numColumns_ = 0;
		fields_.clear();
		std::ifstream csvFile(filename.c_str());
		if (!csvFile.is_open()) {
			throw std::runtime_error("The file " + filename + " cannot be opened.");
		}
		std::string row;
		std::size_t rowId{0};
		while (std::getline(csvFile, row)) {
			parseRow_(row, rowId++, sep, quote, trim);
		}
	}

	std::size_t num_rows() const {
		return numRows_;
	}

	std::size_t num_columns() const {
		return numColumns_;
	}

	const std::string& cell(std::size_t row, std::size_t column) const {
		if (row >= numRows_) {
			throw std::runtime_error("Non-existing row " + std::to_string(row) + ". Must be between 0 and " + std::to_string(numRows_ - 1) + ".");
		}
		if (column >= numColumns_) {
			throw std::runtime_error("Non-existing column " + std::to_string(column) + ". Must be between 0 and " + std::to_string(numColumns_ - 1) + ".");
		}
		return fields_.at(row * numColumns_ + column);
	}

private:
	std::size_t numRows_{0};
	std::size_t numColumns_{0};
	std::vector<std::string> fields_;

	void parseRow_(const std::string& row, std::size_t rowId, char sep, char quote, bool trim) {
		std::size_t numFieldsInRow{tokenize_(row, sep, quote, trim, fields_)};
		if (numFieldsInRow == 0) {
			throw std::runtime_error("Row " + std::to_string(rowId) + " is empty.");
		}
		if (numColumns_ == 0) {
			numColumns_ = numFieldsInRow;
		}
		else if (numColumns_ != numFieldsInRow) {
			throw std::runtime_error("The rows " + std::to_string(rowId - 1) + " and " + std::to_string(rowId) + " have different numbers of columns.");
		}
		numRows_++;
	}

	std::size_t tokenize_(const std::string& sentence, char sep, char quote, bool trim, std::vector<std::string>& tokens) const {
		std::size_t numTokens{0};
		if (sep == quote) {
			throw std::runtime_error("Separator and quotation mark are identical.");
		}
		bool outsideQuotes{true};
		std::size_t wordLength{0};
		std::size_t posWordStart{0};
		std::string token;
		for (std::size_t pos{0}; pos < sentence.size(); pos++) {
			if (sentence.at(pos) == quote) {
				if (!outsideQuotes and pos < sentence.size() - 1) {
					if (sentence.at(pos + 1) != sep) {
						throw std::runtime_error("Sentence contains closing quote which is followed by a char different from " + std::to_string(sep) + ".");
					}
				}
				wordLength++;
				outsideQuotes = !outsideQuotes;
			}
			else if (outsideQuotes and sentence.at(pos) == sep) {
				if (wordLength > 0) {
					token = sentence.substr(posWordStart, wordLength);
					if (trim) {
						trim_(token);
					}
					tokens.push_back(token);
				}
				else {
					tokens.emplace_back("");
				}
				numTokens++;
				posWordStart = pos + 1;
				wordLength = 0;
			}
			else {
				wordLength++;
			}
		}
		if (!outsideQuotes) {
			throw std::runtime_error("Sentence contains unbalanced quotes.");
		}
		if (wordLength > 0) {
			token = sentence.substr(posWordStart, wordLength);
			if (trim) {
				trim_(token);
			}
			tokens.push_back(token);
			numTokens++;
		}
		return numTokens;
	}

	void trim_(std::string& token) const {
		token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](char c) { return c != ' '; }));
		token.erase(std::find_if(token.rbegin(), token.rend(), [](char c) { return c != ' '; }).base(), token.end());
	}
};

#endif //HGCCPP_CSV_PARSER_HPP
