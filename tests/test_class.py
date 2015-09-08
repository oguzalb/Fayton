class A():
    pass

assert A() != A(), "instance equals calls object equals False"

a = A()
assert a == a, "instance equals calls object equals True"

class A(dict):
    pass

a = A()
a["name"] = "Fayton"
assert a["name"] == "Fayton", "builtin subclass works"

class B(A):
    pass

b = B()
a["name"] = "Fayton"
assert a["name"] == "Fayton", "builtin subclass subclass works"

class C(list):
    def average(self):
        total = 0
        for i in self:
            total = total + i
        return total / self.__len__()
c = C()
c.append(1)
c.append(10)
assert c.average() == 5, "builtin subclass can use new/parent functions"
