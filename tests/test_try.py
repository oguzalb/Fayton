e = None
try:
    assert False, "without exception class try block"
except:
    e = True
assert e == True, "catch without exception class"
e = None
try:
    assert 1 == 2, "with Exception class raise try block"
except Exception as e:
    pass
assert e.message == "with Exception class raise try block", "Exception raise"
e = None
try:
    assert 1 == 1, "with Exception class, success try block"
except Exception as e:
    pass
assert e == None, "No exception raise"
