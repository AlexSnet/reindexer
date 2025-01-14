#pragma once

#include "reindexer_api.h"

class SparseIndexesApi : public ReindexerApi {
protected:
	void SetUp() override {
		Error err = rt.reindexer->OpenNamespace(default_namespace);
		ASSERT_TRUE(err.ok()) << err.what();

		DefineNamespaceDataset(default_namespace, {IndexDeclaration{kFieldId, "hash", "string", IndexOpts().PK(), 0},
												   IndexDeclaration{kFieldName, "tree", "string", IndexOpts().Sparse(), 0},
												   IndexDeclaration{kFieldSerialNumber, "hash", "int64", IndexOpts().Sparse(), 0}});

		FillNsFromJson();
	}

	void FillNsFromJson() {
		char sourceJson[1024];
		const char jsonPattern[] = R"xxx({"id": "%s", "name" : "%s", "serialNumber" : %ld})xxx";
		for (long i = 0; i < 100; ++i) {
			Item item = NewItem(default_namespace);
			EXPECT_TRUE(item.Status().ok()) << item.Status().what();

			auto serialNumber = std::to_string(i);
			auto id = "key" + serialNumber;
			string name = "name" + serialNumber;
			sprintf(sourceJson, jsonPattern, id.c_str(), name.c_str(), i);

			Error err = item.FromJSON(sourceJson);
			EXPECT_TRUE(err.ok()) << err.what();

			Upsert(default_namespace, item);

			err = Commit(default_namespace);
			EXPECT_TRUE(err.ok()) << err.what();
		}
	}

	void CheckSelectAll() {
		QueryResults qr;
		Error err = rt.reindexer->Select(Query(default_namespace), qr);
		EXPECT_TRUE(err.ok()) << err.what();
		EXPECT_TRUE(qr.Count() == 100);

		for (size_t i = 0; i < qr.Count(); ++i) {
			Item ritem(qr[i].GetItem(false));
			std::string expectedName = "name" + std::to_string(i);
			Variant nameVal = ritem[kFieldName];
			EXPECT_TRUE(nameVal.Type() == KeyValueString);
			EXPECT_TRUE(nameVal.As<string>() == expectedName);

			int64_t expectedSerialNumber = static_cast<int64_t>(i);
			Variant serialNumberVal = ritem[kFieldSerialNumber];
			EXPECT_TRUE(serialNumberVal.Type() == KeyValueInt64);
			EXPECT_TRUE(static_cast<int64_t>(serialNumberVal) == expectedSerialNumber);
		}
	}

	void CheckSelectByTreeIndex() {
		QueryResults qr;
		Error err = rt.reindexer->Select(Query(default_namespace).Where(kFieldName, CondEq, Variant("name5")), qr);
		EXPECT_TRUE(err.ok()) << err.what();
		EXPECT_TRUE(qr.Count() == 1);
		Item ritem(qr[0].GetItem(false));
		Variant nameVal = ritem[kFieldName];
		EXPECT_TRUE(nameVal.Type() == KeyValueString);
		EXPECT_TRUE(nameVal.As<string>() == "name5");

		QueryResults qr2;
		const string toCompare("name2");
		err = rt.reindexer->Select(Query(default_namespace).Where(kFieldName, CondLt, Variant(toCompare)), qr2);
		EXPECT_TRUE(err.ok()) << err.what();

		for (auto it : qr2) {
			Item ritem(it.GetItem(false));
			Variant nameVal = ritem[kFieldName];
			EXPECT_TRUE(nameVal.Type() == KeyValueString);
			EXPECT_TRUE(nameVal.As<string>().compare(toCompare) < 0);
		}
	}

	void CheckSelectByHashIndex() {
		QueryResults qr;
		Error err = rt.reindexer->Select(Query(default_namespace).Where(kFieldSerialNumber, CondLt, Variant(static_cast<int64_t>(50))), qr);
		EXPECT_TRUE(err.ok()) << err.what();
		EXPECT_TRUE(qr.Count() == 50);
		for (int64_t i = 0; i < static_cast<int64_t>(qr.Count()); ++i) {
			Item ritem(qr[i].GetItem(false));
			Variant serialNumberVal = ritem[kFieldSerialNumber];
			EXPECT_TRUE(serialNumberVal.Type() == KeyValueInt64);
			EXPECT_TRUE(static_cast<int64_t>(serialNumberVal) == i);
		}

		QueryResults qr2;
		int64_t expectedValue(static_cast<int64_t>(77));
		err = rt.reindexer->Select(Query(default_namespace).Where(kFieldSerialNumber, CondEq, Variant(expectedValue)), qr2);
		EXPECT_TRUE(err.ok()) << err.what();
		EXPECT_TRUE(qr2.Count() == 1);

		Item ritem(qr2[0].GetItem(false));
		Variant serialNumberVal = ritem[kFieldSerialNumber];
		EXPECT_TRUE(serialNumberVal.Type() == KeyValueInt64);
		EXPECT_TRUE(static_cast<int64_t>(serialNumberVal) == expectedValue);
	}

	const char* kFieldId = "id";
	const char* kFieldName = "name";
	const char* kFieldSerialNumber = "serialNumber";
};
