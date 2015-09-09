e = None
try:
    assert 1 == 2, "message"
except Exception as e:
    pass
assert e != None, "Exception raise"
assert e.message == "message", "Exception raise"
e = None
try:
    assert 1 == 1, "message"
except Exception as e:
    pass
assert e == None, "No exception raise"
