import unittest

class MyTestCase(unittest.TestCase):
    def test_assert_true_works(self):
        self.assertTrue(1 == 1)
        self.assertFalse(1 == 2)
case = MyTestCase()
case.test_assert_true_works()
