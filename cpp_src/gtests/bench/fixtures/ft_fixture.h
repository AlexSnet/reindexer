#pragma once

#include <string>
#include <vector>

#include "base_fixture.h"
#include "helpers.h"

class FullText : protected BaseFixture {
public:
	virtual ~FullText() {}
	FullText(Reindexer* db, const string& name, size_t maxItems);

	virtual Error Initialize();
	virtual void RegisterAllCases();

protected:
	virtual Item MakeItem();

protected:
	void Insert(State& state);
	void BuildInsertSteps(State& state);

	void BuildCommonIndexes(State& state);
	void BuildFastTextIndex(State& state);
	void BuildFuzzyTextIndex(State& state);

	void Fast1WordMatch(State& state);
	void Fast2WordsMatch(State& state);
	void Fuzzy1WordMatch(State& state);
	void Fuzzy2WordsMatch(State& state);

	void Fast1PrefixMatch(State& state);
	void Fast2PrefixMatch(State& state);
	void Fuzzy1PrefixMatch(State& state);
	void Fuzzy2PrefixMatch(State& state);

	void Fast1SuffixMatch(State& state);
	void Fast2SuffixMatch(State& state);
	void Fuzzy1SuffixMatch(State& state);
	void Fuzzy2SuffixMatch(State& state);

	void Fast1TypoWordMatch(State& state);
	void Fast2TypoWordMatch(State& state);
	void Fuzzy1TypoWordMatch(State& state);
	void Fuzzy2TypoWordMatch(State& state);

	void BuildStepFastIndex(State& state);

	void InitForAlternatingUpdatesAndSelects(State&);
	void AlternatingUpdatesAndSelects(benchmark::State&);

protected:
	string CreatePhrase();

	string MakePrefixWord();
	string MakeSuffixWord();
	string MakeTypoWord();

	wstring GetRandomUTF16WordByLength(size_t minLen = 4);

	vector<string> GetRandomCountries(size_t cnt = 5);
	string RandString();
	Item MakeSpecialItem();

protected:
	vector<string> words_;
	vector<string> countries_;
	std::vector<std::string> values_;

private:
	const string letters = "abcdefghijklmnopqrstuvwxyz";
	const char* alternatingNs_ = "FtAlternatingUpdatesAndSelects";

	size_t raw_data_sz_ = 0;
};
