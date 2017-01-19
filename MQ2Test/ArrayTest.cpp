
#include "../MQ2Main/ArrayClass.h"

#include <stdio.h>
#include <exception>
#include <vector>

void FailTest(const char* message) {
	throw std::exception(message);
}

template <typename ArrayType>
void DoArrayClassTests()
{
	auto isEqual = [](const auto& array1, const std::vector<int>& vec) -> bool
	{
		if (array1.GetLength() != vec.size())
			return false;

		for (int i = 0; i < (int)vec.size(); ++i)
		{
			if (array1[i] != vec[i])
				return false;
		}

		return true;
	};

	ArrayType arr;

	if (!isEqual(arr, {})) {
		FailTest("empty array");
	}

	arr.Add(0);
	arr.Add(1);
	arr.Add(2);
	arr.Add(3);
	if (!isEqual(arr, { 0, 1, 2, 3 })) { FailTest("Add"); }

	arr.InsertElement(1, 5);
	if (!isEqual(arr, { 0, 5, 1, 2, 3 })) { FailTest("InsertElement"); }

	arr.DeleteElement(2);
	if (!isEqual(arr, { 0, 5, 2, 3 })) { FailTest("DeleteElement"); }

	arr.SetElementIdx(0, 9);
	if (!isEqual(arr, { 9, 5, 2, 3 })) { FailTest("SetElementIdx"); }

	ArrayType arr2 = arr;
	if (!isEqual(arr2, { 9, 5, 2, 3 })) { FailTest("Copy"); }

	arr2.InsertElement(1, 5);
	if (!isEqual(arr2, { 9, 5, 5, 2, 3 })) { FailTest("InsertElement2"); }

	for (int i = 0; i < 30; ++i)
		arr2.Add(i);
	if (!isEqual(arr2, { 9, 5, 5, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29})) { FailTest("InsertElement3"); }


	arr = arr2;
	if (!isEqual(arr, { 9, 5, 5, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29 })) {
		FailTest("Copy2");
	}

	arr2.Reset();
	if (!isEqual(arr2, {})) { FailTest("Reset"); }

	if (!isEqual(arr, { 9, 5, 5, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29 })) {
		FailTest("Consistency");
	}
}

bool DoArrayClassTests()
{
	int which = 1;
	try {
		DoArrayClassTests<ArrayClass<int>>();
		which = 2;
		DoArrayClassTests<ArrayClass2<int>>();
	}
	catch (const std::exception& exc) {
		printf("Failed test ArrayClass%s: %s", (which == 1 ? "" : "2"), exc.what());
		return false;
	}

	return true;
}
