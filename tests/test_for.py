l = []
for i in [1, 2, 3]:
    l.append(i)
assert l == [1, 2, 3], "for on variable"

l = []
for i, j in [[1, 2], [3, 4]]:
    l.append(i)
    l.append(j)
assert l == [1, 2, 3, 4], "for on tuple"

l = []
for (i, j) in [[1, 2], [3, 4]]:
    l.append(i)
    l.append(j)
assert l == [1, 2, 3, 4], "for on tuple with parenthesis"

class Iterable():
    def __init__(self, my_list):
        self.iterator = my_list.__iter__()

    def __iter__(self):
        return self

    def next(self):
        return self.iterator.next()

l = []
for i in Iterable([1, 2, 3, 4]):
    l.append(i)

assert l == [1, 2, 3, 4]
