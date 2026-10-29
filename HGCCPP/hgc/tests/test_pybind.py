from .. lib import TestPybind

Test = TestPybind.test_pybind()

Test.test()
dictionary = Test.test_dict()
print(dictionary[0])
print(dictionary[1])
print(dictionary[2])
