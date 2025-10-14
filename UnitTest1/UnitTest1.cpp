#include "pch.h"
#include "CppUnitTest.h"

extern "C" {
    #include "../BinaryTree/BinaryTree.cpp"
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(AddAndFindNode)
		{
			Node* tree = initTree();
			tree = addNode(tree, 5);
			tree = addNode(tree, 3);
			tree = addNode(tree, 7);

			Node* found = findNode(tree, 3);
			Assert::IsNotNull(found);
			Assert::AreEqual(3, found->value);

			found = findNode(tree, 7);
			Assert::IsNotNull(found);
			Assert::AreEqual(7, found->value);

			found = findNode(tree, 10);
			Assert::IsNull(found);

			freeTree(tree);
		}

		TEST_METHOD(RemoveNode)
		{
			Node* tree = initTree();
			tree = addNode(tree, 5);
			tree = addNode(tree, 3);
			tree = addNode(tree, 7);

			tree = removeNode(tree, 3);
			Node* found = findNode(tree, 3);
			Assert::IsNull(found);

			tree = removeNode(tree, 5);
			found = findNode(tree, 5);
			Assert::IsNull(found);

			freeTree(tree);
		}

		TEST_METHOD(SaveAndLoadTree)
		{
			Node* tree = initTree();
			tree = addNode(tree, 5);
			tree = addNode(tree, 3);
			tree = addNode(tree, 7);

			FILE* file = fopen("test_tree.bin", "wb");
			Assert::IsNotNull(file);
			saveTree(file, tree);
			fclose(file);

			file = fopen("test_tree.bin", "rb");
			Assert::IsNotNull(file);
			Node* loadedTree = loadTree(file);
			fclose(file);

			Node* found = findNode(loadedTree, 3);
			Assert::IsNotNull(found);
			Assert::AreEqual(3, found->value);

			found = findNode(loadedTree, 7);
			Assert::IsNotNull(found);
			Assert::AreEqual(7, found->value);

			freeTree(tree);
			freeTree(loadedTree);
		}
	};
}
