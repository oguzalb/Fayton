e = None
try:
    assert False, "without exception class try block"
except:
    e = True
assert e == True, "catch without exception class"
e = None
try:
    assert 1 == 2, "with Exception class raise try block"
except AssertionError as e:
    pass
assert e.message == "with Exception class raise try block", "Exception raise"
e = None
try:
    assert 1 == 1, "with Exception class, success try block"
except AssertionError as e:
    pass
assert e == None, "No exception raise"
e = None
try:
    raise Exception("with general Exception class raise try block")
except Exception as e:
    pass
assert e.message == "with general Exception class raise try block", "General Exception raise"
e = None
try:
    try:
        raise Exception("something is wrong")
    except AssertionError as e:
        pass
except Exception as e:
    pass
assert not isinstance(e, AssertionError) and e != None, "general exception not caught by subclass exception"
