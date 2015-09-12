def func(a, b=1):
    return b
assert func(1) == 1, "func kwarg default value"
assert func(1, 2) == 2, "func kwarg pass value"
assert func(1, b=2) == 2, "func kwarg pass value with name"
e = None
try:
    func(1, 2, 3)
except TypeError as e:
    pass
assert e != None, "function with too many params after a kwarg"
def func(a, b=1, c=2):
    return c + b
assert func(1) == 3, "func kwarg default value, not first"
assert func(1, 3, 4) == 7, "func kwarg pass value"
assert func(1, 3, c=4) == 7, "func kwarg pass mixed"
assert func(1, b=3, c=4) == 7, "func kwarg pass all value"

class C():
    def func(self, a, b=1):
        return b
c = C()
assert c.func(1) == 1, "class method kwarg default value"
assert c.func(1, 2) == 2, "class method kwarg pass value one"
assert c.func(1, b=2) == 2, "class method kwarg pass value with name 2"
class C():
    def func(self, a, b=1, c=2):
        return c + b
c = C()
assert c.func(1) == 3, "class method kwarg default value, not first"
assert c.func(1, 3, 4) == 7, "class method kwarg pass value two"
assert c.func(1, 3, c=4) == 7, "class method kwarg pass mixed"
assert c.func(1, b=3, c=4) == 7, "class method kwarg pass all value"
e = None
try:
    c.func(1, 2, 3, 4)
except TypeError as e:
    pass
assert e != None, "class method with too many params, after a kwarg"

