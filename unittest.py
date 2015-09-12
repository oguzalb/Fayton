class TestCase():
    def assertTrue(self, expr, message="Assert True Error"):
        assert expr, message
    def assertFalse(self, expr, message="Assert False Error"):
        assert not expr, message
    def assertEquals(self, expr1, expr2, message="Assert Equals Error"):
        assert expr1 != expr2, message
